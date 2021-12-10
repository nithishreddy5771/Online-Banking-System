
#include<stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<time.h>
#include "structures.c"

// all function declarations
void add_account();
void delete_account( char account_number[] );
void delete_username(char username[]);
void delete();
void print_customers();
void print_accounts();
void print_transactions();
void modify();
void search();


//void deposit_money(char account_number[1024]);
//void withdraw_money(char account_number[1024]);
//void print_accounts_acc(char account_number[1024]);
//void print_transactions_acc(char account_number[1024]);

void password_change(char username[1024]);

void joint_deposit_money(char account_number[1024], int joint);
void joint_withdraw_money(char account_number[1024],int joint);
void joint_print_accounts_acc(char account_number[1024], int joint);
void joint_print_transactions_acc(char account_number[1024], int joint);

void main()
{
	printf("------------------Welcome to STATE BANK OF HYDERABAD------------------------------\n");
	printf("Enter username and password to login to your account\n");
	char username[1024], password[1024];
	printf("Username:  ");
	scanf("%s", username);
	printf("\nPassword:  ");
	scanf("%s", password);
	
	int login_checked=0;  // 0 for credentials doesn't exist and 1 for creadentials exist
	int fd=open("Customer.txt", O_RDWR, 0744);
	struct Customer check;
	// we need to check in the database for login to the system
	// search for the username and password, if customer found, retrieve the customer_type
	while(read(fd, &check, sizeof(check)) != 0)
	{
		//printf("%s\t %s\n", check.username, check.password);
		if( strcmp(check.username, username)==0 && strcmp(check.password, password)==0 )
		{
			login_checked=1;
			break;
		}
	} // end of while
	close(fd);
	if(login_checked==0) // if username and password doesn't match
	{
		printf("username and password doesn't match with existing records\n");
		exit(0);
	}
	
	if(check.customer_type== 0)
	{
		
		printf("------------welcome administrator------------------\n");
		while(1)
		{
		printf("\n\npress 1 to add an account\npress 2 to delete username/account\npress 3 to modify an account\npress 4 to search for an account\npress 5 to exit\n");
		int option_selected;
		scanf("%d", &option_selected);
		
		if(option_selected==1) // add account
			add_account();
		else if(option_selected==2) // delete username/account
			delete();
		else if(option_selected==3) // only modification we can do in customer record is account type
			modify();
		else if(option_selected==4) // search can be done with username or account_number
			search();
		else if(option_selected==5)
			exit(0);
		else
			printf("Please choose only the operations that are mentioned in the menu\n");
		} // end of infinite while		
				
	} // end of administrator			
	else if(check.customer_type==1)
	{
		printf("--------------welcome normal user----------------\n");
		while(1)
		{
		printf("\n\npress 1 to **Deposit money** into the account\npress 2 to **Withdraw money** from the account\npress 3 for **Balance Enquiry**\npress 4 to for **Password Change**\npress 5 to **View transaction details**\npress 6 to **Exit**\n");
		int option_selected;
		scanf("%d", &option_selected);
		
		if(option_selected==1)
			joint_deposit_money(check.account_number, 0); //passing username's corresponding account_number 
		else if(option_selected==2)
			joint_withdraw_money(check.account_number, 0); // passing username's corresponding account number
		else if(option_selected==3) // sending joint var -- 0
			joint_print_accounts_acc(check.account_number, 0);// print total balance of the given user
		else if(option_selected==4)
			password_change(check.username); // change password
		else if(option_selected==5)
			joint_print_transactions_acc(check.account_number, 0); // print transactions details
		else if(option_selected==6)
			exit(0);
		else
			printf("press one of the given keys from the menu\n");
		} // end of infinite while loop
	}
	else if(check.customer_type==2)
	{
		printf("------------welcome joint account user-----------\n");
		while(1)
		{
		printf("\n\npress 1 to **Deposit money** into the account\npress 2 to **Withdraw money** from the account\npress 3 for **Balance Enquiry**\npress 4 to for **Password Change**\npress 5 to **View transaction details**\npress 6 to **Exit**\n");
		// implementing read lock while viewing account details, balance enquiry
		// implement write lock while depositing money, withdrawing money
		int option_selected;
		scanf("%d", &option_selected);
		
		if(option_selected==1)
			joint_deposit_money(check.account_number, 1); //implement write lock 
		else if(option_selected==2)
			joint_withdraw_money(check.account_number, 1); // write lock
		else if(option_selected==3)
			joint_print_accounts_acc(check.account_number, 1);// read lock -- send joint var - 1
		else if(option_selected==4)
			password_change(check.username);//no lock - both users can change their passwords independently
		else if(option_selected==5)
			joint_print_transactions_acc(check.account_number, 1); // read lock
		else if(option_selected==6)
			exit(0);
		else
			printf("press one of the given keys from the menu\n");
		} // end of infinite while loop
	}


} // end of main method


