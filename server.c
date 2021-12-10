// server code

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
void joint_withdraw_money(char account_number[1024], int joint, int nfd);
void joint_print_accounts_acc(char account_number[1024], int type, int nfd); // printing one account
void joint_print_transactions_acc(char account_number[1024], int type, int nfd);


void main()
{
	char username[1024], password[1024];
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	struct sockaddr_in server, client;
	int fd, nfd, client_len;
	char buff[60],c;
	if( (fd=socket(AF_INET, SOCK_STREAM, 0))==-1 )
	{
		perror("\nerror creating socket ");
		exit(1);
	}
	server.sin_family= AF_INET;
	server.sin_addr.s_addr= INADDR_ANY;
	server.sin_port= htons(3856); 
	if( bind(fd, (struct sockaddr *)&server, sizeof(server))==-1 )
	{
		perror("\nerror at binding");
		exit(1);
	}
	if( listen(fd, 2)==-1 )  //queue length
	{ 
		perror("\nerror at listen");
		exit(1);
	}
	write(1, "\nWaiting for the Client to send connect to the server    ", sizeof("\nWaiting for the Client to send connect to the server    "));
	while(1)
	{
		client_len=sizeof(client);
		nfd=accept(fd, (struct sockaddr *)&client, &client_len); 
		write(1, "\nConected to the Client    ", sizeof("\nConected to the Client    "));
		if(!fork()) // child process
		{
			close(fd);  //child does not require
			write(nfd, "\nConnected to the Server    ", sizeof("\nConnected to the Server    "));
			int fd=open("Customer.txt", O_RDWR, 0744);
			int login_check=0; 
			int option_selected;
			struct Customer check;
			read(nfd, username, sizeof(username));
			read(nfd, password, sizeof(password));
			while( read(fd, &check, sizeof(check)) != 0)
			{
				if(strcmp(check.username, username)==0 && strcmp(check.password, password)==0 )
				{				
						login_check=1;
						break;
				}
			} 
			close(fd);
			if(login_check==0)  // if entered username and password does not  match
				write(nfd, &login_check, sizeof(login_check));
			else //if the account exist in the database
			{
				write(nfd, &login_check, sizeof(login_check));
				write(nfd, &check.customer_type, sizeof(check.customer_type));			
				if(check.customer_type == 0) // admin
				{
					int option_selected;
					while(1)
					{
						read(nfd, &option_selected, sizeof(option_selected));
						if(option_selected==1) 
							add_account(nfd);
						else if(option_selected==2) 
							delete(nfd);
						else if(option_selected==3)
							modify(nfd);
						else if(option_selected==4)
							search(nfd);
						else if(option_selected==5)
							print_customers(nfd);
						else if(option_selected==6)
						{
							close(nfd);
							exit(1);
						}
					}
				} //  end of admin option selection 		
			else if(check.customer_type==1) // single account user
			{
				int option_selected;
				while(1)
				{
					read(nfd, &option_selected, sizeof(option_selected));
					write(nfd, &check, sizeof(check));	
					if(option_selected==1)						
						joint_deposit_money(check.account_number, check.customer_type, nfd); 
					else if(option_selected==2)					
						joint_withdraw_money(check.account_number, check.customer_type, nfd);					
					else if(option_selected==3)						
						joint_print_accounts_acc(check.account_number,  check.customer_type, nfd);				
					else if(option_selected==4)						
						password_change(check.username, nfd);					
					else if(option_selected==5)					// print transactions details
						joint_print_transactions_acc(check.account_number,  check.customer_type, nfd);
					else if(option_selected==6)
					{
						close(nfd);
						exit(1);
					}
				}						
			} // end of else if single user option seletion
			else if(check.customer_type==2) // for joint user account
			{
				int option_selected;
				while(1)
				{
					read(nfd, &option_selected, sizeof(option_selected));
					write(nfd, &check, sizeof(check));
					if(option_selected==1)						
						joint_deposit_money(check.account_number, check.customer_type, nfd); 
					else if(option_selected==2)					
						joint_withdraw_money(check.account_number, check.customer_type, nfd);					
					else if(option_selected==3)						
						joint_print_accounts_acc(check.account_number,  check.customer_type, nfd);				
					else if(option_selected==4)						
						password_change(check.username, nfd);					
					else if(option_selected==5)					// print transactions details
						joint_print_transactions_acc(check.account_number,  check.customer_type, nfd);
					else if(option_selected==6)
					{
						close(nfd);
						exit(1);
					}
			   }
			
			} // end of else if joint account option selection
		} // end of else -- account exists in the database
		exit(1);
	} // end of if fork() statement
	else{ //for parent
			close(nfd);//child has these parent does not require it
		}
	} // end of outer while(1) infinite loop	
	close(fd);
} // end of main method


