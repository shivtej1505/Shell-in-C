#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/stat.h> 
#include <signal.h>

char *curComm[100],pipedInput[100][100];
// For input-output redirection
int in, out;
int fd[2];
char *repHome(char *pwd,char *home)
{
	int lp,lh,i,min;
	lp = strlen(pwd);
	lh = strlen(home);
	min = ( (lp) < (lh) ? (lp) : (lh));

	for(i = 0; i<min; i++)
	{
		if(pwd[i] != home[i])
			break;
	}
	if(i == lh)
	{
		pwd[0]='~';
		for(i = 1; i<=lp-lh; i++)
			pwd[i] = pwd[i+lh-1];
		pwd[i] = '\0';
		//printf("pwd: %s\n", pwd);
		return pwd;
	}
	else
		return pwd;
}
// Prints the prompt
void promtPrint()
{
	char sysName[1005];
	char temp[1005];
	char *user=NULL, *pwd=NULL, *home=NULL,*tmp=NULL;
	int res;

	user = getenv("LOGNAME");
	home = getenv("HOME");
	tmp = getcwd(tmp,1000);
	pwd = repHome(tmp, home);
	res = gethostname(sysName, 1000);
	//printf("PWD: %s \n HOME: %s\n", pwd, home);

	if(res == 0)
		printf("%s@%s:%s$ ",user, sysName, pwd);
	else
		// can't get system name
		printf("An error occured.");
}

void exe_commmand(int args, int redirect)
{
	int i;
	if(!strcmp(curComm[0],"cd"))
	{
		if(args == 1 || !strcmp(curComm[1],"~") || !strcmp(curComm[1],"~/") )
		{
			int result = chdir(getenv("HOME"));
			if(result != 0)
				printf("Error going in home directory.\n");
		}
		else if(args == 2)
		{
			int result = chdir(curComm[1]);
			if(result == 0)
				setenv("PWD",curComm[1],1);
			else
				printf("No such directory.\n");
		}
		else
		{
			printf("cd command take 2 arguments. %d arguments given\n",args);
		}
	}
	else
	{
		pid_t pidM;
		pidM = fork();
		if (pidM < 0)
		{
			printf("Cannot create child process.\n");
			_exit(-1);
		}
		else if(pidM == 0)
		{
			if(redirect == 0 || redirect == 3)  // For input redirection
				dup2(in, 0);
			if(redirect == 1 || redirect == 3)  // For output redirection
				dup2(out, 1);
			int isError = execvp(curComm[0],curComm);
			if( isError == -1)
				printf("%s: command not found.",curComm[0]);
			_exit(0);
		}
		waitpid(pidM,NULL,0);
		return;
	}
}

int parser(char *command,int size,int noSpace)
{
	int i,j,k=0;
	for(i=0;i<=noSpace;i++)
	{
		curComm[i] = malloc(100*sizeof(char));
		if(k>size)
			break;
		j=0;
		while(command[k] != ' ' && command[k] != '\0')
		{
			curComm[i][j] = command[k];
			j++;
			k++;
		}
		if(command[k] == ' ' || command[k] == '\0')
		{
			curComm[i][j]='\0';
			//printf("%d. %s\n",i, curComm[i]);
			k++;
		}
	}
	for(;i<100;i++)
		curComm[i]='\0';
	return noSpace+1;
}

int calSpace(char *stg)
{
	int i,l = strlen(stg);
	int space = 0;
	for(i=0; i<l; i++)
	{
		if(stg[i] == ' ')
			space++;
	}
	return space;
}

// Cutoff unused spaces in commands;
char *cutOffSpace(char *strg)
{
	int l=strlen(strg),i,j=0;
	char *sgt = (char*)malloc(sizeof(char) * 10000);
	if(strg[0] == ' ')
		i=1;
	else
		i=0;
	if(strg[l-1] == ' ')
	{
		while(i<l-1)
			sgt[j++]=strg[i++];
	}
	else
	{
		while(i<l)
			sgt[j++]=strg[i++];
	}
	sgt[j]='\0';
	return sgt;
}
void broken_pipe(char *raw_command)
{
	char *token=NULL,*savePtr=NULL,*init=NULL,*cmd_one=NULL,*cmd_two=NULL;
	int no_pipe = 0,i=0,j;
	while(raw_command[i] != '\0')
	{
		if(raw_command[i] == '|')
			no_pipe++;
		i++;
	}
	printf("%d\n",no_pipe);
	if(raw_command[i-1] == '|')
	{
		perror("Invalid command\n");
		return;
	}
	if(no_pipe)
	{
		j=0;
		for(init = raw_command;;init = NULL)
		{
			token = strtok_r(init,"|",&savePtr);
			if(token == NULL)
				break;
			strcpy(pipedInput[j],token);
			j++;
		}
		execute_pipe(j);
	}
	else
		redirection(raw_command);
}

