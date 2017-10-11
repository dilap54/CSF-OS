#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
	int awkToSortPipe[2], sortToHeadPipe[2], headToBufferPipe[2], bufferToAwk2Pipe[2], awk2ToTotalPipe[2], totalToAwk3Pipe[2], status;
	
	pipe(awkToSortPipe);
	pipe(sortToHeadPipe);
	pipe(headToBufferPipe);
	pipe(bufferToAwk2Pipe);
	pipe(awk2ToTotalPipe);
	pipe(totalToAwk3Pipe);
	

	pid_t pidAwk = fork();
	
	if (!pidAwk) { 

		dup2(awkToSortPipe[1], 1);
		
		close(awkToSortPipe[0]);
		close(awkToSortPipe[1]);
		close(sortToHeadPipe[0]);
		close(sortToHeadPipe[1]);
		close(headToBufferPipe[0]);
		close(headToBufferPipe[1]);
		close(bufferToAwk2Pipe[0]);
		close(bufferToAwk2Pipe[1]);
		close(awk2ToTotalPipe[0]);
		close(awk2ToTotalPipe[1]);
		close(totalToAwk3Pipe[0]);
		close(totalToAwk3Pipe[1]);

		char* command[4] = {"awk", "{\
				sum[substr($4,2,11)]+=$10;\
				total+=$10;\
			} \
			END {for(i in sum) {\
				print i, sum[i]\
			}\
		}", "../log.txt", 0};
		execvp(command[0], command);

		exit(EXIT_FAILURE);
	} else if (pidAwk == -1) {
		fprintf(stderr, "Can't fork, exiting...\n");
		exit(EXIT_FAILURE);
	}


	pid_t pidSort = fork();
	
	if (!pidSort) {
		/* В этом процессе мы будем читать из канала */
		// перенапрявляем вывод канала в STDIN (fd==0)
		dup2(awkToSortPipe[0], 0);
		dup2(sortToHeadPipe[1], 1);
		
		close(awkToSortPipe[0]);
		close(awkToSortPipe[1]);
		close(sortToHeadPipe[0]);
		close(sortToHeadPipe[1]);
		close(headToBufferPipe[0]);
		close(headToBufferPipe[1]);
		close(bufferToAwk2Pipe[0]);
		close(bufferToAwk2Pipe[1]);
		close(awk2ToTotalPipe[0]);
		close(awk2ToTotalPipe[1]);
		close(totalToAwk3Pipe[0]);
		close(totalToAwk3Pipe[1]);

		char* command[4] = {"sort", "-k", "2,2nr", 0};
		execvp(command[0], command);
		
		exit(EXIT_FAILURE);
	} else if (pidSort == -1) {
		fprintf(stderr, "Can't fork, exiting...\n");
		exit(EXIT_FAILURE);
	}
	
	

	pid_t pidHead = fork();

	if (!pidHead){
		dup2(sortToHeadPipe[0],0);
		dup2(headToBufferPipe[1],1);

		close(awkToSortPipe[0]);
		close(awkToSortPipe[1]);
		close(sortToHeadPipe[0]);
		close(sortToHeadPipe[1]);
		close(headToBufferPipe[0]);
		close(headToBufferPipe[1]);
		close(bufferToAwk2Pipe[0]);
		close(bufferToAwk2Pipe[1]);
		close(awk2ToTotalPipe[0]);
		close(awk2ToTotalPipe[1]);
		close(totalToAwk3Pipe[0]);
		close(totalToAwk3Pipe[1]);

		char* command[3] = {"head", "-10", 0};
		execvp(command[0], command);

		

		exit(EXIT_FAILURE);
	} else if (pidHead == -1) {
		fprintf(stderr, "Can't fork, exiting...\n");
		exit(EXIT_FAILURE);
	}
	
	pid_t pidawk2 = fork();
	
	if (!pidawk2){
		dup2(bufferToAwk2Pipe[0],0);
		dup2(awk2ToTotalPipe[1],1);

		close(awkToSortPipe[0]);
		close(awkToSortPipe[1]);
		close(sortToHeadPipe[0]);
		close(sortToHeadPipe[1]);
		close(headToBufferPipe[0]);
		close(headToBufferPipe[1]);
		close(bufferToAwk2Pipe[0]);
		close(bufferToAwk2Pipe[1]);
		close(awk2ToTotalPipe[0]);
		close(awk2ToTotalPipe[1]);
		close(totalToAwk3Pipe[0]);
		close(totalToAwk3Pipe[1]);

		char* command[3] = {"awk", "{total = total + $2} END { print total }",0};
		execvp(command[0], command);

		exit(EXIT_FAILURE);
	} else if (pidawk2 == -1) {
		fprintf(stderr, "Can't fork, exiting...\n");
		exit(EXIT_FAILURE);
	}

	char REFS_LIST[1024] = "";
	char totalc[32] = "";


	close(awkToSortPipe[0]);
	close(awkToSortPipe[1]);
	close(sortToHeadPipe[0]);
	close(sortToHeadPipe[1]);

	read(headToBufferPipe[0], REFS_LIST, sizeof(REFS_LIST)-1);
	close(headToBufferPipe[0]);
	close(headToBufferPipe[1]);

	write(bufferToAwk2Pipe[1], REFS_LIST, sizeof(REFS_LIST)-1);
	close(bufferToAwk2Pipe[0]);
	close(bufferToAwk2Pipe[1]);

	read(awk2ToTotalPipe[0], totalc, sizeof(totalc)-1);	
	close(awk2ToTotalPipe[0]);
	close(awk2ToTotalPipe[1]);

	pid_t pidawk3 = fork();
	if (!pidawk3){
		dup2(totalToAwk3Pipe[0],0);

		close(awkToSortPipe[0]);
		close(awkToSortPipe[1]);
		close(sortToHeadPipe[0]);
		close(sortToHeadPipe[1]);
		close(headToBufferPipe[0]);
		close(headToBufferPipe[1]);
		close(bufferToAwk2Pipe[0]);
		close(bufferToAwk2Pipe[1]);
		close(awk2ToTotalPipe[0]);
		close(awk2ToTotalPipe[1]);
		close(totalToAwk3Pipe[0]);
		close(totalToAwk3Pipe[1]);

		char totalc2[80] = "";
		strcpy(totalc2, "--assign=");
		strcat(totalc2, "total=");
		strcat(totalc2, totalc);

		char* command[4] = {"awk", totalc2,  "{\
			printf(\"%d. %s - %d - %.2f%%\\n\", NR, $1, $2, $2/total*100)\
		}",0};
		execvp(command[0], command);

		exit(EXIT_FAILURE);
	} else if (pidawk3 == -1) {
		fprintf(stderr, "Can't fork, exiting...\n");
		exit(EXIT_FAILURE);
	}

	write(totalToAwk3Pipe[1], REFS_LIST, sizeof(REFS_LIST)-1);
	close(totalToAwk3Pipe[0]);
	close(totalToAwk3Pipe[1]);

	waitpid(pidAwk, NULL, 0);
	waitpid(pidSort, NULL, 0);
	waitpid(pidHead, NULL, 0);
	waitpid(pidawk2, NULL, 0);
	waitpid(pidawk3, &status, 0);

	exit(status);
	
	return 0;
}