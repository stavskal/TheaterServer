#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define timh_A 50
#define timh_B 40
#define timh_C 35
#define timh_D 30

//struct gia ta dedomena pou 8a stelnei o client sto server
typedef struct{
	char buf1[80];
	int pelatis_id;
	int kostos;
	int valid;
	}answer;


char buf[80];
answer test;
struct sockaddr myname;
key_t key=1234; //to key ths shmget()
int shmflg,a,b; 
int shmid; //to epistrefei h shmget()



//synarthsh gia na paragei tyxaious ari8mous  se fixed range
int rand_gen(int min, int max)
	{
	int r;
	r = (rand() % (max+1-min))+min;
	return r;
	}

void catch_alarm( int sig_num)
	{
	 signal(SIGALRM, catch_alarm);
	 printf("We are sorry for your waiting, one of our telephonists will be soon available\n");
	 exit(0);
	}

main(int argc ,char *argv[])

{
	srand(time(NULL));
	int sock, adrlen, cnt,i,rand_temp,num_tickets;
	char zone;
	
//dhmiourgountai oi metavlhtes ths krathshs pou 8a staloun ston server
	if( argc==1)
	{
	rand_temp= rand_gen(1,10); //provlepetai to zone pou 8a kanei thn krathsh0
		switch(rand_temp){
			case 1:
			 zone='a'; //to epilegei me pososto 10%
	 		 printf("Zone A\n");
			 break;
			case 2:
			case 3:
			 zone='b'; // 20 %
			 printf("Zone B\n");
			 break;
			case 4:
			case 5:
			case 6:
			  zone='c'; //30 %
			  printf("Zone C\n");
			  break;
			default:
			  zone='d'; //40%
			  printf("Zone D\n");
			}

		num_tickets=rand_gen(1,4); //epilegei tyxaia 1-4 eishthria
		//to struct ths krathshs einai etoimo gia apostolh
		printf("You chose : %d tickets\n",num_tickets);
	test.buf1[0]=zone; //sthn prwth 8esh mpainei h zwnh pou epelekse o client
	test.buf1[1]=(char)((int)'0'+ num_tickets); // sth deuterh o ari8mos twn eishthriwn afou ton metatrepsoume se char
	}
	else if(argc==3)
	{
	test.buf1[0]=(char)argv[1][0];
	test.buf1[1]=(char)argv[2][0];
	printf("%c\t%c \n" , test.buf1[0], test.buf1[1]);
	}
	else
	{printf("Wrong input!!\n");
	 printf("Terminating...\n");
		exit(0);
		}
	sock = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sock < 0)

		{

		printf("client socket failure %d\n", errno);

		perror("client: ");

		exit(1);

		}

	 

	myname.sa_family = AF_UNIX;

	strcpy(myname.sa_data, "/tmp/billb");

	adrlen = strlen(myname.sa_data) +

	sizeof(myname.sa_family);

	 

	if (a=connect( sock, &myname, adrlen) < 0)

		{

		printf("client connect failure %d\n", errno);

		perror("client: ");

		exit(1);

		}

	/* . . . . . */


	
	
	
       

	signal(SIGALRM, catch_alarm);
	alarm(10);
	// stelnei ta dedomena sto server
 	b=send(sock,&test,sizeof(test),0);
	
	if(b<0) puts("Send failed\n");
	alarm(0);
	if(recv(sock,&test,sizeof(test),0)>0)
		{	
		 if(test.valid==0) printf("%s \n", test.buf1);
	 	 else
		{printf("%s\n",test.buf1);
		 printf("Your reservation is under the id: %d \n", test.pelatis_id);
			}
		}

	 

	/* . . . . . */

	exit(0);

}