void execute_pipe(int size)
{
	int h,j,status=0,cpid;
	int pipes[size][2];
	pid_t pid;
	for(j=0;j<size;j++)
		pipe(pipes[j]);
	for(h=0;h<size;h++)
	{
		printf("%s\n",pipedInput[h]);
		char *filtL=NULL,*filtR=NULL,*middle=NULL,*temp;
		int len,no_spaces,args,flagI=0,flagO=0,i;
		char *saveL=NULL,*saveR=NULL,*command;
		// Finding directions
		i=0;
		command = pipedInput[h];
		while(command[i] != '\0')
		{
			if(command[i] == '<')
				flagI = 1;
			else if(command[i] == '>')
				flagO = 1;
			i++;
		}

		if(flagI)
			filtL = strtok_r(command,"<",&saveL);
		if(flagO)
		{
			if(flagI)
			{
				filtR = strtok_r(NULL,">",&saveL);
				middle = filtR;
				filtR = strtok_r(NULL,">",&saveL);
			}
			else
			{
				filtR = strtok_r(command,">",&saveR);
				middle = filtR;
				filtR = strtok_r(NULL,">",&saveR);
			}
		}
		else
		{
			if(filtL == NULL)
				middle = command;
			else
				middle = strtok_r(NULL,"<",&saveL);
		}

		if(filtL == NULL)
			temp = cutOffSpace(middle);
		else
			temp = cutOffSpace(filtL);

		no_spaces = calSpace(temp);
		len = strlen(temp);
		args = parser(temp, len, no_spaces);
		printf("temp:%s\nlen:%d\nspaces:%d\n",temp,len,no_spaces);
		//continue;
		pid = fork();
		if(pid<0)
			perror("fork error");
		else if(pid == 0)
		{
			if(flagO)
			{
				out = open(cutOffSpace(filtR),O_RDONLY | O_WRONLY | O_CREAT, 0666);
				dup2(out,1);
				close(out);
			}
			else if( h != size-1)
			{
				if((dup2(pipes[h][1],1))<0)
					perror("dup2 error");
			}
			if(flagI)
			{
				in = open(cutOffSpace(middle),O_RDONLY);
				dup2(in,0);
				close(in);
			}
			else if(h)
			{
				if((dup2(pipes[h-1][0],0))<0)
					perror("dup2 error");
			}
			if((execvp(curComm[0],curComm))<0)
			{
				perror("can't execute this command");
				_exit(-1);
			}
			_exit(0);
		}
		else;
			//waitpid(pid,&status,0);
	}
	for(j=0;j<h;j++)
	{
		close(pipes[0][j]);
		close(pipes[1][j]);
	}
}

void redirection(char *command)
{
	char *filtL=NULL,*filtR=NULL,*middle=NULL,*temp;
	int len,no_spaces,args,flagI=0,flagO=0,i;
	char *saveL=NULL,*saveR=NULL;
	// Finding directions
	i=0;
	while(command[i] != '\0')
	{
		if(command[i] == '<')
			flagI = 1;
		else if(command[i] == '>')
			flagO = 1;
		i++;
	}

	if(flagI)
		filtL = strtok_r(command,"<",&saveL);
	if(flagO)
	{
		if(flagI)
		{
			filtR = strtok_r(NULL,">",&saveL);
			middle = filtR;
			filtR = strtok_r(NULL,">",&saveL);
		}
		else
		{
			filtR = strtok_r(command,">",&saveR);
			middle = filtR;
			filtR = strtok_r(NULL,">",&saveR);
		}
	}
	else
	{
		if(filtL == NULL)
			middle = command;
		else
			middle = strtok_r(NULL,"<",&saveL);
	}

	if(filtL == NULL)
		temp = cutOffSpace(middle);
	else
		temp = cutOffSpace(filtL);

	no_spaces = calSpace(temp);
	len = strlen(temp);
	args = parser(temp, len, no_spaces);

	if(flagI && flagO)
	{
		in = open(cutOffSpace(middle), O_RDONLY);
		out = open(cutOffSpace(filtR),O_RDONLY | O_WRONLY | O_CREAT, 0666);
		exe_commmand(args,3);
		close(in);
		close(out);
	}
	else if(flagI)
	{
		in = open(cutOffSpace(middle),O_RDONLY);
		exe_commmand(args,0);
		close(in);
	}
	else if(flagO)
	{
		out = open(cutOffSpace(filtR),O_RDONLY | O_WRONLY | O_CREAT, 0666);
		exe_commmand(args,1);
		close(out);
	}
	else
		exe_commmand(args,-1);
}

int main()
{	
	// The main shell loop
	int args;
	char c;
	char *token;
	char tmp[1000005];
	int noChars,flag,noSpace;
	while(1)
	{
		noChars=0;
		noSpace=0;
		flag=0;
		promtPrint();
		c = getc(stdin);
		while(c != '\n')
		{
			while(isspace(c))
			{
				if(noChars)
					flag=1;
				c = getc(stdin);
				if(c == '\n')
					break;
			}
			if(c == '\n')
				break;
			if(flag)
			{
				noSpace++;
				tmp[noChars++]=' ';
				flag=0;
			}
			tmp[noChars++]=c;
			c = getc(stdin);
		}
		tmp[noChars]='\0';
		//printf("%s\n",tmp);
		token = strtok(tmp,";");
		char *temp;
		while(token != NULL)
		{
			temp = cutOffSpace(token);	
			broken_pipe(temp);
			//redirection(temp);
			token = strtok(NULL, ";");
		}
		//printf("%s\n",tmp);
	}
	return 0;
}
