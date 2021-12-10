// client side

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdbool.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<time.h>
#include "structures.c"


void add_account(int nfd);
void modify(int nfd);				
void search(int nfd);		
void delete_account(char account_number[1024], int nfd);				
void delete_username(char username[1024], int nfd);					
void delete(int nfd);		
// printing methods
void print_customers(int nfd);
void print_accounts(int nfd);
void print_transactions(int nfd);		
				
void password_change(char username[1024], int nfd);

void joint_deposit_money(char account_number[1024], int joint, int nfd);
void joint_withdraw_money(char acc_num[1024], int joint,int nfd);
void joint_print_accounts_acc(char account_number[1024], int joint, int nfd);
void joint_print_transactions_acc(char acc_num[1024], int type, int nfd);	


void main()
{
	char username[1024], password[1024];
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	int login_check,type;
	struct sockaddr_in client;
	int fd, msg_length;
	char buff[60];
	char result;
	if( (fd=socket(AF_INET, SOCK_STREAM, 0)) ==-1)
	{
		perror("\nerror creating socket ");
		exit(1);
	}
	client.sin_family= AF_INET; //address family 
	client.sin_addr.s_addr=inet_addr("127.0.0.1"); //inet_addr("127.0.0.1"); //Internet Addres
	client.sin_port= htons(3856); //port number 
	write(1, "\nWaiting to Connect to the Server    ", sizeof("\nWaiting to Connect to the Server    "));
	if( connect(fd, (struct sockaddr *)&client, sizeof(client))==-1 )
	{
		perror("\nerror at connect call ");
		exit(1);
	}				
	read(fd, buff, sizeof(buff));
	printf("%s", buff);
	printf("------------------Welcome to STATE BANK OF HYDERABAD------------------------------\n");
	printf("\nEnter your username and password to login to your account:\nUsername: ");
	scanf("%s", username);
	printf("\nPassword: ");							
	scanf("%s", password);	
	write(fd, username, sizeof(username));
	write(fd, password, sizeof(password));
	read(fd, &login_check, sizeof(login_check)); // reading from server .... login check validatin status
	if(login_check==0) // if entered username and password does not  match
	{
		printf("\nUsername and password entered doesn't match with any existing user\n");
		close(fd);	
		exit(1);
	}
	read(fd, &type, sizeof(type)); // reading the customer_type from server
	if(type == 0)
	{	
		while(1)
		{
		printf("\n********************Welcome ADMIN********************\n");
		printf("\npress 1 to Add an account\n press 2 to delete account\npress 3 to modify account\n press 4 to Search for an account\npress 5 print Customers\n press 6 to Exit\n");
		getchar();
		
		int choice;
		scanf("%d", &choice);
		
		write(fd, &choice, sizeof(choice));
		if(choice==1) 
			add_account(fd);
		else if(choice==2) 
			delete(fd);
		else if(choice==3)
			modify(fd);
		else if(choice==4)
			search(fd);
		else if(choice==5)
			print_customers(fd);
		else if(choice==6){						
			close(fd);
			exit(1);					
		}
		}			
	}
	else if(type==1) // for single account customer
	{
		while(1)
		{
		printf("\n****************welcome normal single account user*****************");
		printf("\npress 1 to deposit money\npress 2 to Withdraw money\npress 3 to check balance\npress 4 to change password\npress 5 to view transactions\npress 6 to Exit\n");				
		
		struct Customer check;
		
		int choice;
		scanf("%d", &choice);
		write(fd, &choice, sizeof(choice));
		read(fd, &check, sizeof(check));
					
		if(choice==1)						
			joint_deposit_money(check.account_number, check.customer_type, fd); //sending account_number and then askinng other details 
		else if(choice==2)					
			joint_withdraw_money(check.account_number, check.customer_type, fd);						
		else if(choice==3)					
			joint_print_accounts_acc(check.account_number, check.customer_type, fd);						
		else if(choice==4)						
			password_change(check.username, fd);					
		else if(choice==5)	// print transactions details					
			 joint_print_transactions_acc(check.account_number, check.customer_type, fd);
		else if(choice==6){						
			close(fd);
			exit(1);					
		}
	     }
	}
	else if(type==2) // for joint account customer
	{
		while(1)
		{
			printf("\n**************welcome joint account user*******************");				
			printf("\npress 1 to deposit money\npress 2 to Withdraw money\npress 3 to check balance\npress 4 to change password\npress 5 to view transactions\npress 6 to Exit\n");				
			
			struct Customer check;
			int choice;
			scanf("%d", &choice);
			write(fd, &choice, sizeof(choice));
			read(fd, &check, sizeof(check));
			
			if(choice==1)						
				joint_deposit_money(check.account_number, check.customer_type, fd);
			else if(choice==2)					
				joint_withdraw_money(check.account_number, check.customer_type, fd);						
			else if(choice==3)					
				joint_print_accounts_acc(check.account_number, check.customer_type, fd);						
			else if(choice==4)						
				password_change(check.username, fd);					
			else if(choice==5)	// print transactions details					
				 joint_print_transactions_acc(check.account_number, check.customer_type, fd);
			else if(choice==6){						
				close(fd);
				exit(1);					
			}
	
		}
	}

	close(fd);	
} // main method end




