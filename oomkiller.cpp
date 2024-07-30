//============================================================================
// Name        : oomkiller.cpp
// Author      : odity
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : OOM killer
//============================================================================

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <signal.h>
#include <sys/resource.h>

using namespace std;

struct MemInfo{
	long MemTotal;
	long MemAvail;
	long MemFree;
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
			cout<<"Potential: "<<pid<<" "<<p.oom_score_adj<<" "<<p.oom_score<<" "<<p.vm_rss<<endl;
		}
	}
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

	cout<<"Kill process "<<name<<" of pid "<<pid<<endl;
	if(kill(badpid, 9) != 0){
		perror("Could not kill process :(");
		sleep(1);
	}
}

int main(int argc, char **argv) {
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
	long proc;
	while(1){
		m = getstate();
		proc = 100 * m.MemFree/m.MemTotal;
		if (proc < 3) //3% free
		{
			cout<<"Alarm: memory "<<m.MemFree<<"Mb"<<endl;
			killindir(procdir);
		}
		usleep(50000); // 10ms
	}
	return 0;
}