//***************** SINGLE & JOINT USER OPERATIONS CODE STARTS HERE ********************************//


void joint_print_transactions_acc(char account_number[1024], int joint)
{
	printf("***********transaction ledger of the account********\n");
	printf("Date\t     Account_num\t      credit\t   debit\t   total_balance\n");
	int fd=open("Transaction.txt", O_RDWR, 0744);
	struct Transaction loop;
	while(read(fd, &loop, sizeof(loop)) != 0)
	{
		if( strcmp(loop.account_number, account_number)==0 )
		{
			
			struct flock lock;
			if(joint==1) // its a joint account-- lock it
			{
				lseek(fd, -sizeof(loop), SEEK_CUR); // moving one record back
				lock.l_type = F_RDLCK;
				lock.l_pid = getpid();
				lock.l_whence = SEEK_CUR;
				lock.l_start = 0;  // current plus 0 bytes
				lock.l_len = sizeof(loop);
				fcntl(fd,F_SETLKW,&lock);
			}
			printf("%s\t %s\t %0.2lf\t %0.2lf\t %0.2lf\n", loop.date, loop.account_number, loop.amount_credit, loop.amount_debit, loop.balance_remaining);
			if(joint==1) // joint account -- unlock
			{
				int num;
				while(1)
				{
					printf("press any number other than 0 to unlock\n");
					scanf("%d", &num);
					if(num!=0)
						break;
				}
				lock.l_type = F_UNLCK;
				fcntl(fd,F_SETLK,&lock);
				lseek(fd, sizeof(loop), SEEK_CUR);
			}
    		
		}
	} // end of while
	close(fd);

}


void joint_print_accounts_acc(char account_number[1024], int joint)
{
	printf("**********printing account details************\n");
	int fd=open("Account.txt", O_RDWR, 0744);
	struct Account loop;
	while(read(fd, &loop, sizeof(loop)) != 0)
	{
		
		if(strcmp(loop.account_number, account_number) ==0)
		{
			lseek(fd, -sizeof(loop), SEEK_CUR); // moving one record back
			struct flock lock;
			if(joint==1) // joint account .... so lock
			{
				lock.l_type = F_RDLCK;
				lock.l_pid = getpid();
				lock.l_whence = SEEK_CUR;
				lock.l_start = 0;  // current plus 0 bytes
				lock.l_len = sizeof(loop);
				fcntl(fd,F_SETLKW,&lock);
			}
    		printf("%s\t %0.2f\n", loop.account_number, loop.balance);
    		if(joint==1) // joint account -- unlock
    		{	
				int num;
				while(1)
				{
					printf("press any number other than 0 to unlock\n");
					scanf("%d", &num);
					if(num!=0)
						break;
				}
				lock.l_type = F_UNLCK;
				fcntl(fd,F_SETLK,&lock);
			}
			break;
		}	
	} // end of while
	close(fd);	

}