void joint_deposit_money(char account_number[1024], int joint, int nfd)
{
	if(joint==1) // single account user deposit
	{
		double amount;						
		printf("\nEnter the amount you want to credit:  ");					
		scanf("%lf", &amount);						
		fflush(stdin);
		write(nfd, &amount, sizeof(amount));
		int found;
		read(nfd, &found, sizeof(found));
		if(found==1)
		{
			double bal;
			read(nfd, &bal, sizeof(bal));
			printf("\namount %0.2lf has successfully deposited \t Current balance: %0.2lf", amount, bal);	
		}
	} // single account user deposit end
	else //// joint account deposit
	{
		char buffer[1024];
		double amount;
		int found=0, r, unlock=1;
		struct Account temp;
		
		printf("\nEnter the amount you want to credit:  ");
		scanf("%lf", &amount);
		fflush(stdin);
		
		write(nfd, &amount, sizeof(amount));
		read(nfd, &found, sizeof(found)); // reading account found status from server side
		
		if(found==1)
		{		
			printf("\nLocking the record for deposit write  \n");
			read(nfd, &temp.balance, sizeof(temp.balance)); // reading account balance from server side
			
			while(1)
			{
				r=read(nfd, buffer, sizeof(buffer)); // reading print statement from server side
				write(1, buffer, r); //statement print from server side... printing on client terminal
				scanf("%d", &unlock);
				//fflush(stdin);
				write(nfd, &unlock, sizeof(unlock));
				if(unlock!=0)
					break;
			}// infinite lock loop end
			printf("\namount %0.2lf has successfully credit into the account\t Current balance: %0.2lf", amount, temp.balance);
		}
	} // else end -- joint account
} // joint deposit money

void joint_withdraw_money(char account_number[1024], int joint, int nfd)
{
	if(joint==1) // single user account
	{
		double amount;								
		printf("\nEnter the amount you want to debit:  ");					
		scanf("%lf", &amount);						
		write(nfd, &amount, sizeof(amount));
		int found;
		read(nfd, &found, sizeof(found));
		if(found==1)
		{
			double balance;
			read(nfd, &balance, sizeof(balance));
			printf("\namount %0.2lf has successfully withdrawn \t Current balance: %0.2lf", amount, balance);	
		}
		else if(found==2)
			printf("\nInsufficient balance");
		
	} // single user account if statement end
	else // joint account withdrawl
	{
		double amount;							
		int found=0, unlock=0, correct=0;
		char buffer[1024];									
		printf("\nEnter the amount to withdraw from the account:  ");					
		scanf("%lf", &amount);						
		fflush(stdin);
		write(nfd, &amount, sizeof(amount)); // sending amount to withdraw to server
		read(nfd, &found, sizeof(found));	 // reading account found status from server			
		if(found==1) // account is present in the database
		{
			read(nfd, &correct, sizeof(correct));
			if(correct==1)
			{
				printf("\nInsufficient funds to withdraw  ");
				return;
			}
			write(1, "\nLocking the record for withdraw write  \n", sizeof("\nLocking the record for withdraw write  \n"));
			struct Account temp;
			// duplicate statement at 287 after while loop because below line was reading garbage
			read(nfd, &temp.balance, sizeof(temp.balance)); // read balance from server
			while(1)
			{
				memset(buffer, 0, sizeof(buffer));
				int bytes_read=read(nfd, &buffer, sizeof(buffer)); // reading print statement from server
				write(1, &buffer, bytes_read);
				scanf("%d", &unlock);
				//fflush(stdin);
				write(nfd, &unlock, sizeof(unlock)); // sending unlock to server
				if(unlock!=0)
					break;
			} // infinite unlock while loop
			fflush(stdout);
			read(nfd, &temp.balance, sizeof(temp.balance));  // duplicate read balance from server
			printf("\namount %0.2lf has successfully withdrawn from the account\t Current balance: %0.2lf", amount, temp.balance);
		}
	} // end of else joint account withdrawl
}

