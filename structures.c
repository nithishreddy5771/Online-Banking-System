
#include<stdio.h>
#include<stdlib.h>

struct Customer
{
	char username[1024];
	char password[1024];
	char account_number[1024];
	int customer_type;  //admin - 0   single account user -  1   joint account user- 2
};

struct Account
{
	char account_number[1024];
	double balance;
};

struct Transaction
{
	char date[128];
	char account_number[1024];
	double amount_credit;
	double amount_debit;
	double balance_remaining;
};