void joint_deposit_money(char account_number[1024], int joint, int nfd)
{
	if(joint==1) // single user account
	{
		double amount;									
		read(nfd, &amount, sizeof(amount));								
		int fd=open("Account.txt", O_RDWR, 0744);
		int found=0;
		struct Account loop;				
		while(read(fd, &loop, sizeof(loop)) != 0)
		{
			if(strcmp(loop.account_number, account_number)==0)
			{
				loop.balance = loop.balance + amount;
				lseek(fd, -sizeof(loop), SEEK_CUR);
				write(fd, &loop, sizeof(loop));
				int tfd=open("Transaction.txt", O_RDWR, 0744);
				// formatting date time to 
				char cur_time[1024];
				memset(cur_time, 0, sizeof(cur_time));				
	  		  	time_t t;					
			  	struct tm* ptm;					
			   	time(&t);		
			  	ptm = localtime(&t);
				strftime(cur_time, 1024, "%d-%b-%Y %H:%M:%S", ptm);
				
				struct Transaction trans1;				
				lseek(tfd, 0, SEEK_END);							
				strcpy(trans1.date, cur_time);		
				strcpy(trans1.account_number, account_number);
				trans1.amount_credit=amount;
				trans1.amount_debit=0;
				trans1.balance_remaining=loop.balance;
				write(tfd, &trans1, sizeof(trans1));
				close(tfd);
				found=1;
				break;
			}	
		} // end of while loop
		write(nfd, &found, sizeof(found));
		if(found==1)
			write(nfd, &loop.balance, sizeof(loop.balance));
		close(fd);
			
	} // single user deposit end
	else // joint account deposit
	{
		double amount;
		read(nfd, &amount, sizeof(amount));	
		int fd=open("Account.txt", O_RDWR, 0744);
		struct Account loop;
		int found=0, unlock=0;
		while(read(fd, &loop, sizeof(loop)) != 0)
		{
			if(strcmp(loop.account_number, account_number)==0)
			{
				found=1; // found 1 means account found
				break;
			}		
		} 
		write(nfd, &found, sizeof(found)); // sending account found status to client side
		if(found==1)
		{
			lseek(fd, -sizeof(loop), SEEK_CUR); 
			struct flock lock;
			lock.l_type = F_WRLCK;
			lock.l_start = 0;  // current plus 0 bytes
			lock.l_whence = SEEK_CUR;
    		lock.l_len = sizeof(loop);
    		lock.l_pid = getpid();
    		fcntl(fd, F_SETLKW, &lock);
			loop.balance=loop.balance + amount;
			write(fd, &loop, sizeof(loop));
			write(nfd, &loop.balance, sizeof(loop.balance)); // sending account balance to client side
			while(1)
			{
				write(nfd ,"\nEnter any number other than 0 to unlock\n", sizeof("\nEnter any number other than 0 to unlock\n")); // sending to print statement to client side
				read(nfd, &unlock, sizeof(unlock));
				if(unlock!=0)
				{
					lock.l_type = F_UNLCK;
    				fcntl(fd,F_SETLK,&lock);
    				break;
    			}
			} //infinite lock loop end
			int tfd=open("Transaction.txt", O_RDWR, 0744);
			// formatting date time to insert into Transaction.txt
			char cur_time[1024];
			memset(cur_time, 0, sizeof(cur_time));
  		  	time_t t;
		  	struct tm* ptm;
		   	t = time(NULL);
		  	ptm = localtime(&t);
			strftime(cur_time, 1024, "%d-%b-%Y %H:%M:%S", ptm);

			struct Transaction trans1;
			lseek(tfd, 0, SEEK_END);							
			strcpy(trans1.date, cur_time);		
			strcpy(trans1.account_number, account_number);
			trans1.amount_credit=amount;
			trans1.amount_debit=0;
			trans1.balance_remaining=loop.balance;
			write(tfd, &trans1, sizeof(trans1));
			close(tfd);
		}
	close(fd);	
	} // else end -- joint account	
} // joint deposit money

