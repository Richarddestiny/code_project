#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>

void printdir(char *dir, int depth)
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
    struct tm *local_time = NULL;
    char str_time[100];
	if ((dp = opendir(dir)) == NULL) {
		fprintf(stderr, "Can`t open directory %s\n", dir);
		return ;
	}
	
	chdir(dir);
	while ((entry = readdir(dp)) != NULL) {
		lstat(entry->d_name, &statbuf);
		if (S_ISDIR(statbuf.st_mode)) {
			if (strcmp(entry->d_name, ".") == 0 || 
				strcmp(entry->d_name, "..") == 0 )  
				continue;	
            local_time = localtime(&statbuf.st_mtime);
            strftime(str_time,sizeof(str_time),"%Y-%m-%d,%H:%M:%S",local_time);
            printf("DIR:%*s%s/   %s\n", depth, "", entry->d_name, str_time);
			printdir(entry->d_name, depth+4);
		} else
        {
            local_time = localtime(&statbuf.st_mtime);
            strftime(str_time,sizeof(str_time),"%Y-%m-%d,%H:%M:%S",local_time);
            printf("FILE:%*s%s   %s\n", depth, "", entry->d_name,str_time);

    	}
    }
	chdir("..");
	closedir(dp);	
}


int main(int argc, char *argv[])
{
	char *topdir = ".";
	if (argc >= 2)
		topdir = argv[1];

	printf("Directory scan of %s\n", topdir);
	printdir(topdir, 0);
	printf("done.\n");
	exit(0);
}
