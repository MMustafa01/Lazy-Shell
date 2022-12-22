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
// #include "builtin.c"

int local_pid = 0;
int background[10] = {0,0,0,0,0,0,0,0,0,0}; //Array to store background pid
char* jobs[10]; //list of all background jobs
char *history[1000]; // THis is for the history command
int fp = 0;
int bp = 0;
char buf[1000];
char *dir;
char *gdir;
char *to;
int exit_bool = 1;
int c = 0;
int builtin_flag = 0;
char curent_directory[256];
int i =0;
int j = 0;

void sigchild_handler(int sig)
{	
	
	int x = 0;
    int pid = waitpid(0, NULL, WNOHANG);
	int q;	
	for (q = 0; q < 10; q++)
		{
			if (background[q] == pid )
			{
				x  = q;
				background[q] = 0; // if background[j] job has finished then we make that index zero, i.e. job is finished
				jobs[q] = "";// A "" means that the job has been finished
				printf("Background Job with pid == %d has been finished", pid);
			}
				
		}
	
									     			
 }


void change_directory(parseInfo *p)
{
	gdir = getcwd(curent_directory, sizeof(curent_directory));
	dir = strcat(gdir,"/");
	to =strcat(dir,p->CommArray[0].VarList[1]);
	int flag;			
	flag = chdir(to);
	if (flag != 0)
		printf("Error. non existing working directory\n");
	return ;

}

void kill_pid(parseInfo *p)
{
	printf("kill is running \n");
	int kill_flag = atoi(p->CommArray[i].VarList[0]);			
	background[kill_flag] = 0;//This will result in error if a number is not given
	jobs[kill_flag] = ""; 
	return;
}

void history_print(parseInfo *p)
{
    for (i = bp; i < (fp - bp); i++)
        {
            printf(" %d -- >%s \n", i,history[i]);
        
        }
}

void jobsbuiltin()
{
    for (i = 0; i<10; i++)
        printf("%s has a local is mapped to ---> %d \n", jobs[i], i);
    return;
}

void exiting(parseInfo *p)
{
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
			
		}
    return;
		
}
void builtin(char *entered_command, parseInfo * p)
{	
	/*change directory*/
	if (!strcmp(p->CommArray[i].command, "cd")){
		change_directory(p);
		builtin_flag = 1;
	}
	
	/*kill pid*/
	
	if (!strcmp(p->CommArray[i].command, "kill")){
		kill_pid(p);
		builtin_flag = 1;
	}	


	/*history*/
	if (!strcmp(entered_command, "history")){
        history_print(p);
		builtin_flag = 1;
	}
	
	
	/*help*/

	if (!strcmp(entered_command, "help")){
		printf("jobs: \t\tProvides list of all background processes and their local PIDS \n");
		printf("history: \t\tprints a list of previously executed commands.  \n");
		printf("Kill PID: \t\tTerminates the background processes identified by local PIDS \n");
		printf("exit: \t\tterminates the shell only if ther ae no background jobs \n");
		printf("help: \t\tPrints list of bultin commands \n");
		builtin_flag = 1;
	}
		
	
	/*exit()*/
	if (!strcmp(entered_command, "exit")){
		exiting(p);
        builtin_flag = 1;
	}
		
	
	//printf("entered_command = %s\n",entered_command);
	if (!strcmp(entered_command,"jobs"))
	{
		//printf("This condiiton runs\n");
		jobsbuiltin();
		builtin_flag = 1;		
	}
	return;
}


void piping(parseInfo *p, int number_pipes)
{   
    for (i = 0;  i < number_pipes; i++)
    {
        int fd[2];
        pipe(fd);
        int pid_grand = fork();
        if (pid_grand < 0)
        {
            printf("Erorr. Creating fork() \n");
            exit(1);
        }
        else if(pid_grand == 0)
        {
            dup2(fd[1], 1);
            close(fd[0]);

            execvp(p->CommArray[i].command,p->CommArray[i].VarList);
        }
        else
        {

            dup2(fd[0],0);
            close(fd[1]);
        }
    }
}


void in_out_redirect(parseInfo *p, int errors)
{
    if (p->boolInfile)
    {
   
        
        
        int file_inp = open(p->inFile, O_RDONLY);
    
        if (access(p->inFile, F_OK) == -1)
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
        exit(1);
    return;    
}

int main()
{
	signal(SIGCHLD, sigchild_handler);
    
    
	
    while(1){
  
		
		getcwd(curent_directory, sizeof(curent_directory));
		
		
        printf("%s>>", curent_directory);
        parseInfo *p;
        char entered_command[200];
        scanf("%[^\n]%*c",entered_command);
        p =parse(entered_command);
		int number_pipes = p->pipeNum;

		/*maintaining history*/
		if (fp - bp == 10)
		{		
			history[fp++] = entered_command;
			history[bp++] = "";
			
		}
		
		builtin(entered_command,p);
		if(builtin_flag == 1)
		{
			builtin_flag = 0;
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
            if(p->boolBackground && local_pid == 9)
            {
				 printf("There are already 10 background processes running\n");
				continue;
				//background[local_pid++] = p; //adding parseInfo to background array
                //Send signal to the process indicating the the this is background
            }

            in_out_redirect(p, errors);

            
            
            piping(p, number_pipes);
			

            execvp(p->CommArray[number_pipes].command,p->CommArray[number_pipes].VarList);

        }
        else
        {
            
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
				

            }
            else
                waitpid(pid, NULL,0);

        }
        
		
    }
	
    return 0;
}