void joint_deposit_money(char account_number[1024], int joint)
{
	double amount;
	printf("Enter the amount you want to deposit\n");
	scanf("%lf", &amount);
	
	int fd=open("Account.txt", O_RDWR, 0744);
	struct Account loop;
	while(read(fd, &loop, sizeof(loop)) != 0)
	{
		if(strcmp(loop.account_number, account_number)==0)
		{
			// write lock
			lseek(fd, -sizeof(loop), SEEK_CUR); // moving one record back
			struct flock lock;
			if(joint==1) // joint account locking it
			{
				lock.l_type = F_WRLCK;
				lock.l_pid = getpid();
				lock.l_whence = SEEK_CUR;
				lock.l_start = 0;  // current plus 0 bytes
				lock.l_len = sizeof(loop);
				printf("locking for deposit write\n");
				fcntl(fd,F_SETLKW,&lock);
			}
			loop.balance=loop.balance+amount;
			write(fd, &loop, sizeof(loop));
			printf("Amount %0.2lf has successfully deposited\nCurrent balance: %0.2lf\n", amount, loop.balance);
			if(joint==1) //joint accout unlock
			{
				int num;
				while(1)
				{
					printf("press any number other than 0 to unlock\n");
					scanf("%d", &num);
					if(num!=0)
						break;
				}
				lock.l_type = F_UNLCK;
				fcntl(fd,F_SETLK,&lock);
			}	
			int tfd=open("Transaction.txt", O_RDWR, 0744);
			// formatting date time to insert into Transaction.txt
			char cur_time[128];
  		  	time_t t;
		  	struct tm* ptm;
		    t = time(NULL);
		  	ptm = localtime(&t);
			strftime(cur_time, 128, "%d-%b-%Y %H:%M:%S", ptm);
			//printf("current date  and time is %s\n", cur_time);
			struct Transaction trans;
			
			lseek(tfd, 0, SEEK_END);
			//while(read(tfd, &trans, sizeof(trans)) != 0);
			// the loop breaks at the end of file
			
			strcpy(trans.date, cur_time);
			strcpy(trans.account_number, account_number);
			trans.amount_credit=amount;
			trans.amount_debit=0;
			trans.balance_remaining=loop.balance;
			write(tfd, &trans, sizeof(trans));
			close(tfd);
			
			break;
		}
				
	} // end of while
	close(fd);
	//print_customers();
	//print_accounts();
	//print_transactions();
	joint_print_transactions_acc(account_number, 0);	

} // joint deposit money

void joint_withdraw_money(char account_number[1024], int joint)
{
	double amount;
	printf("Enter the amount you want to withdraw\n");
	scanf("%lf", &amount);
	
	int fd=open("Account.txt", O_RDWR, 0744);
	struct Account loop;
	while(read(fd, &loop, sizeof(loop)) != 0)
	{
		if(strcmp(loop.account_number, account_number)==0)
		{
			if(loop.balance-amount<0) // insufficient funds
			{
				printf("Insufficient balance\nYou have only %0.2lf amount in your account\n", loop.balance);
				return;
			}
			lseek(fd, -sizeof(loop), SEEK_CUR); // moving one record back
			struct flock lock;
			if(joint==1) // joint account lock
			{
				lock.l_type = F_WRLCK;
				lock.l_pid = getpid();
				lock.l_whence = SEEK_CUR;
				lock.l_start = 0;  // current plus 0 bytes
				lock.l_len = sizeof(loop);
				printf("locking for withdrawal write\n");
				fcntl(fd,F_SETLKW,&lock);
			}
			loop.balance=loop.balance-amount;
			write(fd, &loop, sizeof(loop));
			printf("Amount %0.2lf has successfully withdrawn\nCurrent balance: %0.2lf\n", amount, loop.balance);
			if(joint==1) // joint account unlock
			{
				int num;
				while(1)
				{
					printf("press any number other than 0 to unlock\n");
					scanf("%d", &num);
					if(num!=0)
						break;
				}
				lock.l_type = F_UNLCK;
				fcntl(fd,F_SETLK,&lock);
			}
			int tfd=open("Transaction.txt", O_RDWR, 0744);
			// formatting date time to insert into Transaction.txt
			char cur_time[128];
  		  	time_t t;
		  	struct tm* ptm;
		    t = time(NULL);
		  	ptm = localtime(&t);
			strftime(cur_time, 128, "%d-%b-%Y %H:%M:%S", ptm);
			//printf("current date  and time is %s\n", cur_time);
			struct Transaction trans;
			
			lseek(tfd, 0, SEEK_END);
			//while(read(tfd, &trans, sizeof(trans)) != 0);
			// the loop breaks at the end of file
			
			strcpy(trans.date, cur_time);
			strcpy(trans.account_number, account_number);
			trans.amount_credit=0;
			trans.amount_debit=amount;
			trans.balance_remaining=loop.balance;
			write(tfd, &trans, sizeof(trans));
			close(tfd);
			
			break;
		}
				
	} // end of while
	close(fd);
	//print_customers();
	//print_accounts();
	joint_print_transactions_acc(account_number, 0);
	
} // joint withdraw method end

