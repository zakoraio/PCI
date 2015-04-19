#include <sys/types.h>
#include <sys/wait.h>
#include<stdio.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>

#define NUMCHLDS 16

int pids[NUMCHLDS];
char *dirs[8] = {"/usr","/bin","/dev","/etc","/home","/bin","/opt","/root"};
char *argv[7]={"/root/OS/randread","/root","1234","0.4","0.8","1",NULL};

double randProb()
{
	return (double)rand() / (double)((unsigned)RAND_MAX + 1);
}

void spawnReader(int pid,int place){
	int idx,randomN;		
	char buf[6],buf1[7],buf2[7];	
	srand(rand()%200000);
	randomN = rand()%2000;
	sprintf(buf,"%f",randomN);sprintf(buf1,"%f",randProb());sprintf(buf2,"%f",randProb());
	argv[1]=dirs[rand()%8];argv[2]=buf;argv[3]=buf1;argv[4]=buf2;argv[6]=NULL;

	if(pid !=0)
		idx = findPidIndex(pid); 
	else
		idx = place;
	
	pids[idx] = fork();

	if(pids[idx]==0){	
		close(1);	
		creat("out1",0644);
                sprintf(buf,"%d",idx);  
		argv[5]=buf;
		execvp( argv[0],argv);
	}
}

int findPidIndex(int pid){
	int i;
	for(i=0;i<NUMCHLDS;i++)
	{
		if(pids[i] == pid){
			return i;
   		}
	}
 	return -1;
}


int main(){
	FILE *infile;
	int ch;
	int i,status,index=0,counter=0;
	pid_t result;
		
	for (counter = 0; counter < NUMCHLDS; ++counter) {
		pids[counter]= fork();
    		if (pids[counter]) {	
	    		printf("Reader spawned pid is %d\n",pids[counter]);
        		continue;
    		} else if (pids[counter] == 0) {
			spawnReader(0,counter);
        		break;
    		} else {
        		printf("fork error\n");
        		exit(1);
    		}
	}	

	while(1){
 		sleep(5);
		result = waitpid(-1, &status, WNOHANG);
           	if(result!=0 && result!=-1){
             		printf("Reader with pid %d exited \n",result);
             		spawnReader(result,-1);
           	}	
		infile = fopen("kill.txt", "r"); 
		ch = getc(infile);
        	fclose(infile);
		if(ch=='1'){
	  		for(i=0;i<NUMCHLDS;i++){
                		printf("killing reader with pid %d \n",pids[i]);
				kill(pids[i],SIGKILL);
        		}		
			break;
		}
     		index++;
     		if(index>NUMCHLDS-1)index =0;
	}
	
	return 0;
}