void joint_withdraw_money(char account_number[1024], int joint, int nfd)
{
	if(joint==1) // single user withdrawl
	{
		double amount;												
		read(nfd, &amount, sizeof(amount));
		int found=0;
		int fd=open("Account.txt", O_RDWR, 0744);
		struct Account loop;
		while(read(fd, &loop, sizeof(loop)) != 0)
		{
			if(strcmp(loop.account_number, account_number)==0)
			{
				if(loop.balance-amount<0)
				{
					found=2;
					break;
				}
				loop.balance=loop.balance - amount;
				lseek(fd, -sizeof(loop), SEEK_CUR);
				write(fd, &loop, sizeof(loop));
		
				int tfd=open("Transaction.txt", O_RDWR, 0744);
				// formatting date time to insert into Transaction.txt
				char cur_time[1024];
				memset(cur_time, 0, sizeof(cur_time));		
	  		  	time_t t;			
			  	struct tm* ptm;			
				t = time(NULL);			
			  	ptm = localtime(&t);					
				strftime(cur_time, 1024, "%d-%b-%Y %H:%M:%S", ptm);				
											
				struct Transaction trans1;
				lseek(tfd, 0, SEEK_END);							
				strcpy(trans1.date, cur_time);		
				strcpy(trans1.account_number, account_number);
				trans1.amount_credit=0;
				trans1.amount_debit=amount;
				trans1.balance_remaining=loop.balance;
				write(tfd, &trans1, sizeof(trans1));
				close(tfd);
				found=1;
				break;
			}		
		} // end of while loop
		write(nfd, &found, sizeof(found));
		if(found==1)
			write(nfd, &loop.balance, sizeof(loop.balance));
					
		close(fd);			
	} // single user withdraw end
	
	else // joint account withdrawl
	{
		double amount;														
		read(nfd, &amount, sizeof(amount)); // reading amount to withdraw from client							
		int fd=open("Account.txt", O_RDWR, 0744);
		int found=0, unlock=0, correct=0;
		struct Account loop;
		while(read(fd, &loop, sizeof(loop)) != 0)
		{															
			if(strcmp(loop.account_number, account_number)==0)
			{		
				found=1; // account found
				break;
			}		
		}
		write(nfd, &found, sizeof(found));  // sending account found status
		if(found==1) // account is present in the database
		{
			if(loop.balance-amount<0) // insufficient balance
			{
				correct=1; // correct = 1 means insufficient balance
				write(nfd, &correct, sizeof(correct)); // sending correct to server -- newly added
				return;
			}
			lseek(fd, -sizeof(loop), SEEK_CUR); 
			struct flock lock;
			lock.l_type = F_WRLCK;
			lock.l_start = 0;  
			lock.l_whence = SEEK_CUR;
			lock.l_len = sizeof(loop);
			lock.l_pid = getpid();
			fcntl(fd, F_SETLKW, &lock);
			loop.balance=loop.balance-amount;
			write(fd, &loop, sizeof(loop)); //overwriting on the record after performing the debit
			//printf("%0.2lf\n", loop.balance);
			// duplicate statement at 387 after while loop because below line was sending garbage
			write(nfd, &loop.balance, sizeof(loop.balance)); // sending balance to client
			while(1)
			{
				write(nfd ,"\nEnter any number other than 0 to unlock \n", sizeof("\nEnter any number other than 0 to unlock \n"));
				read(nfd, &unlock, sizeof(unlock)); // reading unlock from client
				if(unlock!=0)
				{
					lock.l_type = F_UNLCK;
					fcntl(fd,F_SETLK,&lock);
					break;
				}
			}	// infinite unlock while loop
			write(nfd, &loop.balance, sizeof(loop.balance));  // duplicate balance sending to client
			int tfd=open("Transaction.txt", O_RDWR, 0744);
			char cur_time[1024];
			memset(cur_time, 0, sizeof(cur_time));	
	  		time_t t;
			struct tm* ptm;
			t = time(NULL);
			ptm = localtime(&t);
			strftime(cur_time, 1024, "%d-%b-%Y %H:%M:%S", ptm);
			
			struct Transaction trans1;
			lseek(tfd, 0, SEEK_END);							
			strcpy(trans1.date, cur_time);		
			strcpy(trans1.account_number, account_number);
			trans1.amount_credit=0;
			trans1.amount_debit=amount;
			trans1.balance_remaining=loop.balance;
			write(tfd, &trans1, sizeof(trans1));
			close(tfd);		
		}
	close(fd);
	} // joint account withdraw else end
} // joint withdraw method end

