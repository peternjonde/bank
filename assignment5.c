#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

pthread_mutex_t lock;

struct AccountInfo
{

    int transaction_value;
    int account_value;
};

void *withdraw(void *acount_info)
{
    struct AccountInfo *info = (struct AccountInfo *)acount_info;

    pthread_mutex_lock(&lock);

    if (info->account_value - info->transaction_value < 0)
    {
        return info->account_value; // if transaction would turn the bank account negative then return original ammount
    }
    else
    {
        return info->account_value - info->transaction_value;
    }

    pthread_mutex_unlock(&lock);
}

void *deposit(void *acount_info)
{
    struct AccountInfo *info = (struct AccountInfo *)acount_info;

    pthread_mutex_lock(&lock);

    if (info->account_value + info->transaction_value < 0)
    {
        return info->account_value;
    }
    else
    {
        return info->account_value + info->transaction_value;
    }
    pthread_mutex_unlock(&lock);
}

int main(void)
{
    int num_accounts = 0;
    int num_clients = 0;
    int count = 0;

    FILE *file;
    char line[100]; // Adjust the size according to your needs

    // Open the file in read mode
    file = fopen("assignment_5_input.txt", "r");

    // Check if the file is opened successfully
    if (file == NULL)
    {
        perror("Error opening file");
        return 1; // Exit the program with an error code
    }

    // First read of file to get numbers of accounts/ clients
    while (fgets(line, sizeof(line), file) != NULL)
    {

        // gets numbers of accounts
        if (line[0] == 'A')
        {
            num_accounts++;
        }
        // gets number of clients
        if (line[0] == 'C')
        {
            num_clients++;
        }
    }

    fclose(file);

    // secound run to get the actual info from file
    file = fopen("assignment_5_input.txt", "r");

    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    int account_amount[num_accounts]; // array holding account values
    pthread_t client[num_clients];

    // Read the file line by line using fgets
    while (fgets(line, sizeof(line), file) != NULL)
    {

        if (line[0] == 'A')
        {
            // turns the number of the acount into char* so I can use atoi
            char help[2];
            help[0] = line[1];
            help[1] = '\0';

            int acount_num = atoi(help); // shows account number
            int j = 0;
            int num_find = 0;     // shows if num found
            char amount_init[50]; // stores initial char value of acount

            // get the deposit number
            for (int i = 0; line[i] != '\0'; i++)
            {
                // Check if the character is a digit and if a non-digit was found before
                if (isdigit(line[i]) && i != 1)
                {
                    // Store the digit in the numericString
                    amount_init[j] = line[i];
                    j++;
                    num_find = 1; // Set the flag to indicate a digit has been found
                }
                else if (num_find)
                {
                    // Break the loop if a non-digit character is encountered after digits
                    break;
                }
            }

            account_amount[acount_num - 1] = atoi(amount_init);
        }

        if (line[0] == 'C')
        {
            int err_thread_deposit, err_thread_withdraw;
            // thread for each client
            // to get client number
            char help[2];
            help[0] = line[1];
            help[1] = '\0';
            int client_num = atoi(help);

            for (int i = 0; line[i] != '\0'; i++)
            {

                // Client wants to deposit
                if (line[i] == 'd')
                {
                    int err_thread_deposit;
                    i = i + 9; // move pointer to the number of the acount
                    char help_account[2];
                    help_account[0] = line[i];
                    help_account[1] = '\0';
                    int account_num = atoi(help_account);

                    i = i + 2; // move pointer to deposit amount
                    char amount_init[50] = " ";
                    int j = 0;
                    // get deposit amount
                    while (isdigit(line[i]))
                    {
                        amount_init[j] = line[i];
                        i++;
                        j++;
                    }
                    int deposit_value = atoi(amount_init);

                    struct AccountInfo info = {deposit_value, account_amount[account_num - 1]};

                    err_thread_deposit = pthread_create(&client[client_num], NULL, &deposit, (void *)&info); // create thread for deposit

                    if (err_thread_deposit != 0)
                    {
                        printf("\n Error creating thread");
                        return 0;
                    }

                    int deposit_result;

                    err_thread_deposit = pthread_join(client[client_num], (void **)&deposit_result);

                    if (err_thread_deposit != 0)
                    {
                        printf("\n Error");
                        return 0;
                    }

                    account_amount[account_num - 1] = deposit_result;
                    // printf("Account %d deposit result: %d\n", account_num, deposit_result);
                }
                // clients wants to withdraw
                else if (line[i] == 'w')
                {

                    int err_thread_withdraw;
                    i = i + 10; // move pointer to the number of the acount
                    char help_account[2];
                    help_account[0] = line[i];
                    help_account[1] = '\0'; // maybe able to remove
                    int account_num = atoi(help_account);

                    i = i + 2; // move pointer to withdraw amount
                    char amount_init[50] = " ";
                    int j = 0;
                    // get withdraw amount
                    while (isdigit(line[i]))
                    {
                        amount_init[j] = line[i];
                        i++;
                        j++;
                    }
                    int withdraww_value = atoi(amount_init);

                    struct AccountInfo info = {withdraww_value, account_amount[account_num - 1]};

                    err_thread_withdraw = pthread_create(&client[client_num], NULL, &withdraw, (void *)&info); // create thread for deposit

                    if (err_thread_withdraw != 0)
                    {
                        printf("\n Error creating thread");
                        return 0;
                    }

                    int withdraw_result;

                    err_thread_withdraw = pthread_join(client[client_num], (void **)&withdraw_result);

                    if (err_thread_withdraw != 0)
                    {
                        printf("\n Error");
                        return 0;
                    }

                    account_amount[account_num - 1] = withdraw_result;

                    // printf("Acount %d withdraw result: %d\n", account_num, withdraw_result);

                    if (num_clients == client_num)
                    { // all clients have passed
                        break;
                    }
                }
            }
        }
    }

    // Close the file when you're done

    printf("# of Accounts: %d\n", num_accounts);
    printf("# of Clients: %d\n", num_clients);

    for (int i = 0; i < num_accounts; i++)
    {

        printf("A%d balance %d\n", i + 1, account_amount[i]);
    }

    pthread_mutex_destroy(&lock);
    return 0; // Exit the program successfully
}
