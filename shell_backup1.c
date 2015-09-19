#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

char *curComm[100];
// custom input output pointer
int in, out;
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
			if(redirect == 0 || redirect == 3)      // stdin ---> in
				dup2(in,0);
			if(redirect == 1 || redirect == 3)      // stdout ---> out
				dup2(out,1);
			int isError = execvp(curComm[0],curComm);
			if( isError == -1)
				printf("%s: command not found.",curComm[0]);
			_exit(0);
		}
		wait();
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
	char sgt[1000];
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

void redirection(char *command)
{
	char *inputL,*outputR,*temp,*filtR,*filtL;
	int len,no_spaces,args,flagI=0,flagO=0;

	// finding args
	filtL = strtok(command,"<");
	filtR = strtok(NULL,">");
	if(filtR != NULL)
		temp = cutOffSpace(filtR);
	else
		temp = cutOffSpace(filtL);
	printf("%s\n",temp);
	no_spaces = calSpace(temp);
	len = strlen(temp);
	args = parser(temp,len,no_spaces);

	// Left side of '<'
	inputL = strtok(command,"<");
	inputL = strtok(NULL,"<");
	printf("L:%s\n",inputL);

	// Right side of '>'
	outputR = strtok(command,">");
	outputR = strtok(NULL,">");
	printf("R:%s\n",outputR);
	
	if(inputL != NULL)
	{
		printf("L:%s\n",inputL);
		printf("R:%s\n",outputR);
		in = open(cutOffSpace(inputL),O_RDONLY);
		flagI=1;
	}
	if(outputR != NULL)
	{
		out = open(cutOffSpace(outputR),O_RDONLY | O_WRONLY | O_CREAT, 0666);
		flagO=1;
	}
	if(flagI && flagO)
	{
		exe_commmand(args,3);
		close(in);
		close(out);
	}
	else if(flagI)
	{
		exe_commmand(args,0);
		close(in);
	}
	else if(flagO)
	{
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
			//int noS = calSpace(temp);
			//int len = strlen(temp);

			//args = parser(temp,len,noS);
			//exe_commmand(args);
			redirection(temp);
			token = strtok(NULL, ";");
		}
		//printf("%s\n",tmp);
	}
	return 0;
}