void joint_print_accounts_acc(char account_number[1024], int joint, int nfd)
{
	printf("\n*************Printing Account Details***************\nAccount number\t\tBalance");	
	fflush(stdin);
	
	if(joint==1)
	{
		struct Account temp;
		int done;
		
		read(nfd, &done, sizeof(done));
		if(done!=0) // account number is found in the database
		{
			read(nfd, &temp, sizeof(temp));
			printf("\n%s\t %0.2lf ", temp.account_number, temp.balance);
		}
		else
			printf("\naccount number doesn't exist in the database\n");
	}// single account ending here
	else // joint account print account details
	{
		struct Account temp;
		int found=0,r, unlock=0;
		read(nfd, &found, sizeof(found)); // reading account found status from server
		if(found==1)
		{
			//printf("\nLocking the record --read lock");
			read(nfd, &temp, sizeof(temp)); // reading the account record
			printf("\n%s\t %0.2lf", temp.account_number, temp.balance);
			
			while(1)
			{
				printf("\nEnter any number other than 0 to unlock: ");
				scanf("%d", &unlock);
				fflush(stdin);
				write(nfd, &unlock, sizeof(unlock));
				if(unlock!=0)
					break;
			}
		}	
	} // joint account print end

}

void joint_print_transactions_acc(char account_number[1024], int joint, int nfd){

	write(1, "\ndate\t     account number\t      credit\t   debit\t   total balance", sizeof("\ndate\t     account_number\t      credit\t   debit\t   total balance"));
	
	struct Transaction temp;	
	if(joint==1){
		int done;
		read(nfd, &done, sizeof(done));
		while(1){
			if(done!=0){
				read(nfd, &temp, sizeof(temp));
				 printf("\n%s\t\t %s\t %0.2lf\t %0.2lf\t %0.2lf\n", temp.date, temp.account_number, temp.amount_credit, temp.amount_debit, temp.balance_remaining);
				read(nfd, &done, sizeof(done));
			}else{
				break;
			}
		}
		
	}else{
	
		int unlock=0, r, temp1=0;
		int done;
		char buff[100];
		read(nfd, &done, sizeof(done));
		while(1){
			if(done!=0){
				
				/*printf("\nLocking the Joint record for Print ");
				
				read(nfd, &temp, sizeof(temp));
				
				printf("\n%s\t %s\t %0.2lf\t %0.2lf\t %0.2lf", temp.date, temp.account_number, temp.amount_credit, temp.amount_debit, temp.balance_remaining);
				
				printf("\nEnter any number other than 0 to unlock: ");
				scanf("%d", &unlock);
				fflush(stdin);
				
				write(nfd, &unlock, sizeof(unlock));
				
				
				printf("tee");
				*/
				read(nfd, &temp, sizeof(temp));
				
				printf("\n%s\t %s\t %0.2lf\t %0.2lf\t %0.2lf", temp.date, temp.account_number, temp.amount_credit, temp.amount_debit, temp.balance_remaining);
				read(nfd, &done, sizeof(done));
			}
			else{
				break;
			}
			
		}
		//--	
	}
}
//For Admin
void password_change(char username[1024], int nfd)						
{						
	char password1[1024], password2[1024];						
	struct Customer check;
	int found, done;
	read(nfd, &found, sizeof(found));
	if(found==1)
	{
		memset(password1, 0, sizeof(password1));
		printf("\nEnter the new password: ");			
		scanf("%s", password1);
		memset(password2, 0, sizeof(password2));				
		printf("\nRe-enter the new password: ");			
		scanf("%s", password2);
		
		write(nfd, &password1, sizeof(password1));
		write(nfd, &password2, sizeof(password2));
		
		read(nfd, &done, sizeof(done));
		if(done==1)
			printf("\nPassword changed successfully....");			
		else
			printf("\nPasswords entered does not match ");					
	}		
	print_customers(nfd);
} 

void print_customers(int nfd)
{								
	printf("\n***********Printing Customers***********\nUser_name\t password\t\tBank_Acc\t\tType");								
	int done;
	struct Customer print;	
	read(nfd, &done, sizeof(done));
	while(1)
	{
		if(done!=0)
		{
			read(nfd, &print, sizeof(print));
			printf("\n%s\t\t%s\t\t%s\t\t%d",print.username, print.password, print.account_number, print.customer_type);
			read(nfd, &done, sizeof(done));
		}
		else
			break;
	}
}