void password_change(char username[1024])
{
	
	int fd=open("Customer.txt", O_RDWR, 0744);
	struct Customer check;
	while(read(fd, &check, sizeof(check)) != 0)
	{
		if( strcmp(check.username, username)==0 )
		{
			char password1[1024], password2[1024];
			printf("Enter the new password\n");
			scanf("%s", password1);
			printf("Re-enter the new password\n");
			scanf("%s", password2);
			if( strcmp(password1, password2) == 0) // change password logic
			{
				strcpy(check.password, password1);
				lseek(fd, -sizeof(check), SEEK_CUR);
				write(fd, &check, sizeof(check));
				printf("successfully changed the password for user : %s\n", username);
				break;
			}
			else
			{
				printf("passwords entered doesn't match with each other\n");
				break;
			}
		}

	} // end of while
	close(fd);
	print_customers();
} // password_change method end


//***************** SINGLE & JOINT USER OPERATIONS CODE ENDS HERE *********************************//


//********************  ADMIN OPERATIONS  CODE STARTS HERE ************************************//

void search()
{	
	int option_selected;
	printf("press 1 to search for username+account details, press 2 to just search for account details\n");
	scanf("%d", &option_selected);
	if(option_selected==1) // print customer + account details
	{
		char username[1024];
		int found=0; // no record found
		printf("Admin - Enter the username of the account you want to search\n");
		scanf("%s", username);
		printf("***********customer details + account details***********\n");
		char account_number[1024];
		int fd=open("Customer.txt",O_RDWR, 0744);
		struct Customer print;
		while(read(fd, &print, sizeof(print)) != 0)
		{
			if(strcmp(print.username, username)==0 && print.customer_type!=5)
			{
				printf("%s\t%s\t%s\t%d\n",print.username,print.password,print.account_number, print.customer_type);
				strcpy(account_number,print.account_number);
				found=1;
				break;
			}
		} // end of while
		close(fd);
		if(found==0)
		{
			printf("The customer with entered username doesn't exist in the database\n");
			return;
		}
		
		struct Account loop;
		fd=open("Account.txt", O_RDWR, 0744);
		while(read(fd, &loop, sizeof(loop)) != 0)
		{
			if( strcmp(loop.account_number, account_number) ==0)
				printf("%s\t %f\n", loop.account_number, loop.balance);		
		} // end of while
		close(fd);
		
	}
	else if(option_selected==2) // print just account details
	{
		char account_number[1024];
		printf("Admin - Enter the account number you want to search for\n");
		scanf("%s", account_number);
		printf("The account details are:\t");
		struct Account loop;
		int fd=open("Account.txt", O_RDWR, 0744);
		while(read(fd, &loop, sizeof(loop)) != 0)
		{
			if( strcmp(loop.account_number, account_number) ==0)
				printf("%s\t %f\n", loop.account_number, loop.balance);		
		} // end of while
		close(fd);
		
	}

}

