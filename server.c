#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/sem.h>
#include "definitions.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>

#define BILLION 1E9

struct sockaddr myname;
struct timespec requestStart1, requestEnd1 ,requestStart2, requestEnd2;

char buf[80];
key_t key=1234; 
int shmid;
sem_t *mutex,*thl,*bank, *full, *credit_card;
answer ans,ans_test,e;
//function that defines the message to be sent to client
void send_message(Theatro *s, answer ans, int new_sd, int flag, int flag_krathsh, int ntickets)
{
int apostoli=0;
if (flag==0)
{
strcpy(e.buf1,"The theatre is full!\n");
e.valid=0;
apostoli=1;
}
else{
	if (flag_krathsh==0)
	{
	strcpy(e.buf1, "Your credit card is not valid\n");
	e.valid=0;
	apostoli=1;
	}
	else if(flag_krathsh==1)
	{
	
		switch(ans.buf1[0]){
			case 'a':
			if(s->count_ia==0) 
				{
				strcpy(e.buf1, "There are no available seats in zone A \n");
				e.valid=0;
				apostoli=1;
					 }
		       break;

			case 'b':
			if(s->count_ib==0) {
				strcpy(e.buf1, "There are no available seats in zone B\n");
				e.valid=0;
				apostoli=1;
				}
			break;

			case 'c':
			if(s->count_ic==0) {
				strcpy(e.buf1, "There are no available seats in zone C\n");
				e.valid=0;
				apostoli=1;
				}
			break;

			case 'd':
			if(s->count_id==0) {
				strcpy(e.buf1, "There are no available seats in zone D \n");
				e.valid=0;
				apostoli=1;
				}
			break;
		}//switch
	if(apostoli==0)
	{
	strcpy(e.buf1,"Your reservation was succesful\n");
	e.valid=1;
	e.pelatis_id=new_sd;
		switch(ans.buf1[0])
		{
		case 'a':
		   e.kostos=50*ntickets;
		   break;
		case 'b':
		   e.kostos=40*ntickets;
		   break;
		case 'c':
		   e.kostos=35*ntickets;
		   break;
		case 'd':
		   e.kostos=30*ntickets;
		   break;

		}
	}//end if send
    }//elseif
}//else flag
send(conn, &e, sizeof(e),0); //stelnoume thn apanthsh ston client
}//void sendmessage


//Avoid zombie child proccesses
void sig_child (int signo)
{
 signal(SIGCHLD, sig_child);
 pid_t pid;
 int stat;
 while((pid=waitpid(-1,&stat, WNOHANG))>0)
   {
	printf("Child %d terminated. \n",pid);
   }
}
//synarthsh pou kaleitai apo thn diergasia pou koitaei ta eishthria gia na dei an h diergasia ths trapezas exei dwsei to ok
//gia thn egkyrothta ths pistwtikhs wste na proxwrhsoume se krathsh
//h plhrhs ekshghsh ths egkyrothtas ths pistwtikh vrisketai sthn anafora!!!

/*
function that checks the credibility of the "credit card" of client that connected to
the server and requests for a seat in the theatr
*/
int check_credibility(pid_t pid, Theatro *s)
{
int i,valid;
valid=-1; //se periptwsh la8ous epistrefetai -1
sem_wait(credit_card);
	for(i=0;i<60;i++)
	{

		if(s->creditcard[i][0]==(int)pid) //koitaei na vrei to keli pou exei to pid ths
		{
		   if(s->creditcard[i][1]==1){ valid=1;} //analoga me thn timh ths 2hs sthlhs einai egkyrh h mh h pistwtikh
		   else if(s->creditcard[i][1]==0){valid=0;}
		   s->creditcard[i][0]=0;//h timh tou keliou ginetai ksana mhden wste na mporei na xrhsimopoih8ei apo allh diergasia
			}	
	}
sem_post(credit_card);
return valid;
}
/*
an array in shared memory is used so as the processes can tell each other whether
the credit card is valid or not
*/
int write_credibility(int flag_credit, Theatro *st,pid_t ppid)
{
int i;
	for(i=0;i<60;i++)
	{
	 if(st->creditcard[i][0]==0)
		{
		 st->creditcard[i][0]=(int)ppid;
		 st->creditcard[i][1]=flag_credit;
		 return 666; //TA CHIP TWN PISTWTIKWN MAS ELEGXOUN
			}

	}


}
//function that generates random values so we decide whether credit card is valid or not
void info_trapeza(unsigned int conn, int childpid)
{
    srand(time(NULL));
	int temp,shmtrapeza,flag_credit,check;
	pid_t ppid;
	Theatro *st;
	shmtrapeza=shmget(key, sizeof( Theatro),IPC_CREAT|0666);
	st=(Theatro *)shmat(shmtrapeza, NULL, 0);
	temp=rand_gen(1,10);
	ppid=getppid(); 
	if (temp==5) //au8aireta epilegw mia apo tis 10 pi8anes times gia thn aporripsh ths kartas
		{
		printf("Credit card was denied...\n");
		st->failed_orders +=1; //auksanoume ton counter twn apotyxhmenwn paraggeliwn
		flag_credit=0;
		}
	else 
		{
		printf("Your credit card has been accepted \n");
		flag_credit=1;
			}

	st->all_orders +=1;//auksanoume ton counter twn sunolikwn paraggeliwn	
	sem_wait(credit_card);
	check=write_credibility(flag_credit, st, ppid); //grafoume sthn koinh mnhmh thn egkyrothta h mh ths pistwtikhs wste na thn diavasei h diergasia pateras
	sem_post(credit_card);
	if(check!=666){printf("ERROR!\nERROR\nERROR\nERROR\n");}
	
}