void print_accounts(int nfd)
{		
	printf("\n**********Printing Accounts************\nAccount Number\t\t\tBalance");
	int done;
	int fd=open("Account.txt", O_RDWR, 0744);
	struct Account print1;
	read(nfd, &done, sizeof(done));
	while(1)
	{
		if(done!=0)
		{
			read(nfd, &print1, sizeof(print1));
			printf("\n%s\t\t%f", print1.account_number, print1.balance);
			read(nfd, &done, sizeof(done));
		}
		else
			break;
	}
} 

void modify(int nfd)
{
	char username[1024];
	memset(username, 0, sizeof(username));
	printf("\nEnter the Username: ");				
	scanf("%s", username);	
	write(nfd, &username, sizeof(username));							
							
	int choice=0;				
	printf("\npress 1 to modify the single user to joint account\npress 2 to change password\n");
	scanf("%d", &choice);												
	write(nfd, &choice, sizeof(choice));	
						
	if(choice==1){			
		struct Customer temp;					
		
		int found=0; 
		read(nfd, &found, sizeof(found));
		
		if(found==1){
			printf("\nAccount is already a joint Account");
			return;	
		}else if(found==0){
			printf("\nNo record exist with the given Username\n");
			return;
		}
		
		struct Customer second_user;
		
		printf("\nEnter details of the second user of the joint account");	
		struct Customer temp2;
		while(1){
			memset(second_user.username, 0, sizeof(second_user.username));					
			printf("\nEnter the username: ");  						
			scanf("%s", second_user.username);					
			write(nfd, &second_user.username, sizeof(second_user.username));	
								
			int found=0; // username is unique . that is not found
			read(nfd, &found, sizeof(found)); 
			if(found==1){
				printf("\nUser Name Already Exists. Please, Enter a Unique Username ");
			}else if(found==0){
				break;}
		}	
		memset(second_user.password, 0, sizeof(second_user.password));				
		printf("\nEnter password : ");					
		scanf("%s", second_user.password);		
		write(nfd, &second_user.password, sizeof(second_user.password));
		
		
	}else{
		password_change(username, nfd);
	} 
	
	print_customers(nfd); 
	print_accounts(nfd);	
}	
	
void search( int nfd)
{	
	int choice;
	printf("\npress 1 to Search for Username + Account Details\npress 2 to Just Search for Account Details\n");
	scanf("%d", &choice);
	write(nfd, &choice, sizeof(choice));
	if(choice==1) // print customer + account details
	{
		char username[1024];								
		int found=0; // no record found				
		memset(username, 0, sizeof(username));							
		printf("\nEnter the Username: ");					
		scanf("%s", username);							
		write(nfd, &username ,sizeof(username));				
		printf("\n************* Customer & Account details *************");
		printf("\nUser_name\t password\t\tBank_Acc\t\tType\tBalance");			
		char account_number[1024];									
		struct Customer temp;
		read(nfd, &found, sizeof(found));	
		if(found==1)
		{
			read(nfd, &temp, sizeof(temp));					
			printf("\n%s\t\t%s\t%s\t%d\t",temp.username, temp.password, temp.account_number, temp.customer_type);				
			strcpy(account_number, temp.account_number);					
		}				
		else
		{
			printf("\nNo Record exists With the given Username ");				
			return;
		}		
		struct Account temp1;			
		read(nfd, &found, sizeof(found));
		if(found==2)
		{
			read(nfd, &temp1, sizeof(temp1));					
			printf("\t%f\n", temp1.balance);										
		}					
	}					
	else if(choice==2) // prinnting only account details 
	{									
		char account_number1[1024];				
		memset(account_number1, 0, sizeof(account_number1));					
		printf("\nEnter the Account Number: ");			
		scanf("%s", account_number1);						
		write(nfd, &account_number1 ,sizeof(account_number1));
		printf("\n************ Account Details *************");					
		printf("\nBank account \t\t\tBalance");
		struct Account temp2;									
		int found=0;
		read(nfd, &found, sizeof(found));
		if(found==3)
		{
			read(nfd, &temp2, sizeof(temp2));					
			printf("\n%s\t\t%f\n", temp2.account_number, temp2.balance);									
		}
		else
		{
			printf("\nNo Record exists With the given Account");				
			return;
		}								
	}						
}	

