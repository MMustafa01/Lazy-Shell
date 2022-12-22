#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parse.h"
#include <unistd.h> /*This is for the linux*/ 
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>


int bk_index = 0;
int background[10] = {0,0,0,0,0,0,0,0,0,0}; //Array to store background pid
char* jobs[10]; //list of all background jobs
char *history[1000]; // THis is for the history command
int h_front = 0;
int h_back = 0;

void handler(int sig)
{	
	
	int x = 0;
    int pid = waitpid(0, NULL, WNOHANG);
	int j;	
	for (j = 0; j < 10; j++)
		{
			if (background[j] == pid )
			{
				x  = j;
				background[j] = 0; // if background[j] job has finished then we make that index zero, i.e. job is finished
				jobs[j] = "";// A "" means that the job has been finished
				printf("Background Job with pid == %d has been finished", pid);
			}
				
		}
	
									     			
 }

int main()
{
	signal(SIGCHLD, handler);
    
    
	int c = 0;
    while(1){
        /*
        1)Display the prompt
        2)input the command 
        3)parse the command*/
        //implement minishell here
		char cwd[256];
		getcwd(cwd, sizeof(cwd));
		int i =0;
       	int j = 0;
		
        char buf[1000];
		char *dir;
		char *gdir;
		char *to;
		int exit_bool = 1;

		
		
        printf("%s>>", cwd);
        parseInfo *p;//making a object of type parseInfo
        char cmdline[200];
        scanf("%[^\n]%*c",cmdline);//taking the input line
        p =parse(cmdline);
		int pn = p->pipeNum;

		/*change directory*/
		if (!strcmp(p->CommArray[i].command, "cd")){
			gdir = getcwd(cwd, sizeof(cwd));
			dir = strcat(gdir,"/");
			to =strcat(dir,p->CommArray[0].VarList[1]);
			int check;			
			check = chdir(to);
			if (check != 0)
				printf("Error. non existing working directory\n");
			continue;
		}
		
		/*kill pid*/
		
		if (!strcmp(p->CommArray[i].command, "kill")){
			printf("kill is running \n");
			int y = atoi(p->CommArray[i].VarList[0]);			
					
				
			background[y] = 0;//This will result in error if a number is not given
			jobs[y] = "";
			continue;
		}	


		/*history*/
        if (!strcmp(cmdline, "history")){
	
			for (i = h_back; i < (h_front - h_back); i++)
			{
				printf(" %d -- >%s \n", i,history[i]);
			
			}
			
			continue;
		}
		
		/*maintaining history*/
		if (h_front - h_back == 10)
		{		
			history[h_front++] = cmdline;
			history[h_back++] = ""; //"" signifies that an element has been removed from that position
			//printf("history[%d] = %s\n", h_front -1 , history[h_front -1]);
		}
		else if (h_front - h_back < 10)
		{		
			history[h_front++] = cmdline;
			history[h_back] = ""; //"" signifies that an element has been removed from that position
			//h_back would logically be 0 here
			//printf("history[%d] = %s \n", h_front -1 , history[h_front -1]);
		}
		/*help*/

		if (!strcmp(cmdline, "help")){
			printf("jobs        ---> Provides list of all background processes and their local PIDS \n");
			printf("cd PATHNAME ---> Set Pathname as the working directory \n");
			printf("history     ---> prints a list of previously executed commands.  \n");
			printf("Kill PID    ---> Terminates the background processes identified by local PIDS \n");
			printf("exit        ---> terminates the shell only if ther ae no background jobs \n");
			printf("help        ---> Prints list of bultin commands \n");
			
		}
			
		
		/*exit()*/
		if (!strcmp(cmdline, "exit")){
			for(i = 0; i < 10 ; i++)
			{
				if (background[i] != 0)
					exit_bool = 0;
			}
			if (exit_bool == 1)
				exit(1);
			else
			{
				printf("There are background jobs present, unable to exit");
				continue;
			}
		}
			
		
		//printf("cmdline = %s\n",cmdline);
		if (!strcmp(cmdline,"jobs"))
		{
			//printf("This condiiton runs\n");
			for (i = 0; i<10; i++)
				printf("%s has a local pid of %d \n", jobs[i], i);
			continue;		
		}
        
		int errors = 0; //will count the number of errors

        int pid = fork();
        if (pid == -1)
        {
            printf("Error %d. has occured in creating the child process",  ++errors);
        }
        else if (pid == 0)
        {
            //child process
            
			


            /*Check if background process*/
            if(p->boolBackground && bk_index == 9)
            {
				 printf("There are already 10 background processes running\n");
				continue;
				//background[bk_index++] = p; //adding parseInfo to background array
                //Send signal to the process indicating the the this is background
            }


            if (p->boolInfile)
            {
                //Check fo access
                
                
                
                //The command has specified an input file
                int file_inp = open(p->inFile, O_RDONLY);
                /*if (file_inp == -1)
                {
                    printf("error in opening file\n"); 
                    return 2;
                } */
				if (access(p->inFile, F_OK) == -1)//(file_inp == NULL)
				{
					printf("Error. %d the file doesnot exist \n",++errors);				
								
				} 
                int fd_access = access(p->inFile, R_OK);
                if (fd_access == -1)
                {
                    printf("Error. %d The file is not readable \n", ++errors);
                    
                } 
                int file_inp2 = dup2(file_inp, STDIN_FILENO);
                close(file_inp);
				int x;
            }
            if (p->boolOutfile)
            {
                //Check fo access
                
                //The command has specified an output file
                int file = open(p->outFile, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
				int fd_access = access(p->outFile, W_OK);
                if (fd_access == -1)
                {
                    printf("Error. %d The file is not writable \n",  ++errors);
                  
                }
                if (file == -1)
                {
                    printf("Error. %d in creating file\n",  ++errors); 
                    
                }  
                int file2 = dup2(file, STDOUT_FILENO);
                close(file);
            }
			if (errors > 0)
				return 2;

			//printf("pipenum: %d\n", pn);
			for (i = 0;  i < pn; i++)
			{
				int fd[2];
				pipe(fd);
				int pid_grand = fork();
				if (pid_grand < 0)
				{
					printf("Erorr. Creating fork() \n");
					exit(1);
				}
				else if(pid_grand == 0)//grandchild process
				{
					dup2(fd[1], 1);//redirect output to write end
					close(fd[0]);
					//printf("%d -- %s\n", i, p->CommArray[i].command);
					execvp(p->CommArray[i].command,p->CommArray[i].VarList);
				}
				else
				{
					//parents read the input
					dup2(fd[0],0);//redirect iinput to read end
					close(fd[1]);
				}
			}
			//printf("%d -- %s\n", pn, p->CommArray[pn].command);
            execvp(p->CommArray[pn].command,p->CommArray[pn].VarList);

        }
        else
        {
            //int status=0;
			//wait(NULL);
            if (p->boolBackground)
            {
               int x = 0;
				for (i = 0; i < 10 ; i++)
				{
					if (background[i] == 0 )
					{	
						background[i] = pid;
						jobs[i] = p->CommArray[0].command;
						printf("%s put in jobs[%d]\n", p->CommArray[0].command, i);
						break;
					}
				}
				
				/*if (bk_index != 9)
					background[bk_index++] = pid;*/
            }
            else
                waitpid(pid, NULL,0);
            //printf("some post processing \n");
        }
        
		
    }
	
    return 0;
}