void joint_print_accounts_acc(char account_number[1024], int joint, int nfd)
{
	if(joint==1) // single user
	{
		int fd=open("Account.txt", O_RDWR, 0744);
		struct Account temp;
		int done=0;
		while( read(fd, &temp, sizeof(temp))!=0)
		{
			if(strcmp(temp.account_number, account_number) ==0)
			{
				done=1;
				write(nfd, &done, sizeof(done)); // done !=0 tells we have found the account
				write(nfd, &temp, sizeof(temp));
				break; 
			}	
		}
		close(fd);	
	} // end of single user
	else  // joint print accounts
	{
		int fd=open("Account.txt", O_RDWR, 0744);
		int found=0;
		struct Account temp;
		int done=0;
		while(read(fd, &temp, sizeof(temp)) != 0){				
			if(strcmp(temp.account_number, account_number) ==0){
				
				found=1; // account found in the database
				break;					
			}			
		}
		write(nfd, &found, sizeof(found)); // sending account found status to client
		if(found==1)
		{
			lseek(fd, -sizeof(temp), SEEK_CUR); // moving one record back				
			struct flock lock;			
			lock.l_type = F_RDLCK;				
			lock.l_start = 0;  // current plus 0 bytes			
			lock.l_whence = SEEK_CUR;				
    		lock.l_len = sizeof(temp);			 
    		lock.l_pid = getpid();		
    		fcntl(fd, F_SETLKW, &lock);	
    					
    		write(nfd, &temp, sizeof(temp)); 
    		while(1)
    		{
    			int unlock;
				read(nfd, &unlock, sizeof(unlock));
				if(unlock!=0)
				{
					lock.l_type = F_UNLCK;
    				fcntl(fd,F_SETLK,&lock);
    				break;
    			}
			}	
		}
		close(fd);
	}

} // print accounts using account number

void joint_print_transactions_acc(char account_number[1024], int joint, int nfd)
{
	if(joint==1)  // single user account
	{
		int fd=open("Transaction.txt", O_RDWR, 0744);
		struct Transaction temp;	
		int done=0;
		while( (done=read(fd, &temp, sizeof(temp))) != 0)
		{					
			if( strcmp(temp.account_number, account_number)==0 )
			{
				write(nfd, &done, sizeof(done));
				write(nfd, &temp, sizeof(temp));
				done=0;
			}
		} 
		write(nfd, &done, sizeof(done));
		close(fd);
	} // end of single user 
	else   // joint account print
	{
	 	int fd=open("Transaction.txt", O_RDWR, 0744);
		struct Transaction temp;
		int found=0, unlock=0, done=0;
		while( (done=read(fd, &temp, sizeof(temp))) != 0) //not done yet
		{
			if( strcmp(temp.account_number, account_number)==0 )
			{
				write(nfd, &done, sizeof(done));
				done=0;
				/*lseek(fd, -sizeof(temp), SEEK_CUR);						 
				struct flock lock;
				lock.l_type = F_RDLCK;
				lock.l_start = 0; 
				lock.l_whence = SEEK_CUR;
				lock.l_len = sizeof(temp);
				lock.l_pid = getpid();	
				fcntl(fd, F_SETLKW, &lock);
				read(nfd, &unlock, sizeof(unlock));
				if(unlock==1)
				{
					lock.l_type = F_UNLCK;
					fcntl(fd,F_SETLK,&lock);
					unlock=0;
				}
				*/
				write(nfd, &temp, sizeof(temp));
			}
		} // end of while
		write(nfd, &done, sizeof(done));
		close(fd);
	} // end of else joint account
}

