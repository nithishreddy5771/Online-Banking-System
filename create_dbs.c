
#include<stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include "structures.c"


void main(){
	
	int fd=open("Customer.txt", O_CREAT|O_RDWR, 0744);
	
	struct Customer admin; // creating customer with name admin
	strcpy(admin.username,"admin1");
	strcpy(admin.password, "admin@123");
	strcpy(admin.account_number, "SBH-00001-000001");  // SBH- BRANCH CODE LEN-5   AND   ACTUAL ACC NUM LEN -6
	admin.customer_type=0;
	write(fd, &admin, sizeof(admin));
	close(fd);
	
	fd=open("Account.txt", O_CREAT|O_RDWR, 0744);
	struct Account ad;
	strcpy(ad.account_number, "SBH-00001-000001");
	ad.balance=0.0;
	write(fd, &ad, sizeof(ad));
	close(fd);
	
	fd=open("Transaction.txt", O_CREAT|O_RDWR, 0744);
	close(fd);

}