void modify()
{
	char username[1024];
	printf("Admin - Enter the username of the account you want to modify\n");
	scanf("%s", username);
	
	int mod=0;
	printf("press 0 to change the password\npress 1 to modify the single user to joint account\n");
	scanf("%d", &mod);
	if(mod==0)
		password_change(username);
	else if(mod==1) // modifying
	{
		int fd=open("Customer.txt", O_RDWR, 0744);
		struct Customer check;
		// search for the username, if customer found, retrieve the customer_type
		int found=0; // currently username not found
		while(read(fd, &check, sizeof(check)) != 0)
		{
			if( strcmp(check.username, username)==0 && check.customer_type==2)
			{
				printf("Account is already a joint account\n");
				found=1;
			}
			else if( strcmp(check.username, username)==0 && check.customer_type==1)
			{
				check.customer_type=2; // changing single user account to joint account
				//put below two lines after adding second customer (advice)-- after close(sec_fd) -- line 653
				lseek(fd, -sizeof(check), SEEK_CUR);
				write(fd, &check, sizeof(check));
				found=1;
				
				// Inserting new joint account second customer
				printf("Admin--Enter following details to add the 2nd customer details of joint account\n");
				struct Customer second_user;
				
				while(1)
				{
					printf("Enter the username\t");  
					// check user name availability here..... use while loop to allow user to enter unique username
					// while loop use -- to find duplicate and ask again to enter unique username
					scanf("%s", second_user.username);
					int found=0; // username is unique . that is not found
					int lfd=open("Customer.txt", O_CREAT|O_RDWR, 0744);
					struct Customer iterate;
					while(read(lfd, &iterate, sizeof(iterate)) != 0)
					{
						if(strcmp(iterate.username, second_user.username)==0)
						{
							printf("User name already exists in the database. Enter unique username\n");
							found=1;
						}		
					} // end of inner while
					close(lfd);
					if(found==0)
						break;
				}
				
				printf("\nEnter password you want to set for your account\t");
				scanf("%s", second_user.password);
				strcpy(second_user.account_number, check.account_number);
				second_user.customer_type=2;
				int sec_fd=open("Customer.txt", O_CREAT|O_RDWR, 0744);
				
				int del_found=0;
				struct Customer loop;
				while(read(sec_fd, &loop, sizeof(loop)) != 0)
				{
					if(loop.customer_type==5)
					{
						printf("deleted account/ duplicate account found and modifying it\n");
						del_found=1;
						break;
					}
							
				} // end of while
				if(del_found==0) // if no deleted record is found
					write(sec_fd, &second_user, sizeof(second_user));
				else // overriding on the deleted record place
				{
					// our details are present in second_user customer
					lseek(sec_fd, -sizeof(loop), SEEK_CUR);
					write(sec_fd, &second_user, sizeof(second_user));
				}	
				close(sec_fd);	
				// no need to add the account details once again for joint user 		
			}
		} // end of while
		if(found==0)
			printf("Entered username doesn't match or doesn't exist in the database\n");
			
		close(fd);
	} // else if mod end
	
	print_customers(); // calling print customers and acconts method
		
}	
	
void add_account()
{
	// create customer struct object and Account struct object
	printf("Admin--Enter following details to add the customer into the banking services\n");
	struct Customer new_user;
	
	while(1)
	{
		printf("Enter the username\t");  
		// check user name availability here..... use while loop to allow user to enter unique username
		// while loop use -- to find duplicate and ask again to enter unique username
		scanf("%s", new_user.username);
		int found=0; // username is unique . that is not found
		int fd=open("Customer.txt", O_CREAT|O_RDWR, 0744);
		struct Customer iterate;
		while(read(fd, &iterate, sizeof(iterate)) != 0)
		{
			if(strcmp(iterate.username, new_user.username)==0)
			{
				printf("User name already exists in the database. Enter unique username\n");
				found=1;
			}		
		} // end of inner while
		close(fd);
		if(found==0)
			break;
	}
	printf("\nEnter password you want to set for your account\t");
	scanf("%s", new_user.password);
	//  for account number/// use username + date time trimmed version	
	// we gonna have unique username ... so using username to build account number
	char acc[1024];
	strcat(acc, "SBH-TS-WNP-00016-");// state-district-branchcode-username
	strcat(acc, new_user.username);
	strcpy(new_user.account_number, acc);
	printf("\n Enter customer type  single account user-1, joint account user-2\t");
	scanf("%d", &new_user.customer_type);
	
	//********************** adding new customer*****************************************
	int fd=open("Customer.txt", O_CREAT|O_RDWR, 0744);
	// find the first record that is duplicate or deleted.    customer_type= 5
	int del_found=0; // no duplicate or deleted record as of now
	struct Customer loop;
	while(read(fd, &loop, sizeof(loop)) != 0)
	{
		if(loop.customer_type==5)
		{
			printf("deleted customer record/ duplicate record found and modifying it\n");
			del_found=1;
			break;
		}
				
	} // end of while
	
	if(del_found==0) // if no deleted record is found
		write(fd, &new_user, sizeof(new_user));
	else // overriding on the deleted record place
	{
		// our details are present in new_user customer
		lseek(fd, -sizeof(loop), SEEK_CUR);
		write(fd, &new_user, sizeof(new_user));
	}		
	close(fd);
	
	// ************** adding account number*****************************
	struct Account new_account;
	strcpy(new_account.account_number, new_user.account_number);
	new_account.balance=0.0;
	fd=open("Account.txt", O_CREAT|O_RDWR, 0744);
	del_found=0; // no duplicate or deleted record as of now
	struct Account print;
	while(read(fd, &print, sizeof(print)) != 0)
	{
		if(strcmp(print.account_number,"0000")==0 )// 4 zeroes in account_number signifies that deleted account
		{
			printf("deleted account/ duplicate account found and modifying it\n");
			del_found=1;
			break;
		}		
	} // end of while
	if(del_found==0) // if no deleted record is found
		write(fd, &new_account, sizeof(new_account));
	else // overriding on the deleted record place
	{
		// our details are present in new_user customer
		lseek(fd, -sizeof(print), SEEK_CUR);
		write(fd, &new_account, sizeof(new_account));
	}		
	close(fd);
	
	if(new_user.customer_type==2) // enter joint account details
	{
		// extra customer addition for joint account
		printf("Admin--Enter following details to add the 2nd customer details of joint account\n");
		struct Customer second_user;
		//printf("Enter the username\t");
		//scanf("%s", second_user.username);
		
		while(1)
		{
			printf("Enter the username\t");  
			// check user name availability here..... use while loop to allow user to enter unique username
			// while loop use -- to find duplicate and ask again to enter unique username
			scanf("%s", second_user.username);
			int found=0; // username is unique . that is not found
			int fd=open("Customer.txt", O_CREAT|O_RDWR, 0744);
			struct Customer iterate;
			while(read(fd, &iterate, sizeof(iterate)) != 0)
			{
				if(strcmp(iterate.username, second_user.username)==0)
				{
					printf("User name already exists in the database. Enter unique username\n");
					found=1;
				}		
			} // end of inner while
			close(fd);
			if(found==0)
				break;
		}
		
		printf("\nEnter password you want to set for your account\t");
		scanf("%s", second_user.password);
		strcpy(second_user.account_number, new_user.account_number);
		second_user.customer_type=2;
		fd=open("Customer.txt", O_CREAT|O_RDWR, 0744);
		
		int del_found=0;
		struct Customer loop;
		while(read(fd, &loop, sizeof(loop)) != 0)
		{
			if(loop.customer_type==5)
			{
				printf("deleted account/ duplicate account found and modifying it\n");
				del_found=1;
				break;
			}
					
		} // end of while
		if(del_found==0) // if no deleted record is found
			write(fd, &second_user, sizeof(second_user));
		else // overriding on the deleted record place
		{
			// our details are present in second_user customer
			lseek(fd, -sizeof(loop), SEEK_CUR);
			write(fd, &second_user, sizeof(second_user));
		}		
		close(fd);
		// no need to add the account details once again for joint user 
	}
	
	print_customers();
	
} // ********** end of add_account method***********