void add_account(int nfd)						
{			
	struct Customer new;						
	printf("\nEnter details of the customer");	
	struct Customer temp2;
	while(1)
	{
		memset(new.username, 0, sizeof(new.username));							
		printf("\nEnter the username: ");  						
		scanf("%s", new.username);					
		write(nfd, &new.username, strlen(new.username)+1);					
		int found=0; // username is unique . that is not found
		read(nfd, &found, sizeof(found));
		if(found==1)
			printf("\nUser Name Already Exists. Please, Enter a Unique Username ");
		if(found==0)
			break;
	}
	memset(new.password, 0, sizeof(new.password));					
	printf("\nEnter password : ");					
	scanf("%s", new.password);				
	write(nfd, &new.password, sizeof(new.password));				
	printf("\npress 1 for single account user\npress 2 for joint account user\n");				
	scanf("%d", &new.customer_type);				
	write(nfd, &new.customer_type, sizeof(new.customer_type));
	if(new.customer_type==2) // enter joint account details
	{
		struct Customer second_user;
		// extra customer addition for joint account
		printf("\nEnter details of the second user of the joint account");	
		struct Customer temp2;
		while(1)
		{
			memset(second_user.username, 0, sizeof(second_user.username));				
			printf("\nEnter the username: ");  						
			scanf("%s", second_user.username);					
			write(nfd, &second_user.username, sizeof(second_user.username));						
			int found=0; // username is unique . that is not found
			read(nfd, &found, sizeof(found));
			if(found==1)
				printf("\nUser Name Already Exists. Please, Enter a Unique Username ");
			if(found==0)
				break;
		}	
		memset(second_user.password, 0, sizeof(second_user.password));				
		printf("\nEnter password : ");					
		scanf("%s", second_user.password);		
		write(nfd, &second_user.password, sizeof(second_user.password)); 
	}									
	print_customers(nfd);				
	print_accounts(nfd);				
} 	
		
void delete(int nfd)				
{				
	int choice, flg=0;									
	printf("\npress 1 to delete using account number\npress 2 to delete using username\n");				
	scanf("%d", &choice);
	write(nfd, &choice, sizeof(choice));
	if(choice==1) // deleting record using account number
	{
		char account_number[1024];
		memset(account_number, 0, sizeof(account_number));
		printf("\nEnter the account number: ");
		scanf("%s", account_number);
		write(nfd, &account_number, sizeof(account_number));
		delete_account(account_number, nfd);
	}		
	else if(choice==2) // deleting record using username 
	{
		printf("\nEnter the username to delete: ");
		char username[1024];
		memset(username, 0, sizeof(username));
		scanf("%s", username);
		write(nfd, &username, sizeof(username));
		delete_username(username, nfd);
	}
	print_customers(nfd);
	print_accounts(nfd);
}	
	
void delete_account(char account_number[], int nfd)					
{													
	int found=0; 			
	struct Account temp;
	read(nfd, &found, sizeof(found));	
	if(found==0)
	{
		printf("\nNo Record exists in the database with the given Account Number ");
		return;
	}
	else
	{
		found=0;
		int choice=1;
		read(nfd, &temp.balance, sizeof(temp.balance));
		read(nfd, &found, sizeof(found));
		if(found==1)
		{
			printf("\ncurrent balance: %0.2lf. press 0 if you want to delete the account", temp.balance);
			scanf("%d", &choice);	
			write(nfd, &choice, sizeof(choice));
			if(choice==0)
				printf("\nAccount Deleted Successfully");
			if(choice==1)
				return;
		}
	}													
	// finding corresponding username and deleting  them from Customers.txt								
	struct Customer temp1;						
	int done;
	read(nfd, &done, sizeof(done));
	while(1)
	{
		if(done!=0)
		{
			read(nfd, &temp1.username, sizeof(temp1.username));
			delete_username(temp1.username, nfd);
			read(nfd, &done, sizeof(done));
		}
		else
			break;
	}													
}  
	
void delete_username(char username[1024], int nfd)
{
	int found=0;
	struct Customer temp;
	read(nfd, &found, sizeof(found));	
	read(nfd, &temp, sizeof(temp));		
	if(found==0){								
		printf("\nNo user exists with given username ");				
		return;
	}
	else if(found==1 && temp.customer_type==1) // record found and normal single user			
		delete_account(temp.account_number, nfd); // deleting corresponding account from account.txt
	else if(found==1 && temp.customer_type==2) // Record found and its a joint account 
	{	
		int found1=0; // joint user not found till now
		read(nfd, &found1, sizeof(found1));					
		if(found1==1)											
			printf("\nNo need for cascading delete of account number from account.txt");						
		else							
			delete_account(temp.account_number, nfd);
	}							
}			

