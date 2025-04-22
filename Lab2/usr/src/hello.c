#include <stdio.h>

int main() {

    char chaine[] ="Hello world !";
    printf("chaine de base :%s\n\n", chaine);

    printf("adresse de la chaine: %p\n\n", chaine); 
    /* 0x7fffffffd52a est l'adresse trouvee à la 1st execution */

    int i = 0;
    while (*((char *)(0x7fffffffd52a + i)) != '\0') {
        (*((char *)(0x7fffffffd52a + i)))++;
        i++;
    }

    printf("Chaine modifiée :%s\n\n", (char*)0x7fffffffd52a);

    return 0;
};