///******************************print customers and print accounts****************************

void print_customers(int nfd)
{		
	int fd=open("Customer.txt", O_RDWR, 0744);				
	int done=0;						
	struct Customer print;
	while( (done=read(fd, &print, sizeof(print))) != 0)
	{
		if(print.customer_type!=5 && print.customer_type!=0)
		{
			write(nfd, &done, sizeof(done));
			write(nfd, &print, sizeof(print));
			done=0;
		}
	 }			
	write(nfd, &done, sizeof(done));			
	close(fd);
}

void print_accounts(int nfd)
{		
	int fd=open("Account.txt", O_RDWR, 0744);
	int done=0;
	struct Account print;
	read(fd, &print, sizeof(print));
	while( (done=read(fd, &print, sizeof(print))) != 0)
	{
		if(strcmp(print.account_number, "0000") !=0 )
		{
			write(nfd, &done, sizeof(done));
			write(nfd, &print, sizeof(print));
			done=0;
		}		
	} 
	write(nfd, &done, sizeof(done));			
	close(fd);	
} 

//***************************************** admin functionalities starts here ***********************

void modify(int nfd)
{
	char username[1024];
	int option_selected=0;			
	read(nfd, &username, sizeof(username));							
	read(nfd, &option_selected, sizeof(option_selected));					
	if(option_selected==1) // single user to joint account
	{					
		int fd=open("Customer.txt", O_RDWR, 0744);						
		struct Customer temp;					
		int found=0; 
		while(read(fd, &temp, sizeof(temp)) != 0)
		{				
			if( strcmp(temp.username, username)==0 && temp.customer_type==2)
			{				
				found=1;
				break;
			}
			else if( strcmp(temp.username, username)==0 && temp.customer_type==1)
			{					
				found=2;
				break;
			}
		} 
		write(nfd, &found, sizeof(found));
		
		if(found==1) // already a joint account
			return;	
		else if(found==0) // record to modify or username doesn't exist
			return;
		
		if(found==2) // record found and is a single user
		{
				temp.customer_type=2; 
				lseek(fd, -sizeof(temp), SEEK_CUR);
				write(fd, &temp, sizeof(temp));
		}
		close(fd);
		
		struct Customer second_user;   // extra customer addition for joint account
		struct Customer temp2;
		while(1)
		{		
			read(nfd, &second_user.username, sizeof(second_user.username));
			int found=0; // username is unique . that is not found
			int fd=open("Customer.txt", O_RDWR, 0744);
			while(read(fd, &temp2, sizeof(temp2)) != 0)
			{
				if(strcmp(temp2.username, second_user.username)==0 && temp2.customer_type!=5)
				{
					found=1;
					break;
				}		
			} 
			write(nfd, &found, sizeof(found)); 
			close(fd);
			if(found==0)
				break;	
		}	// username unique loop
		read(nfd, &second_user.password, sizeof(second_user.password));		
		strcpy(second_user.account_number, temp.account_number);
		second_user.customer_type=2;
		fd=open("Customer.txt", O_RDWR, 0744);
		found=0;
		//struct customer loop;
		while(read(fd, &temp, sizeof(temp)) != 0)
		{
			if(temp.customer_type==5)// deleted account/ duplicate account found and modifying it
			{
				found=1;
				break;
			}			
		} 
		
		if(found==0) // appending at the end
			write(fd, &second_user, sizeof(second_user));
		else // overwriting on deleted record
		{	
			lseek(fd, -sizeof(second_user), SEEK_CUR);
			write(fd, &second_user, sizeof(second_user));
		}		
		close(fd);
	}
	else
		password_change(username, nfd);
	
	print_customers(nfd); 
	print_accounts(nfd);	
}	


