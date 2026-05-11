// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// clientData structure definition
struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    char address[50];     // account address
    char phone[15];       // phone number
    char email[30];       // email address
    char accountType[10]; // account type: savings or checking
    double balance;       // account balance
};                        // end structure clientData

// user structure for authentication
struct User
{
    char username[20];
    char password[20];
};

// prototypes
int authenticate(void);
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void calculateInterest(FILE *fPtr);
void viewTransactionHistory(unsigned int accountNum);
void logTransaction(unsigned int accountNum, char *type, double amount, double balance);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    // authenticate user
    if (!authenticate())
    {
        printf("Authentication failed. Exiting...\n");
        return 1;
    }

    // fopen opens the file; exits if file cannot be opened
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(-1);
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 7)
    {
        switch (choice)
        {
        // create text file from record file
        case 1:
            textFile(cfPtr);
            break;
        // update record
        case 2:
            updateRecord(cfPtr);
            break;
        // create record
        case 3:
            newRecord(cfPtr);
            break;
        // delete existing record
        case 4:
            deleteRecord(cfPtr);
            break;
        // calculate interest
        case 5:
            calculateInterest(cfPtr);
            break;
        // view transaction history
        case 6:
            {
                unsigned int acct;
                printf("Enter account number to view history: ");
                scanf("%u", &acct);
                viewTransactionHistory(acct);
            }
            break;
        // display if user does not select valid choice
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    }     // end while

    fclose(cfPtr); // fclose closes the file
    return 0;
} // end main

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    int result;     // used to test whether fread read any bytes
    // create clientData with default information
    struct clientData client = {0, "", "", "", "", "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%-50s%-15s%-30s%-10s%10s\n", "Acct", "Last Name", "First Name", "Address", "Phone", "Email", "Type", "Balance");

        // copy all records from random-access file into text file
        while (!feof(readPtr))
        {
            result = fread(&client, sizeof(struct clientData), 1, readPtr);

            // write single record to text file
            if (result != 0 && client.acctNum != 0)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%-50s%-15s%-30s%-10s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        client.address, client.phone, client.email, client.accountType, client.balance);
            } // end if
        }     // end while

        fclose(writePtr); // fclose closes the file
    }                     // end else
} // end function textFile

// update balance in record
void updateRecord(FILE *fPtr)
{
    unsigned int account; // account number
    double transaction;   // transaction amount
    // create clientData with no information
    struct clientData client = {0, "", "", "", "", "", "", 0.0};

    // obtain number of account to update
    printf("%s", "Enter account to update ( 1 - 100 ): ");
    scanf("%d", &account);

    // move file pointer to correct record in file
    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    { // update record
        printf("%-6d%-16s%-11s%-50s%-15s%-30s%-10s%10.2f\n\n", client.acctNum, client.lastName, client.firstName,
               client.address, client.phone, client.email, client.accountType, client.balance);

        // request transaction amount from user
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);
        double oldBalance = client.balance;
        client.balance += transaction; // update record balance

        printf("%-6d%-16s%-11s%-50s%-15s%-30s%-10s%10.2f\n", client.acctNum, client.lastName, client.firstName,
               client.address, client.phone, client.email, client.accountType, client.balance);

        // move file pointer to correct record in file
        // move back by 1 record length
        fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);
        // write updated record over old record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);

        // log transaction
        char type[15];
        if (transaction > 0) strcpy(type, "Deposit");
        else strcpy(type, "Withdrawal");
        logTransaction(account, type, transaction, client.balance);
    } // end else
} // end function updateRecord

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;                       // stores record read from file
    struct clientData blankClient = {0, "", "", "", "", "", "", 0}; // blank client
    unsigned int accountNum;                        // account number

    // obtain number of account to delete
    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    scanf("%d", &accountNum);

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
    } // end if
    else
    { // delete record
        // move file pointer to correct record in file
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        // replace existing record with blank record
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
        printf("Account %d deleted.\n", accountNum);
    } // end else
} // end function deleteRecord

// create and insert record
void newRecord(FILE *fPtr)
{
    // create clientData with default information
    struct clientData client = {0, "", "", "", "", "", "", 0.0};
    unsigned int accountNum; // account number

    // obtain number of account to create
    printf("%s", "Enter new account number ( 1 - 100 ): ");
    scanf("%d", &accountNum);

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
    } // end if
    else
    { // create record
        // user enters last name, first name and balance
        printf("%s", "Enter lastname, firstname, address, phone, email, accountType (savings/checking), balance\n? ");
        scanf("%14s%9s%49s%14s%29s%9s%lf", client.lastName, client.firstName, client.address, client.phone, client.email, client.accountType, &client.balance);

        client.acctNum = accountNum;
        // move file pointer to correct record in file
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        // insert record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
        printf("Account %d created.\n", accountNum);
    } // end else
} // end function newRecord

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    // display available options
    printf("%s", "\nEnter your choice\n"
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - calculate interest for savings accounts\n"
                 "6 - view transaction history\n"
                 "7 - end program\n? ");

    scanf("%u", &menuChoice); // receive choice from user
    return menuChoice;
} // end function enterChoice

// authenticate user
int authenticate(void)
{
    struct User admin = {"admin", "password"}; // hardcoded for simplicity
    char username[20], password[20];
    printf("Enter username: ");
    scanf("%19s", username);
    printf("Enter password: ");
    scanf("%19s", password);
    if (strcmp(username, admin.username) == 0 && strcmp(password, admin.password) == 0)
    {
        return 1;
    }
    return 0;
}

// calculate interest for savings accounts
void calculateInterest(FILE *fPtr)
{
    struct clientData client = {0, "", "", "", "", "", "", 0.0};
    rewind(fPtr);
    while (fread(&client, sizeof(struct clientData), 1, fPtr))
    {
        if (client.acctNum != 0 && strcmp(client.accountType, "savings") == 0)
        {
            double interest = client.balance * 0.05; // 5% interest
            client.balance += interest;
            fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);
            fwrite(&client, sizeof(struct clientData), 1, fPtr);
            printf("Interest added to account %d: %.2f\n", client.acctNum, interest);
            logTransaction(client.acctNum, "Interest", interest, client.balance);
        }
    }
}

// view transaction history
void viewTransactionHistory(unsigned int accountNum)
{
    char filename[20];
    sprintf(filename, "history_%d.txt", accountNum);
    FILE *fPtr = fopen(filename, "r");
    if (fPtr == NULL)
    {
        printf("No transaction history for account %d.\n", accountNum);
        return;
    }
    char line[256];
    printf("Transaction History for Account %d:\n", accountNum);
    while (fgets(line, sizeof(line), fPtr))
    {
        printf("%s", line);
    }
    fclose(fPtr);
}

// log transaction
void logTransaction(unsigned int accountNum, char *type, double amount, double balance)
{
    char filename[20];
    sprintf(filename, "history_%d.txt", accountNum);
    FILE *fPtr = fopen(filename, "a");
    if (fPtr == NULL)
    {
        printf("Error opening history file.\n");
        return;
    }
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    fprintf(fPtr, "%04d-%02d-%02d %02d:%02d:%02d - %s: %.2f, Balance: %.2f\n",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
            type, amount, balance);
    fclose(fPtr);
}