//when CTR+C is pressed semaphores are unlinked as well
void catch_int( int signum)
{
signal(SIGINT, catch_int);
shmctl(shmid, IPC_RMID, NULL);
sem_unlink(sname1);
sem_unlink(sname2);
sem_unlink(sname3);
sem_unlink(sname4);
printf("\nSemaphores unlinked successfully\n");

exit(1);

}

//function that generates random numbers in [min,max]
int rand_gen(int min, int max)
	{
	int r;
	r = (rand() % (max+1-min))+min;
	return r;
	}

//auti i sinartisi ylopoiei to kleisimo twn eisitiriwn
// oi pinakes i_za[],i_zb[],i_zc[],i_zd[]apo8hkevoun proswrina ton ari8mo twn 8esewn stis opoies 8a ginei h krathsh
//Epomenws auti i sinartisi tha trexei afou exoume dextei to signal apo tin sinartisi info_trapeza oti egine epitixws 
//i xrewsi tis kartas. O elenxos tis kartas opws kai o elegxos tis diathesimotitas thesewn  ginontai parallhla. 

/*
function that make the reservation. It assigns to the theatre array the pid of the proccess that
served the client
*/
int switch_kratisi(answer ans, int new_sd,int ntickets, Theatro *s)
{
   int t=0;
   int flag_success = 0;//otan ginete 1 dilwnei to epituxes kleisimo eisitiriwn
   switch(ans.buf1[0])//buf[0] einai o xarakthras pou deixnei th zwnh: a,b,c,d
   {
	case'a':
	{
	 if(s->count_ia >= ntickets) //ean ta zhtoumena eishthria einai ligotera/ isa apo ta enapomeinanta
	 {
	  for(t=0;t<ntickets;t++)
	  {
            s->zoneA[s->i_za[t]] = new_sd; //apo8hkeuoume ton ari8mo ths syndeshs sto plano tou 8eatrou
	    s->count_ia--; //meiwnoume ton ari8mo twn enapomeinantwn 8esewn
	   }
	   flag_success = 1; //epituxes kleisimo eisitiriwn
	  }//end if
         }//end case
	break;
	case'b':
	{
	 if(s->count_ib >= ntickets)
	 {
	  for(t=0;t<ntickets;t++)
	  {
            s->zoneB[s->i_zb[t]] = new_sd;
	    s->count_ib--;
	   }
           flag_success = 1;//epituxes kleisimo eisitiriwn
	  }//end if
         }//end case
	break;
	case'c':
	{
	 if(s->count_ic >= ntickets)
	 {
	  for(t=0;t<ntickets;t++)
	  {
            s->zoneC[s->i_zc[t]] = new_sd;
	    s->count_ic--;
	   }
	   flag_success = 1;//epituxes kleisimo eisitiriwn
	  }//end if
         }//end case
	break;
	case'd':
	{
	 if(s->count_id >= ntickets)
	 {
	  for(t=0;t<ntickets;t++)
	  {
            s->zoneD[s->i_zd[t]] = new_sd;
	    s->count_id--;
	   }
	   flag_success = 1;//epituxes kleisimo eisitiriwn
	  }//end if
         }// end case
	break;
    }//end switch
  return flag_success;
}//telos switch_kratisi