void password_change(char username[1024], int nfd)						
{						
	int fd=open("Customer.txt", O_RDWR, 0744);
	int found=0;						
	struct Customer check;							
	while(read(fd, &check, sizeof(check)) != 0)
	{														
		if( strcmp(check.username, username)==0 )
		{	
			found=1;
			break;
		}
	}
	if(found==1)
	{
		char password1[1024], password2[1024];
		int done=0;
		write(nfd, &found, sizeof(found));	
		read(nfd, &password1, sizeof(password1));
		read(nfd, &password2, sizeof(password2));
		if( strcmp(password1, password2) == 0) 
		{
			strcpy(check.password, password1);		
			lseek(fd, -sizeof(check), SEEK_CUR);			
			write(fd, &check, sizeof(check));		
			done=1;				
		}
		write(nfd, &done, sizeof(done));			
	}
	close(fd);
	print_customers(nfd);
} 

void search(int nfd)
{	
	int option_selected;
	read(nfd, &option_selected, sizeof(option_selected));
	if(option_selected==1) // print customer + account details
	{
		char username[1024];						
		int found=0; // no record found				
		char account_number[1024];
		read(nfd, &username ,sizeof(username));						
		int fd=open("Customer.txt", O_RDWR, 0744);						
		struct Customer temp;
		while(read(fd, &temp, sizeof(temp)) != 0)
		{								
			if(strcmp(temp.username, username)==0 && temp.customer_type!=5)
			{							
				found=1; // account exists in the database		
				break;			
			}			
		} 					
		close(fd);
		write(nfd, &found, sizeof(found));
		if(found==1)
		{					
			write(nfd, &temp, sizeof(temp));
			strcpy(account_number, temp.account_number);					
		}
		else
		{
			close(nfd);
			exit(1);
		}							
		struct Account temp1;			
		fd=open("Account.txt", O_RDWR, 0744);			
		found=0;
		int done=0;					
		while(read(fd, &temp1, sizeof(temp1)) != 0){									
			if( strcmp(temp1.account_number, account_number) ==0){					
				//printf("\t%f\n", temp1.bal);
				found=2;
				break;
			}					
		}
		write(nfd, &found, sizeof(found));										
		if(found==2)		
			write(nfd, &temp1, sizeof(temp1));
								
		close(fd);	
	} // end of customer + account details					
	else if(option_selected==2)  // only account details
	{									
		char account_number1[1024];				
		read(nfd, &account_number1 ,sizeof(account_number1));
		struct Account temp2;						
		int fd=open("Account.txt", O_RDWR, 0744);					
		int found=0;
		while(read(fd, &temp2, sizeof(temp2)) != 0)
		{							
			if( strcmp(temp2.account_number, account_number1) ==0)
			{					
				found=3;
				break;
			}							
		}
		write(nfd, &found, sizeof(found)); 				
		if(found==3)
			write(nfd, &temp2, sizeof(temp2));		
		close(fd);				
	}								
} // end of search method