void delete_account( char account_number[]) // still be treated as pointer. same as ( char *account_number)
{
	int fd=open("Account.txt", O_RDWR, 0744);
	int del_found=0; // no duplicate or deleted record as of now
	struct Account loop;
	printf("*******INSIDE DELETE ACCOUNT NUMBER method*******\n");
	while(read(fd, &loop, sizeof(loop)) != 0)
	{
		if(strcmp(loop.account_number, account_number)==0 )// when there is match
		{
			printf("\nthe record to be deleted is found\n");
			del_found=1;
			break;
		}		
	} // end of while
	if(del_found==0) // if no deleted record is found
		printf("no such account number exists in the database\n");
	else // overriding on the deleted record place
	{
		int consent=0; // default delete
		if(loop.balance>0.0) // reconfirming if the deleted account has some balance
		{
			
			printf("current balance: %f. Press *1* if you don't want to delete the account\n", loop.balance);
			scanf("%d", &consent);
		}
		if(consent!=1)
		{
			strcpy(loop.account_number, "0000");    // 4 zeroes in account number signifies deleted account
			loop.balance=0.0;
			lseek(fd, -sizeof(loop), SEEK_CUR);
			write(fd, &loop, sizeof(loop));
		}
	}		
	close(fd);
	
	// find corresponding usernames and delete them from Customers.txt
	fd=open("Customer.txt", O_RDWR, 0744);
	struct Customer print;
	while(read(fd, &print, sizeof(print)) != 0)
	{
		if( strcmp(account_number, print.account_number)==0 && (print.customer_type==1 || print.customer_type==2) )
		{
			delete_username(print.username);
		}			
	} // end of while
	close(fd);
}  // ****************** end of delete_account method **********************