int xrewsi(int kostos, Theatro *s)
{
 s->ypol_etairias += kostos; 
 return kostos;
}
//synarthsh pou typwnei otan gemisei to 8eatro to plano tou 8eatrou
void plano_theatrou(Theatro *s)
{//teliko plano theatrou 
	int temp = 0;
	int k = 0;
	int i;

	//zone A

	temp = s->zoneA[0];
	printf("Zone A: [ \t");
	for (i=0; i< 100; i++)
	{
	  if(temp != s->zoneA[i])
	      {
		k += 1;
		temp = s->zoneA[i];
		}
	
	  printf(" ,Cl%d\t",k);
	  
	}
	printf(" ]\n ");


	//zone B

	temp = s->zoneB[0];
	printf("Zone B: [ \t");
	for (i=0; i< 130; i++)
	{  
	  if(temp != s->zoneA[i])
	      {
		k += 1;
		temp = s->zoneA[i];
		}
	
	  printf(" ,Cl%d\t",k);
	}
	printf(" ]\n ");


	//zone C

	temp = s->zoneC[0];
	printf("Zone C: [ \t");
	for (i=0; i< 180; i++)
	{
	    if(temp != s->zoneA[i])
	      {
		k += 1;
		temp = s->zoneA[i];
		}

		printf(" ,Cl%d\t",k);
	}
	printf(" ] \n");


	//zone D

	temp = s->zoneD[0];
	printf("Zone D: [ \t");
	for (i=0; i< 230; i++)
	{
	    if(temp != s->zoneA[i])
	      {
		k += 1;
		temp = s->zoneA[i];
		}
	
	    printf(" ,Cl%d\t",k);
	}
	printf(" ]\n ");
}

