//============================================================================
// Name        : oomkiller.cpp
// Author      : odity
// Version     : 0.2
// Copyright   : Your copyright notice
// Description : oomkiller
//============================================================================

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <signal.h>
#include <sys/resource.h>

#define VERSION "0.2"
int notkill=0;
long memorylimit=0;
int killsign=9;
int list=0;
char nameprocess[256];

using namespace std;

struct MemInfo{
	long MemTotal;
	long MemAvail;
	long MemFree;
	long SwapFree;
	long SwapTotal;
};
struct ProcessInfo{
	int oom_score;
	int oom_score_adj;
	unsigned long vm_rss;
	int exited;
};
long findsubstring(char * name, char *buffer )
{
	char * hit = strstr(buffer, name);
	if (hit == NULL)
		return -1;
	long ret = strtol(hit+strlen(name),NULL,10);
	return ret;
}
struct MemInfo getstate(){
	FILE * fd;
	char buf[8192];
	struct MemInfo m;

	fd = fopen("/proc/meminfo","r");
	if (fd == NULL)
	{
		perror("Error open meminfo");
		exit(1);
	}
	size_t len = fread(buf,1,sizeof(buf)-1,fd);
	if (len == 0){
		perror("Error read meminfo");
		exit(2);
	}
	m.MemTotal = findsubstring("MemTotal:",buf);
	m.MemAvail = findsubstring("MemAvailable:",buf);
	m.MemFree  = findsubstring("MemFree:",buf);
	m.SwapFree  = findsubstring("SwapFree:",buf);
	m.SwapTotal  = findsubstring("SwapTotal:",buf);
	fclose(fd);
	return m;
}
static int isnumeric(char* str)
{
	int i=0;
	// Empty string is not numeric
	if(str[0]==0)
		return 0;
	while(1)
	{
		if(str[i]==0) // End of string
			return 1;
		if(isdigit(str[i])==0)
			return 0;
		i++;
	}
}
struct ProcessInfo getprocstate(int pid){
	struct ProcessInfo p = {0,0,0,0};
	char buff[256];
	FILE *f;
	snprintf(buff, sizeof(buff), "%d/oom_score", pid);
	f = fopen(buff,"r");
	if (f == NULL){
		p.exited = 1;
		return p;
	}
	fscanf(f, "%d", &(p.oom_score));
	fclose(f);
	snprintf(buff, sizeof(buff), "%d/oom_score_adj", pid);
	f = fopen(buff,"r");
	if (f == NULL){
		p.exited = 1;
		return p;
	}
	fscanf(f, "%d", &(p.oom_score_adj));
	fclose(f);
	snprintf(buff, sizeof(buff), "%d/statm", pid);
	f = fopen(buff,"r");
	if (f == NULL){
		p.exited = 1;
		return p;
	}
	fscanf(f, "%*u %lu", &(p.vm_rss));
	fclose(f);

	snprintf(buff, sizeof(buff), "%d/stat", pid);
	FILE * stat = fopen(buff, "r");
	if (stat == NULL )
	{
		p.exited = 1;
		return p;
	}

	fscanf(stat, "%*d %s", nameprocess);
	//cout<<name;
	//p.name = (p.name)malloc(sizeof (p)*strlen(256)+1);
	//fscanf(stat, "*d %s", &(p.name));
	//cout<<p.name;
	fclose(stat);
	return p;
}
void killindir(DIR * procdir){
	struct dirent * d;
	struct ProcessInfo p = {0,0,0,0};

	char buff[256];
	int bad = 0,badpid=0;
	int pid=0;
	int maybebad = 0;

	rewinddir(procdir);
	while(1){
		d= readdir(procdir);
		if (d == NULL){
			perror("Error read dir");
			break;
		}
		if(!isnumeric(d->d_name))
			continue;
		pid = strtol(d->d_name,NULL,10);
		if(getpid() == pid)
			continue;
		//cout<<d->d_name<<endl;
		p = getprocstate(pid);
		if (p.exited == 1)
			continue;
		maybebad = p.oom_score;
		if(p.oom_score_adj > 0)
			maybebad-=p.oom_score_adj;
		if(bad < maybebad){
			bad=maybebad;
			badpid = pid;
			cout<<"Potential: \t"<<nameprocess<<"\t "<<pid<<"\t "<<p.oom_score_adj<<"\t "<<p.oom_score<<"\t "<<p.vm_rss<<endl;
		}
	}
	if (list == 1)
		exit(0);
	if(badpid == 0){
		cout<<"Coudn't find a process for kill..."<<endl;
		sleep(1);
		return;
	}
	snprintf(buff, sizeof(buff), "%d/stat", badpid);
	FILE * stat = fopen(buff, "r");
	char name[256];
	fscanf(stat, "%*d %s", name);
	fclose(stat);
	if (notkill == 1)
		return;
	cout<<"Kill process "<<name<<" of pid "<<pid<<endl;
	if(kill(badpid, killsign) != 0){
		perror("Could not kill process :(");
		sleep(1);
	}
}
int main(int argc, char **argv) {
	int c;
	if (argv[0][0] == '-') argv[0]++;
	while((c = getopt(argc, argv, "l:nvs:r")) != EOF) {
			switch(c) {
				case 'n':
					notkill = 1;
					break;
				case 'r':
					list = 1;
					break;
				case 'l':
					memorylimit = atoi(optarg);
					if (memorylimit >100 || memorylimit <0)
						memorylimit = 3;
					break;
				case 's':
					killsign = atoi(optarg);
					if (killsign >15 || killsign <0)
						killsign = 9;
					break;
				default:
					fprintf(stderr, "Usage: %s [-r ] listprocessonly [-l percent|memorylimit%] [-s signkill(0-15)] [-n ]|(notkill process)\n",
					                   argv[0]);
					           exit(EXIT_FAILURE);
			}
		}
		///if (argc != optind) return 1;

	setpriority(PRIO_PROCESS,getpid(),-15);
	if(chdir("/proc")!=0)		{
		perror("Could not cd to /proc");
		exit(4);
	}

	DIR *procdir = opendir(".");
	if(procdir==NULL)
	{
		perror("Could not open /proc");
		exit(5);
	}

	struct MemInfo m;
	long procmem,procswap;
	while(1){
		m = getstate();
		procmem = 100 * m.MemFree/m.MemTotal;
		procswap = 100 * m.SwapFree/m.SwapTotal;
		if (memorylimit == 0)
			memorylimit = 3;
		if (( procmem < memorylimit && procswap < memorylimit*10) || list == 1) //3% free
		{
			cout<<"Alarm: memory "<<m.MemFree<<"Mb"<<endl;
			killindir(procdir);
		}
		usleep(50000); // 10ms
	}
	return 0;
}
