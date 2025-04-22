/*
 * Copyright (C) 2014-2017 Daniel Rossier <daniel.rossier@heig-vd.ch>
 * Copyright (C) 2017-2018 Xavier Ruppen <xavier.ruppen@heig-vd.ch>
 * Copyright (C) 2017 Alexandre Malki <alexandre.malki@heig-vd.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include <stdlib.h>
#include <signal.h>

#define TOKEN_NR	10
#define ARGS_MAX	16

char tokens[TOKEN_NR][80];
char prompt[] = "so3% ";

void parse_token(char *str) {
	int i = 0;
	char *next_token;

	next_token = strtok(str, " ");
	if (!next_token)
		return;

	strcpy(tokens[i++], next_token);

	while ((next_token = strtok(NULL, " ")) != NULL)
		strcpy(tokens[i++], next_token);
}

/*
 * Process the command with the different tokens
 */
void process_cmd(void) {
    int i, pid, sig, pid_child, arg_pos, pipefd[2], pid_child2;
    char *argv[ARGS_MAX], *argv2[ARGS_MAX];
    char filename[30];
    int is_pipe, arg2_pos;

	/* PRIVATE to SYE team */

	if (!strcmp(tokens[0], "dumpsched")) {
		sys_info(1, 0);
		return ;
	}

	if (!strcmp(tokens[0], "dumpproc")) {
		sys_info(4, 0);
		return ;
	}

	if (!strcmp(tokens[0], "exit")) {
		if (getpid() == 1) {
			printf("The shell root process can not be terminated...\n");
			return ;
		} else
			exit(0);

		/* If the shell is the root shell, there is a failure on exit() */
		return ;
	}

	/* end of PRIVATE */

	/* setenv */
	if (!strcmp(tokens[0], "setenv")) {
		/* second arg present ? */
		if (tokens[1][0] != 0) {
			/* third arg present ? */
			if (tokens[2][0] != 0) {
				/* Set the env. var. (always overwrite) */
				setenv(tokens[1], tokens[2], 1);
			} else
				unsetenv(tokens[1]);
		}
		return ;
	}

	/* env */
	if (!strcmp(tokens[0], "env")) {
		/* This function print the environment vars */
		for (i = 0; __environ[i] != NULL; i++)
			printf("%s\n", __environ[i]);

		return ;
	}

	/* kill */
	if (!strcmp(tokens[0], "kill")) {
		/* Send a signal to a process */
		sig = 0;

		if (tokens[2][0] == 0) {
			sig = SIGTERM;
			pid = atoi(tokens[1]);
		} else {
			if (!strcmp(tokens[1], "-USR1")) {
				sig = SIGUSR1;
				pid = atoi(tokens[2]);
			} else if (!strcmp(tokens[1], "-9")) {
				sig = SIGKILL;
				pid = atoi(tokens[2]);
			}
		}

		kill(pid, sig);

		return ;
	}
    // Analyse des tokens pour détecter la présence d'un pipe et séparer les commandes
    arg_pos = 0;
	is_pipe = 0;
	arg2_pos = 0;
    while (tokens[arg_pos][0] != 0) {
        // Détecter le symbole "|"
        if (!is_pipe && strcmp(tokens[arg_pos], "|") == 0) {
            is_pipe = 1;
            argv[arg_pos] = NULL; // Marquer la fin de la première commande
        } else {
            // Répartir les tokens entre les deux commandes
            if (is_pipe) {
                argv2[arg2_pos] = tokens[arg_pos]; // Commande après le pipe
                ++arg2_pos;
            } else {
                argv[arg_pos] = tokens[arg_pos]; // Commande avant le pipe
            }
        }
        ++arg_pos;
    }
    argv[arg_pos] = NULL; 
    argv2[arg2_pos] = NULL; 


    if (is_pipe) {
        // Création d'un pipe
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return;
        }

        // Premier fork pour la première commande
        pid_child = fork();
        if (pid_child == 0) { // Exécution dans l'enfant 1
            close(pipefd[0]); // Fermer l'extrémité de lecture inutilisée
            dup2(pipefd[1], STDOUT_FILENO); // Rediriger stdout vers le pipe
            close(pipefd[1]); // Fermer l'extrémité d'écriture après la redirection

            // Exécution de la première commande
            strcpy(filename, argv[0]);
            strcat(filename, ".elf");
            execv(filename, argv);
            perror("execv");
            exit(EXIT_FAILURE);
        }

        // Deuxième fork pour la deuxième commande
        pid_child2 = fork();
        if (pid_child2 == 0) { // Exécution dans l'enfant 2
            close(pipefd[1]); // Fermer l'extrémité d'écriture inutilisée
            dup2(pipefd[0], STDIN_FILENO); // Rediriger stdin depuis le pipe
            close(pipefd[0]); // Fermer l'extrémité de lecture après la redirection

            // Exécution de la deuxième commande
            strcpy(filename, argv2[0]);
            strcat(filename, ".elf");
            execv(filename, argv2);
            perror("execv");
            exit(EXIT_FAILURE);
        }

        // Processus parent : attendre la fin des deux processus enfants
        close(pipefd[0]); // Fermer les extrémités du pipe dans le parent
        close(pipefd[1]);
        waitpid(pid_child, NULL, 0);
        waitpid(pid_child2, NULL, 0);
    } else {
        // Gestion des commandes sans pipe
        pid_child = fork();
        if (pid_child == 0) { // Exécution dans l'enfant
            strcpy(filename, tokens[0]);
            strcat(filename, ".elf");

            // Exécution de la commande
            if (execv(filename, argv) == -1) {
                printf("%s: exec failed.\n", argv[0]);
                exit(-1);
            }
        } else { // Exécution dans le parent
            // Attendre la fin de l'exécution de l'enfant
            waitpid(pid_child, NULL, 0);
        }
    }
}

/*
 * Ignore the SIGINT signal, but we re-display the prompt to be elegant ;-)
 */
void sigint_sh_handler(int sig) {

	printf("%s", prompt);
	fflush(stdout);
}
/*
 * Main entry point of the shell application.
 */
void main(int argc, char *argv[])
{
	char user_input[80];
	int i;
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));

	sa.sa_handler = sigint_sh_handler;
	sigaction(SIGINT, &sa, NULL);

	while (1) {
		/* Reset all tokens */
		for (i = 0; i < TOKEN_NR; i++)
			tokens[i][0] = 0;

		printf("%s", prompt);
		fflush(stdout);

		gets(user_input);

		if (strcmp(user_input, ""))
			parse_token(user_input);

		/* Check if there is at least one token to be processed */
		if (tokens[0][0] != 0)
			process_cmd();


	}
}