void add_account(int nfd)						
{			
	struct Customer new;						
	struct Customer temp2;
	while(1)
	{													
		read(nfd, &new.username, sizeof(new.username));								
		int found=0;    // assuming username is unique and doesn't exist already in the database
		int fd=open("Customer.txt", O_RDWR, 0744);
		while(read(fd, &temp2, sizeof(temp2)) != 0)
		{	
			if(strcmp(temp2.username, new.username)==0 && temp2.customer_type!=5)
			{								
				found=1;
				break;
			}		
		} 
		write(nfd, &found, sizeof(found));
		close(fd);
		if(found==0)
			break;
	} // end of username unique infinite while loop
	read(nfd, &new.password, sizeof(new.password));			
	char account_number[1024];
	memset(account_number, 0, sizeof(account_number));
	memset(new.account_number, 0, sizeof(new.account_number));								
	strcat(account_number, "SBH-TS-WNP-00015-"); // state-district-branchcode-starting_Sequence-username
	strcat(account_number, new.username);
	strcpy(new.account_number, account_number);
	read(nfd, &new.customer_type, sizeof(new.customer_type));				
	int fd=open("Customer.txt", O_RDWR, 0744);	
	// find the first record that is duplicate or deleted. customer_type= 5				
	int found=0; // no duplicate or deleted record as of now						
	struct Customer temp;						
	while(read(fd, &temp, sizeof(temp)) != 0)
	{						
		if(temp.customer_type==5)
		{														
			found=1;	
			break;			
		}							
	} 				
	if(found==0) // if no deleted record found then append at the end  				
		write(fd, &new, sizeof(new));					
	else // overwriting on the deleted record
	{						
		lseek(fd, -sizeof(temp), SEEK_CUR);					
		write(fd, &new, sizeof(new));						
	}		
	close(fd);
	
	struct Account new_acc; //account creation and pushing record into Account table
	struct Account temp1;
	memset(new_acc.account_number, 0, sizeof(new_acc.account_number));
	strcpy(new_acc.account_number, new.account_number);
	new_acc.balance=0.0;
	fd=open("Account.txt", O_RDWR, 0744);
	found=0; 				
	while(read(fd, &temp1, sizeof(temp1)) != 0)
	{				
		if(strcmp(temp1.account_number,"0000")==0 )// 4 zeroes in account_number signifies that deleted account	
		{			
			found=1;				
			break;			
		}									
	} 										
	if(found==0) // if no deleted record is found				
		write(fd, &new_acc, sizeof(new_acc));
	else  // overwriting on the deleted record 
	{
		lseek(fd, -sizeof(temp1), SEEK_CUR);
		write(fd, &new_acc, sizeof(new_acc));
	}		
	close(fd);
	
	if(new.customer_type==2) // enter joint account details
	{
		struct Customer second_user;  // extra customer addition for joint account
		struct Customer temp2;
		while(1)
		{															
			read(nfd, &second_user.username, sizeof(second_user.username));
			int found=0;  // assuming username is unique
			int fd=open("Customer.txt", O_RDWR, 0744);
			while(read(fd, &temp2, sizeof(temp2)) != 0)
			{
				if(strcmp(temp2.username, second_user.username)==0 && temp2.customer_type!=5){
					found=1;
					break;
				}		
			} 
			write(nfd, &found, sizeof(found));
			close(fd);
			if(found==0)
				break;
		}		
		read(nfd, &second_user.password, sizeof(second_user.password));
		strcpy(second_user.account_number, new.account_number);
		second_user.customer_type=2;
		fd=open("Customer.txt", O_RDWR, 0744);
		found=0;
		while(read(fd, &temp, sizeof(temp)) != 0)
		{
			if(temp.customer_type==5)
			{
				found=1;
				break;
			}			
		} 
		if(found==0) 
			write(fd, &second_user, sizeof(second_user));
		else 
		{	
			lseek(fd, -sizeof(second_user), SEEK_CUR);
			write(fd, &second_user, sizeof(second_user));
		}		
		close(fd);
		
		// as this is the second user.... we map this customer to same account created for first customer 
	}					
	print_customers(nfd);				
	print_accounts(nfd);				
} 	// end of add account
		
void delete(int nfd)				
{				
	int option_selected;		
	read(nfd, &option_selected, sizeof(option_selected));
	if(option_selected==1) // deleting record using account number
	{
		char account_number[1024];
		read(nfd, &account_number, sizeof(account_number));
		delete_account(account_number, nfd);
	}		
	else if(option_selected==2) // deleting record using username 
	{
		char username[1024];
		read(nfd, &username, sizeof(username));
		
		delete_username(username, nfd);
	}
	print_customers(nfd);
	print_accounts(nfd);
} // delete method end