void delete_username(char username[])
{
	int fd=open("Customer.txt", O_RDWR, 0744);
	int del_found=0;
	struct Customer loop;
	while(read(fd, &loop, sizeof(loop)) != 0)
	{
		// account type shouldn't be 5... we don't want to delete already deleted account
		if( strcmp(loop.username, username)==0 && (loop.customer_type==1 || loop.customer_type==2) )
		{
			printf("username to be deleted is found\n");
			del_found=1;
			break;
		}			
	} // end of while
	
	if(del_found==0)
		printf("No user exists with entered username\n");
	else if(del_found==1 && loop.customer_type==1) // del is found and normal single user
	{
		// delete user and account
		loop.customer_type=5; // 5 in customer_type signifies deleted account
		lseek(fd, -sizeof(loop), SEEK_CUR);
		write(fd, &loop, sizeof(loop));
		delete_account(loop.account_number); // deleting corresponding account from Account.txt
		close(fd);
	} // end of else if -- single user
	else if(del_found==1 && loop.customer_type==2) // del is found and joint account 
	{
		char joint_account_number[1024];
		strcpy(joint_account_number, loop.account_number); // copying account number
		// delete current user
		loop.customer_type=5;
		lseek(fd, -sizeof(loop), SEEK_CUR);
		write(fd, &loop, sizeof(loop));
		close(fd);
		//finding if the joint user still in the database or deleted prior to this current user deletion
		int sd=open("Customer.txt", O_RDWR, 0744);
		int sec_found=0; // joint user not found till now
		struct Customer sec_loop;
		while(read(sd, &sec_loop, sizeof(sec_loop)) != 0)
		{
			// account type shouldn't be 5        if joint user exists
			if( strcmp(sec_loop.account_number, joint_account_number)==0  && sec_loop.customer_type==2 )
			{
				printf("joint account holder curretly exists in the database\n");
				sec_found=1;
				break;
			}			
		} // end of while
		if(sec_found==1)
			printf("No need for cascading delete of account number from Account.txt\n");
		else
			delete_account(joint_account_number);
		
	} // end of else if -- joint user
	
}

void delete()
{
	int option_selected;
	printf("press 1 to delete using account number,  press 2 to delete using username\n");
	scanf("%d", &option_selected);
		
	if(option_selected==1) // delete using account number
	{
		printf("Enter the account number to be deleted:\t");
		char account_number[1024];
		scanf("%s", account_number);
		delete_account(account_number);
	}		
	else if(option_selected==2) // delete using username -- handle joint account case too
	{
		printf("Enter the username to delete\n");
		char username[1024];
		scanf("%s", username);
		delete_username(username);
	}
		
	print_customers();
	print_accounts();
}	



//********************  ADMIN OPERATIONS  CODE ENDS HERE ************************************//

// *******************  PRINTING FUNCTIONS START **************************************//

void print_customers()
{		
	printf("***********printing customers***********\n");
	int fd=open("Customer.txt",O_RDWR, 0744);
	struct Customer print;
	while(read(fd, &print, sizeof(print)) != 0)
	{
		if(print.customer_type!=5)
			printf("%s\t%s\t%s\t%d\n",print.username,print.password,print.account_number, print.customer_type);
	} // end of while
	close(fd);
} // end of print customers


void print_accounts()
{
	printf("**********printing accounts************\n");
	int fd=open("Account.txt", O_RDWR, 0744);
	struct Account loop;
	while(read(fd, &loop, sizeof(loop)) != 0)
	{
		if(strcmp(loop.account_number, "0000") !=0)
			printf("%s\t %0.2f\n", loop.account_number, loop.balance);		
	} // end of while
	close(fd);	
} // end of print accounts


void print_transactions()
{
	printf("***********all transactions ledger********\n");
	printf("Date\t     Account_num\t      credit\t   debit\t   total_balance\n");
	int fd=open("Transaction.txt", O_RDWR, 0744);
	struct Transaction loop;
	while(read(fd, &loop, sizeof(loop)) != 0)
	{
		printf("%s\t %s\t %0.2lf\t %0.2lf\t %0.2lf\n", loop.date, loop.account_number, loop.amount_credit, loop.amount_debit, loop.balance_remaining);
	} // end of while
	close(fd);
} // end of print transactions method


// ******************* PRINTING FUNCTIONS END *************************//

