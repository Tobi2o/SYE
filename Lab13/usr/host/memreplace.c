#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>

#define RESET        	2
#define DELTA        	2
#define EXIT_MEMRPLC 	(-2)

#define PAGE_NB_MASK    0x00F0
#define REF_BIT_MASK    0x0004
#define SWAP_BIT_MASK   0x0002
#define VALID_BIT_MASK  0x0001

#define NB_PAGES    	16

/* [7..4] = Page number on 4 bits */
/* [3] = Unused 1 bit */
/* [2] = Reference on 1 bit */
/* [1] = SWAP on 1 bit */
/* [0] = Valid on 1 bit */
uint8_t page_table[NB_PAGES] = {0};
int TDU_table[NB_PAGES] = {0};
int TVC = 0;


void replaceLRU(void) {
    // Recherche de la page à remplacer selon l'algorithme LRU
    int lru_page = 0;
    int min_tdu = INT_MAX;

    for (int i = 0; i < NB_PAGES; ++i) {
        if ((page_table[i] & VALID_BIT_MASK) && TDU_table[i] < min_tdu) {
            min_tdu = TDU_table[i];
            lru_page = i;
        }
    }

    // Mise à jour des bits valid et swap de la page LRU
    page_table[lru_page] |= SWAP_BIT_MASK;
    page_table[lru_page] &= ~VALID_BIT_MASK;
}


void replaceWSC(void) {

    int indexPage = -1;
    
    for(int i = 0; i < NB_PAGES; ++i) {

        // Evite les pages invalides
		if (!(page_table[i] & VALID_BIT_MASK)) {
			continue;
        }

 
        if(indexPage == -1) {
            indexPage = i;
        }

        // Seconde chance
        if(page_table[i] & REF_BIT_MASK) {
            page_table[i] &= ~REF_BIT_MASK;
        } else {

            if(TVC - TDU_table[i] > DELTA) {
                indexPage = i;
                break;
            }
        }
    }


    if (indexPage == -1) {

        printf(" Aucune pag valide trouvee. \n");
        return;
    } else {

        page_table[indexPage] &= ~VALID_BIT_MASK;
        page_table[indexPage] |= SWAP_BIT_MASK;
    }
    
}

void updateTDU(void) {
    // Mise à jour des TDU pour les pages référencées
    for(int i = 0; i < NB_PAGES; ++i) {
        if(page_table[i] & REF_BIT_MASK & VALID_BIT_MASK) {
            TDU_table[i] = TVC;
        }
    }
}


void init_page_table(void) {
    int i;

    for (i = 0; i < NB_PAGES; i++)
        page_table[i] |= (i & 0xF) << 4;

    page_table[0] |= VALID_BIT_MASK;
	page_table[1] |= VALID_BIT_MASK;
	page_table[2] |= VALID_BIT_MASK;
	page_table[3] |= SWAP_BIT_MASK;
}

void print_memory(void) {
    int i;

    printf("RAM : ");

    for (i = 0; i < NB_PAGES; i++)
        if (page_table[i] & VALID_BIT_MASK)
            printf("[%d] ", i);

    printf("\nSWAP : ");
    for (i = 0; i < NB_PAGES; i++)
        if (!(page_table[i] & VALID_BIT_MASK) && (page_table[i] & SWAP_BIT_MASK))
            printf("[%d] ", i);

    printf("\n");
}

int ask_user_page_no() {
    char input[4];

    printf("Enter the page to be access: ");
    fflush(stdout);
    fgets(input, sizeof(input), stdin);

    if (input[0] == 'E' || input[0] == 'e')
        return EXIT_MEMRPLC;

    if (!isdigit(input[0]))
        return -1;

    return atoi(input);
}

int main(int argc, char *argv[]) {
    int page_no;
    bool is_valid = true;
    int LRU_run = 0;

    if (argc != 2) {
        is_valid = 0;
    }
    else if (strcmp("LRU", argv[1]) == 0) {
        LRU_run = 1;
    }
    else if (strcmp("WSC", argv[1]) == 0) {
        LRU_run = 0;
    }
    else {
        is_valid = 0;
    }

    if (!is_valid) {
        printf("Usage: memreplace LRU|WSC\n");
        exit(1);
    }

    init_page_table();
    print_memory();

    for (;;) {
        page_no = ask_user_page_no();

        if (page_no == EXIT_MEMRPLC) {
            return 0;
        }
        else {
            // Modification ici : Ajout de l'incrémentation de TVC
            TVC++;
            if (page_no >= 0 && page_no < NB_PAGES) {
                if (LRU_run) {
                    if (!(page_table[page_no] & VALID_BIT_MASK)) 
                        replaceLRU();

                    // Mise à jour du TDU de la page accédée
                    TDU_table[page_no] = TVC;

                    // Mise à jour des bits de la page accédée
                    page_table[page_no] |= VALID_BIT_MASK;
                    page_table[page_no] &= ~SWAP_BIT_MASK;
                }
                else {
                    if (!(page_table[page_no] & VALID_BIT_MASK)) {
                        replaceWSC();
                        page_table[page_no] &= ~SWAP_BIT_MASK;
                    }

                    // Ajout de la mise à jour des bits de référence et de validité
                    page_table[page_no] |= REF_BIT_MASK;
                    page_table[page_no] |= VALID_BIT_MASK;
                    updateTDU();
                }

                print_memory();
            }
            else
                printf("Invalid page number\n");
        }
    }

    return 0;
}
