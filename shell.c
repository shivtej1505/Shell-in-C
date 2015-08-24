#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

char *repHome(char *pwd, char *home)
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
	if(min == lh)
	{
		pwd[0]='~';
		for(i = 1; i<=lp-lh; i++)
			pwd[i] = pwd[i+lh-1];
		pwd[i] = '\0';
		printf("pwd: %s\n", pwd);
	}
	else
		return pwd;
}
// Prints the prompt
void promtPrint()
{
	char sysName[1005];
	char *user, *pwd, *home,*tmp;
	int res;

	user = getenv("LOGNAME");
	home = getenv("HOME");
	pwd = repHome(getenv("PWD"), home);
	res = gethostname(sysName, 1000);

	printf("user:%s\n home:%s\n pwd:%s\n",user,home,sysName);

	if(res == 0)
		printf("LOL%s@%s:%s$ ",user, sysName, pwd);
	else
		// can't get system name
		printf("An error occured.");
}
int main()
{	
	// The main shell loop
	char c;
	int noChars = 0;
	while(1)
	{
		noChars++;
		promtPrint();
		c = getc(stdin);
//		if(c == '\n')
			break;
	}
	return 0;
}
