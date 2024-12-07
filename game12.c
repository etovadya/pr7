#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

int guess_the_number(int N, int pipe_read, int pipe_write) {
    int tries = 0;
    int guess;
    int round_end = 0; // if 1 --> end
    srand(time(NULL));
    int mid = rand() % (N + 1);
    if (read(pipe_read, &guess, sizeof(int)) <= 0) {
        return 1;
    }
    pid_t pid = getpid();
    while (round_end == 0) {
        tries++;
        printf("Try # %d: %d\n", tries, mid);
        if (mid == guess) {
            time_t end_time = time(NULL); //for random
            printf("Ugadal za %d tries!\n", tries);
            write(pipe_write, &tries, sizeof(int));
            round_end = 1;
	}
	else{
	    mid = rand() % (N + 1);
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int n = atoi(argv[1]);
    int pipefd[2]; //descryptor
    pid_t pid;


    if (pipe(pipefd) == -1) {
        perror("Kanal error");
        return 1;
    }

    pid = fork();
    if (pid < 0) {
        perror("Process error");
        return 1;
    }
    else if (pid == 0) { // Дочерний процесс
        close(pipefd[1]); //end writing
        guess_the_number(n, pipefd[0], pipefd[1]);
        close(pipefd[0]);
    }
    else { // Родительский процесс
        close(pipefd[0]); //end reading
        srand(time(NULL));
        int secret_number = rand() % n + 1;
        int tries;
	int end_signal = -1;

        printf("Perviy igrok zagadyvaet: %d\n", secret_number);
        write(pipefd[1], &secret_number, sizeof(int)); //x --> dochern
        read(pipefd[1], &tries, sizeof(int)); //tries --> dochern
        write(pipefd[1], &secret_number, sizeof(int));
	write(pipefd[1], &end_signal, sizeof(int)); // end of game
        close(pipefd[1]);
	wait(NULL);
    }


    pid_t pid2;
    int pipefd2[2];
    if (pipe(pipefd2) == -1) {
        perror("Kanal error");
        return 1;
    }
    pid2 = fork();
    if (pid2 < 0) {
        perror("Process error");
        return 1;
    }
    else if (pid2 == 0) { // Дочерний процесс
        close(pipefd2[1]); //end writing
        guess_the_number(n, pipefd2[0], pipefd2[1]);
        close(pipefd2[0]);
	exit(0);
    }
    else { // Родительский процесс
        close(pipefd2[0]); //end reading
        int secret_number2 = rand() % n + 1;
        int tries;
        int end_signal = -1;
        printf("Vtoroy igrok zagadyvaet: %d\n", secret_number2);
        write(pipefd2[1], &secret_number2, sizeof(int)); //x --> dochern
        read(pipefd2[1], &tries, sizeof(int)); //tries --> dochern
        write(pipefd2[1], &secret_number2, sizeof(int));
        write(pipefd2[1], &end_signal, sizeof(int)); // end of game
        close(pipefd2[1]);
	wait(NULL);
    }

    return 0;
}
