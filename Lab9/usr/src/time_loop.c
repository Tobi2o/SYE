#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#define DURATION    30  // seconds

int main(int argc, char **argv) {
    int previous;
    pid_t pid;
    size_t count, duration = DURATION;
    int block = 0;
    struct timeval tv;

    if (argc >= 2 && !strcmp(argv[1], "b")) block = 1;


    if (argc == 3) {
        duration = atoi(argv[2]);
    }

    count = 0;
    pid = getpid();

    for (count = 0; count < duration; ++count) {

        if(block) usleep(1000000);
        else
        {
            gettimeofday(&tv, NULL);
            previous = tv.tv_sec;
            do {
                gettimeofday(&tv, NULL);
            } while (previous == tv.tv_sec);
        }

        fprintf(stdout, "[%d] %zu\n", pid, count);
    }

    return 0;
}