//*******//
main()
{

Theatro *s;

pid_t eksyphrethsh, thlefwnhths,pid;

int sock, new_sd, adrlen, cnt,i,shm_temp,flag,shm_start,ntickets,k,j,stato,flag_krathsh;
int kostos = 0; //to sunoliko kostos twn eisitiriwn
float orders_rate = 0; //%apotuximenwn
int profit = 0;//kerdos theatrou
float  overall_time1 ,overall_time2 ,average_time1 ,average_time2; //variables for measuring average serving time
overall_time1 = 0;//arxikopoihsh
overall_time2 = 0;
average_time1 = 0;
average_time2 = 0;

flag=1;
Theatro *shm;

//memory segment is created
shmid = shmget(key, sizeof( Theatro),IPC_CREAT|0666);
shm=(Theatro *)shmat(shmid, NULL, 0);
shm->count_ia=100; //arxikopoioume oles tis times
shm->count_ib=130;
shm->count_ic=180;
shm->count_id=230;
shm->ypol_etairias = 0;
shm->ypol_theatrou = 0;
shm->all_orders = 0;
shm->failed_orders = 0;

	for(i=0;i<100;i++) //initialization of reservation arrays
	{
	shm->zoneA[i]=0;
	}
	for(i=0;i<130;i++)
	{
	shm->zoneB[i]=0;
	}
	for(i=0;i<180;i++)
	{
	shm->zoneC[i]=0;
	}
	for(i=0;i<230;i++)
	{
	shm->zoneD[i]=0;
	}
for(i=0;i<60;i++)
	for(j=0;j<2;j++)
		{
		shm->creditcard[i][j]=0;
	
		}


//semaphores are created
mutex=sem_open(sname3, O_CREAT, 0644, 1); //mutex for shared memory

thl=sem_open(sname2, O_CREAT ,0644 , 10); //Available telephones are 10

bank=sem_open(sname1, O_CREAT ,0644, 4); //Available bank terminals are 4

credit_card=sem_open(sname4, O_CREAT, 0644 , 1);// mutex for credit card credibility array



sock = socket(AF_UNIX, SOCK_STREAM, 0); //d

	if (sock < 0) 

	{

	printf("server socket failure %d\n", errno);

	perror("server: ");

	exit(1);

	}

 

myname.sa_family = AF_UNIX;

strcpy(myname.sa_data, "/tmp/billb");

adrlen = strlen(myname.sa_data) +sizeof(myname.sa_family);

 

unlink("/tmp/billb"); /* defensive programming */

if (bind(sock, &myname, adrlen) < 0) 

	{

	printf("server bind failure %d\n", errno);

	perror("server: ");

	exit(1);

}

 

if (listen(sock, 5) < 0)

	{

	printf("server listen failure %d\n", errno);

	perror("server: ");

	exit(1);

}	

 

/* Ignore child process termination. */

 

signal (SIGCHLD, SIG_IGN);

signal(SIGINT, catch_int);


 

while (flag !=0) 

{
printf("Waiting for connection...\n");
new_sd = accept(sock, &myname, &adrlen); //accept request from queue
	if (new_sd < 0) 

	{
		printf("server accept failure %d\n", errno);
		exit(1);}



 

	printf("Connection established...\n");
	printf("Currently proccessing...\n");
	cnt = recv(new_sd, &ans, sizeof(ans),0);//accepting data from client, Theatre Zone and no of tickets
	clock_gettime(CLOCK_REALTIME, &requestStart1);
	sem_wait(thl);//waiting for telephone semaphore

	clock_gettime(CLOCK_REALTIME, &requestEnd1);
	overall_time1 += ( requestEnd1.tv_sec - requestStart1.tv_sec ) + ( requestEnd1.tv_nsec - requestStart1.tv_nsec )/ BILLION;
	clock_gettime(CLOCK_REALTIME, &requestStart2);

	if ((thlefwnhths=fork()) == 0)  //fork so that we do required tasks

	{ /* child process */
		
		break; //kanoume break apo to loop tou server 

	} /* closing bracket for if (fork() ... ) */

 

} /* closing bracket for while (1) ... ) */
	
	/* . . . . . */
	
	printf ("Client wants %c tickets in zone %c \n",ans.buf1[1] , ans.buf1[0]);
	printf("Proccessing request...\n");
	
	//fork to "simultaneously" chec credit card and seat availability
	if( (eksyphrethsh=fork())==0){
	    sem_wait(bank);
		printf("Checking credit card...\n");
		sleep(2);
		info_trapeza(new_sd,eksyphrethsh);//credibility check
		sem_post(bank);
		exit(0);

            }//if( (eksyphrethsh=fork())==0)
	 else{ //parent process
	//diergasia pou elegxei tis 8eseis tou 8eatrou	 
	 sleep(6);
	//syndeoume thn koinh mnhmh gia na ginei elegxos an yparxoun dia8esimes 8eseis
	 s=shm;
	 i=0;
	 k=0;
	 ntickets=ans.buf1[1]-'0'; //metatrepoume ton ari8mo twn eishthriwn pou epelekse o client apo char->int
	 //flag becomes zero if all seats are taken.
         if(s->count_ia <= 0 && s->count_ib <= 0 && s->count_ic <= 0 && s->count_id <= 0)
	  { 
	    flag = 0;
          }
	 sem_wait(mutex);
	 if(flag)
		{
		
		printf("Searching for empty seats...\n");
	        switch(ans.buf1[0]) //buf[0] einai o xarakthras pou deixnei th zwnh: a,b,c,d
			{
			case 'a':
			 if(s->count_ia >= ntickets) //ean ta zhtoumena eishthria einai ligotera/isa apo ta enapomeinanta tote proxwrame sto psaksimo
			{
				 for(i=0;i<100;i++) //diapername olh thn zwnh mexri na vre8ei kenh 8esh (exoun arxikopoih8ei se mhden)
					{
					if(s->zoneA[i]!=0)continue;//oso vriskei 8eseis diafores tou mhden shmainei oti einai kleismenes                              
					else
					{
					 s->i_za[k] = i;//vrikame thesi kai tin apothikeuoume ston antistoixo pinaka proswrinhs apo8hkeushs tis kathe zonis
				   
			                 k+=1;
					 if(k== ntickets) //otan exoume kleisei ola ta eishthria ypologizoume to kostos kai ginetai break apo to for loop
					 {
					    
					    kostos = 50 * ntickets;
					    break;
					 }
						}//end if else

					} //end for
			
			}	//end if(count_ia >= ntickets)
			else { printf("There are no available seats in zone A %d\n", s->count_ia);
			       s->failed_orders +=1;
		            
			 
				}//end else
			
			break;
			case 'b':
			  if(s->count_ib >= ntickets)
			{
				 for(i=0;i<130;i++) //diapername olh thn zwnh mexri na vre8ei kenh 8esh (exoun arxikopoih8ei se mhden)
					{
					if(s->zoneB[i]!=0)continue;
					else
					{
					 s->i_zb[k] = i;//vrikame thesi kai tin apothikeuoume ston antistoixo pinaka tis kathe zonhs
					 k+=1;
					 if(k== ntickets)
					 {
					    
                        kostos = 40 * ntickets;
					    break;
					 }	
						}//end if else

					} //end for
			
			}	//end while
                       else { 
					         printf("There are no available seats in zone B %d\n", s->count_ib);
			                 s->failed_orders +=1;
		              
							}//end else
			
			break;
			case 'c':
			if(s->count_ic >= ntickets)
			{
				 for(i=0;i<180;i++) //diapername olh thn zwnh mexri na vre8ei kenh 8esh (exoun arxikopoih8ei se mhden)
					{
					if(s->zoneC[i]!=0)continue;
					else
					{
					 s->i_zc[k] = i;//vrikame thesi kai tin apothikeuoume ston antistoixo pinaka tis kathe zonis
					 k+=1;
					 if(k== ntickets)
					 {
					    kostos = 35 * ntickets;
					    break;
					 }
						}//end if else

					} //end for

			}	//end while
                    else { 
						 printf("There are no available seats in zone C %d\n", s->count_ic);
						 s->failed_orders +=1; }
		               
			
			break;
			case 'd':
			if(s->count_id >= ntickets)
			{
				 for(i=0;i<230;i++) //diapername olh thn zwnh mexri na vre8ei kenh 8esh (exoun arxikopoih8ei se mhden)
					{
					if(s->zoneD[i]!=0)continue;
					else
					{
					 s->i_zd[k] = i;//vrikame thesi kai tin apothikeuoume ston antistoixo pinaka tis kathe zonis
					 k+=1;
					 if(k== ntickets)
					 {
					    kostos = 30 * ntickets;
					    break;
					 }
						}//end if else

				         } //end for

			    }	// end if(count_id >= ntickets)
                          else {
								printf("There are no available seats in zone D %d\n", s->count_id);
								s->failed_orders +=1;
		               
                               }//end else   
			            

      		         break;
		 		 
	               }//end switch        
		
		}//if(flag)
		else if (flag == 0){ //sthn periptwsh pou to flag=0 dhladh to 8eatro exei gemise
		    s->failed_orders += 1;
		    printf("The theatre is completelly full\n");
		    orders_rate = (s->failed_orders)*100/(s->all_orders);
		    printf("Number of orders = %d\n",s->all_orders);
		    printf("Percentage of failed orders = %f %% \n",orders_rate);
		    printf("Overall profit = %d\n",s->ypol_etairias);
		    average_time1 =(float) overall_time1 / s->all_orders;
		    
		    printf("Average waiting time of client: %f\n",average_time1);
		    
            plano_theatrou(s); //ektypwsh planou 8eatrou
		    
		   		
			}
	sem_post(mutex);
	eksyphrethsh = wait(&stato); //perimenoume h diergasia ths trapezas na pe8anei gia na doume sthn koinh mnhmh an h karta einai egkyrh
	pid=getpid();
	
	if(flag)//ean to 8eatro den einai gemato
	{	flag_krathsh=check_credibility(pid, s);//kaloume thn synarthsh pou elegxei thn egkyrothta ths kartas
		if(flag_krathsh==0) //mh egkyrh pistwtikh
			{
			 printf("Credit card not valid, cant make reservation \n");
			}//end if flag krathsh=0
		else if(flag_krathsh==1) //egkyrh pistwtikh
		{
			if(switch_kratisi(ans, new_sd, ntickets, s) == 1) //kaloume thn synarthsh pou 8a kleisei ta eishthria
			 {
				kostos = xrewsi(kostos ,s);
				printf("Successful reservation with overall cost : %d E\n",kostos);
				}
			else {printf("Reservation unsuccessful\n");}
		}//end else if flag_krathsh
		else if(flag_krathsh==-1)
		{
			printf("Error making reservation!\n");

			}
	}//end 2nd if(flag)
	send_message(s, ans , conn, flag, flag_krathsh, ntickets); // synarthsh pou stelnei thn katallhlh apanthsh ston client
	}//end if fork
	 sem_post(thl); //h douleia tou thlefwnhth exei teleiwsei opote kanei post ton semaforo
	clock_gettime(CLOCK_REALTIME, &requestEnd2);
	overall_time2 += ( requestEnd2.tv_sec - requestStart2.tv_sec ) + ( requestEnd2.tv_nsec - requestStart2.tv_nsec )/ BILLION;
        average_time2 =(float) overall_time2 / s->all_orders;
	if( flag == 0 )
	{
        	printf("Average time serving client: %f\n",average_time2);
		kill(getppid(), SIGINT);
		kill(getpid(),SIGKILL);
	}
	/* . . . . . */

	close (new_sd); /* close prior to exiting */

	exit(0);


	
 

} /* closing bracket for main procedure */


 

