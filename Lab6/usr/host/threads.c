#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syscall.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>


#define LETTERS_NB 26
#define A_LOWER_OFFSET 97

typedef struct  {
    /* Pointer to position in the text */
	char *text_pointer;
    /* Size of the text chuck to process */
	size_t size;
    /* Letters counters, used to send the results back */
	size_t counters[LETTERS_NB];
} count_param_t;

/**
 * @brief Threads function used to count the letters occurence in a text
 */
void *count_letters(void * args) {
	count_param_t *params = (count_param_t *) args;
	
	//TO COMPLETE
	for (size_t i = 0; i < params->size; ++i) {
		char letter = tolower(params->text_pointer[i]); // Convertir en lettre minuscule
		if (letter >= 'a' && letter <= 'z') {
			params->counters[letter - A_LOWER_OFFSET]++;
		}
	}

	return NULL;
}

/**
 * @brief Calculate the size of a file
 * 
 * @return The size of the file <fp>
 */
size_t fseek_filesize(FILE *fp) {
    long off;


    if (fseek(fp, 0, SEEK_END) == -1)
    {
        printf("failed to fseek\n");
        exit(EXIT_FAILURE);
    }

    off = ftell(fp);
    if (off == -1)
    {
        printf("failed to ftell\n");
        exit(EXIT_FAILURE);
    }

	fseek(fp, 0, SEEK_SET);

	return off;
}
/**
 * @brief Load a file content into a char buffer
 * 
 * @return A char *buffer with the content of the file <filename>
 */
char *buffer_from_file(char *filename) {

	char *buffer;
	size_t text_size = 0;
	FILE *f = fopen(filename, "r");

	if (f == NULL) {
        printf("failed to fopen %s\n", filename);
        exit(EXIT_FAILURE);
	}

	text_size = fseek_filesize(f);
	printf("text_size %zu\n", text_size);

	buffer = malloc(text_size+1);

	if (buffer == NULL) {
        printf("failed to malloc() the buffer\n");
        exit(EXIT_FAILURE);
	} 

	/* Read the whole content into buffer */
	fread(buffer, 1, text_size, f);

	fclose(f);

	/* Add the \0 at the end */
	buffer[text_size] = 0;

	return buffer;
}

/**
 * @brief Main function
 */
int main(int argc, char **argv)
{

	int thread_num;
	size_t result_counters[LETTERS_NB] = { 0 };

	char *filename;


	if(argc != 3)
	{
		printf("Error: arguments number should be 3\n");
		printf("Usage: threads <nb_threads> <filename>\n");
		return EXIT_FAILURE;
	}

	thread_num = atoi(argv[1]);
	filename = argv[2];

	if(thread_num > 0)
	{
		if(thread_num > 15) thread_num = 15;
 
	}
	else
	{
		printf("Error: The number of threads must be between 0 and 15");
		return EXIT_FAILURE;
	}
    // TO COMPLETE: Load the text and its size
	char *text = buffer_from_file(filename);
	size_t text_size = strlen(text);

    // TO COMPLETE: Configure and launch threads
	// Alloc dynamique pour le tableau de thread
	pthread_t *threads = malloc(thread_num * sizeof(pthread_t));
	if (threads == NULL) {
		printf("Error allocating memory for threads\n");
		free(text);
		return EXIT_FAILURE;
	}

	// Allocation dynamique pour le tableau de paramètres de chaque thread
	count_param_t *params = malloc(thread_num * sizeof(count_param_t));
	if (params == NULL) {
		printf("Error allocating memory for thread parameters\n");
		free(text); 
		free(threads);
		return EXIT_FAILURE;
	}

	// Calcul de la taille de chunk standard pour chaque thread
	size_t chunk_size = text_size / thread_num;
	size_t start = 0;

	for (int i = 0; i < thread_num; ++i) {
		params[i].text_pointer = &text[start];

	/*if (i == thread_num - 1) {
    	params[i].size = chunk_size + (text_size % thread_num);
	} else {  
    params[i].size = chunk_size;
	}*/
	// Correspond à
		params[i].size = (i == thread_num - 1) ? (chunk_size + text_size % thread_num) : chunk_size; // Ajouter le rest au dernier thread
		memset(params[i].counters, 0, sizeof(params[i].counters)); // Initialisation des compteurs à 0

		if (pthread_create(&threads[i], NULL, count_letters, &params[i])) {
			printf("Error creating thread\n");
			return 1;
		}

		start += chunk_size;
	}
    // TO COMPLETE: Join the threads and compile the results
	for (int i = 0; i < thread_num; ++i) {
		if (pthread_join(threads[i], NULL)) {
			printf("Error joining thread\n");
			return 2;
		}
		// Compiler les résultats
		for (int j = 0; j < LETTERS_NB; ++j) {
			result_counters[j] += params[i].counters[j];
		}
	}
    /* Display results */
	for (int i = 0; i < LETTERS_NB; ++i) {
		printf("Number of %c: %zu\n", (char)(i+A_LOWER_OFFSET), result_counters[i]);
	}

	// TO COMPLETE: Free allocated memory
	free(text);
	free(threads);
	free(params);
	return EXIT_SUCCESS;
}
