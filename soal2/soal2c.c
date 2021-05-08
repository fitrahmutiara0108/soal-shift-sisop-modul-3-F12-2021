#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int status;
int fd1[2];
int fd2[2];

int main()
{
    pid_t child_id;
    if (pipe(fd1) == -1){
        exit(1);
    }
    if (pipe(fd2) == -1){
        exit(1);
    }

    child_id = fork();
    //child 1
	if (child_id == 0) {

        //write to fd1
		dup2(fd1[1], 1);
        //close fd1
        close(fd1[0]);
        close(fd1[1]);

		char *argv1[] = {"ps", "-aux", NULL};
		execv("/bin/ps", argv1);
	}

	else {

		while(wait(&status) > 0);
        child_id = fork();
        //child 2
		if (child_id == 0) {

            //read from fd1
            dup2(fd1[0], 0);
            //write to fd2
            dup2(fd2[1], 1);

            //close fd
 	        close(fd1[0]);
		  	close(fd1[1]);
		  	close(fd2[0]);
		  	close(fd2[1]);

            char *argv2[] = {"sort", "-nrk", "3,3", NULL};
            execv("/usr/bin/sort", argv2);
		}
        //masuk parent
		else {
            //close unused fd1
			close(fd1[0]);
			close(fd1[1]);

			while(wait(&status) > 0);

            //read from fd2
			dup2(fd2[0], 0);
            //close fd2
			close(fd2[0]);
            close(fd2[1]);

			char *argv3[] = {"head", "-5", NULL};
			execv("/usr/bin/head", argv3);
		}
	}
}