void delete_account(char account_number[], int nfd)					
{								
	int fd=open("Account.txt", O_RDWR, 0744);					
	int found=0; 									
	struct Account temp;
	while(read(fd, &temp, sizeof(temp)) != 0)
	{
		if(strcmp(temp.account_number, account_number)==0 )
		{
			found=1; // record to be deleted is found
			break;
		}		
	} 
	write(nfd, &found, sizeof(found));
	if(found==0) // if the account doesn't exist
		return;
	else
	{
		write(nfd, &temp.balance, sizeof(temp.balance));
		int option_selected=1; 
		found=0;
		if(temp.balance>0.0) // checking whether the account has some balance before deleting
		{	
			found=1;
			write(nfd, &found, sizeof(found));
			read(nfd, &option_selected, sizeof(option_selected));
			
		}
		else
		{
			write(nfd, &found, sizeof(found));
			option_selected=0;
		}
		
		if(option_selected==0)
		{
			strcpy(temp.account_number, "0000");    // 4 zeroes in account number signifies deleted account		
			temp.balance=0.0;					
			lseek(fd, -sizeof(temp), SEEK_CUR);				
			write(fd, &temp, sizeof(temp));					
		}
		else
		{
			close(fd);
			return;
		}			
	}						
	close(fd);			
							
	// finding corresponding username and deleting  them from Customers.txt					
	fd=open("Customer.txt", O_RDWR, 0744);						
	struct Customer temp1;						
	int done=0;					
	while( (done=read(fd, &temp1, sizeof(temp1))) != 0)
	{							
		if( strcmp(account_number, temp1.account_number)==0 && (temp1.customer_type==1 || temp1.customer_type==2) )
		{
			write(nfd, &done, sizeof(done));
			write(nfd, &temp1.username, sizeof(temp1.username));
			delete_username(temp1.username, nfd);					
		}								
	}
	write(nfd, &done, sizeof(done));							
	close(fd);							
}  // end of delete account method

void delete_username(char username[1024], int nfd)
{
	int fd=open("Customer.txt", O_RDWR, 0744);
	int found=0;
	struct Customer temp;
	while(read(fd, &temp, sizeof(temp)) != 0)
	{
		if( strcmp(temp.username, username)==0 && (temp.customer_type==1 || temp.customer_type==2) )
		{				
			found=1;	// customer record to be deleted is found			
			break;			
		}						
	} 					
	write(nfd, &found, sizeof(found));
	write(nfd, &temp, sizeof(temp));
	if(found==0)
	{
		close(fd);
		return;		
	}
	else if(found==1 && temp.customer_type==1) // record found and is normal single user
	{
		temp.customer_type=5; // 5 in customer_type denotes that account is  deleted 
		lseek(fd, -sizeof(temp), SEEK_CUR);			
		write(fd, &temp, sizeof(temp));			
		delete_account(temp.account_number, nfd); // deleting corresponding account from Account table
		close(fd);
	} 
	else if(found==1 && temp.customer_type==2) // Record found and it's a joint account user
	{ 	
		char joint_account_number[1024];
		strcpy(joint_account_number, temp.account_number); 
		temp.customer_type=5;
		lseek(fd, -sizeof(temp), SEEK_CUR);
		write(fd, &temp, sizeof(temp));
		close(fd);
		//finding if the joint user still in the database or deleted prior to this current user deletion
		int fd1=open("Customer.txt", O_RDWR, 0744);
		int found1=0; // joint user not found till now
		struct Customer temp1;			
		while(read(fd1, &temp1, sizeof(temp1)) != 0)
		{						
			if(strcmp(temp1.account_number, joint_account_number)==0  && temp1.customer_type==2 )
			{			
				found1=1; // jointer account second user is exists in the database. no need to delete account
				break;					
			}									
		}									
		write(nfd, &found1, sizeof(found1));	
		if(found1==1)											
			return;						
		else							
			delete_account(joint_account_number, nfd);						
										
	}  // end of else if joint account delete handling							
} // end of delete username method
	
