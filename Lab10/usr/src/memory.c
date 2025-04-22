#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>


int main(int argc, char **argv) {
    uint8_t *buffer;

    // Allocation de 4 KiB de mémoire
    buffer = (uint8_t *) malloc(4096);
    if (buffer == NULL) {
        perror("Erreur d'allocation mémoire");
        return EXIT_FAILURE;
    }

    printf("Adresse virtuelle du buffer: %p\n", buffer);

    // Conversion de l'adresse virtuelle en adresse physique
    uint32_t physical_addr = sys_translate((uint32_t) buffer);
    

    if (physical_addr == 0) {
        printf("Erreur dans la traduction de l'adresse\n");
        free(buffer);
        return EXIT_FAILURE;
    }


    printf("Adresse physique correspondante: %x\n", physical_addr);


    free(buffer);

    return EXIT_SUCCESS;
}