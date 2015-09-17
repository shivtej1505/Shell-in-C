#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

char *curComm[100];
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
	char *user=NULL, *pwd=NULL, *home=NULL,*tmp=NULL;
	int res;

	user = getenv("LOGNAME");
	home = getenv("HOME");
	pwd = repHome(getenv("PWD"), home);
	res = gethostname(sysName, 1000);
	//printf("PWD: %s \n HOME: %s\n", pwd, home);

	if(res == 0)
		printf("%s@%s:%s$ ",user, sysName, pwd);
	else
		// can't get system name
		printf("An error occured.");
}

void exe_commmand(int args)
{
	int i;
	/*
	   for(i=0;i<args;i++)
	   printf("%d. %s\n",i,curComm[i]);
	   */
	if(!strcmp(curComm[0],"cd"))
	{
		if(args == 1)
		{
			int result = chdir(getenv("HOME"));
			if(result == 0)
				setenv("PWD",getenv("HOME"),1);
		}
		else if(args == 2)
		{
			int result = chdir(curComm[1]);
			if(result == 0)
				setenv("PWD",curComm[1],1);
			else
				printf("error.\n");
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
			execvp(curComm[0],curComm);
			_exit(0);
		}
		wait();
		return;
	}
}
int parser(char *command,int size,int noSpace)
{
	printf("fine\n");
	int i,j,k=0;
	for(i=0;i<=noSpace;i++)
	{
		curComm[i] = malloc(100*sizeof(char));
		if(k>size)
		{
			// May it happen
			break;
		}
		j=0;
		printf("%d\n",k);
		while(command[k] != ' ' && command[k] != '\0')
		{
			curComm[i][j] = command[k];
			j++;
			k++;
		}
		if(command[k] == ' ' || command[k] == '\0')
		{
			curComm[i][j]='\0';
			//		printf("%d. %s\n",i, curComm[i]);
			k++;
		}
	}
	for(;i<100;i++)
		curComm[i]='\0';
	return noSpace+1;
}
int calSpace(char *stg)
{
	int i,l=strlen(stg);
	int space=0;
	for(i=0;i<l;i++)
	{
		if(stg[i]==' ')
			space++;
	}
	return space;
}
char *cutOffSpace(char *strg)
{
	int l=strlen(strg),i,j=0;
	char sgt[1000];
	for(i=0;i<l;i++)
	{
		if(strg[0] != ' ')
			break;
	}
	for(;i<l;i++)
		sgt[j++]=strg[i];
	sgt[j]='\0';
	return sgt;
}
int main()
{	
	// The main shell loop
	int args;
	char c;
	char *tokens;
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
				c=getc(stdin);
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
		tokens = strtok(tmp,";");
		while(tokens != NULL)
		{
			int lol = calSpace(tokens);
			int len = strlen(tokens);
			printf("%s %d %d\n",tokens,len,lol);
			//args = parser(tokens, noChars,noSpace);
			args = parser(cutOffSpace(tokens),len,lol);
			exe_commmand(args);
			tokens = strtok(NULL, ";");
		}
		//printf("%s\n",tmp);
	}
	return 0;
}
