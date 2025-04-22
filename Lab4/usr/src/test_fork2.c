/**
 * @file    test_fork2.c
 * @author  REDS Institute / HEIG-VD
 * @date    2021-09-23
 *
 * @brief   SYE 2021 - Lab02
 *
 * Test application for syscall fork2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
#if defined(syscallFork2)
    pid_t child_pid = sys_fork2();
    if (child_pid != 0) {
        pid_t pid = getpid();
        fprintf(stdout, "I, process with pid %d, gave birth to child with pid %d\n", pid, child_pid);
        waitpid(child_pid, NULL, 0);
    }
    return EXIT_SUCCESS;
#else
    fprintf(stderr, "syscall fork2 not found\n");
    return EXIT_FAILURE;
#endif
}
