#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "cpu_core.h"

#define IMMEDIATE_PATTERN "^[0-9]+$"
#define REGISTER_PATTERN "^[ABCD]X$"
#define MEMORY_DIRECT_PATTERN "^\\[([0-9])+\\]$"
#define REGISTER_INDIRECT_PATTERN "^\\[(AX|BX|CX|DX)\\]$"
#define SEGMENT_OVERRIDE_PATTERN "^\\[[A-Z]S:[A-Z]X\\]$"

//Exercice 5 :
int matches(const char* pattern, const char* string) {
    regex_t regex ;
    int result = regcomp (&regex , pattern , REG_EXTENDED );
    if (result) {
        fprintf (stderr, "Regex compilation failed for pattern : %s\n", pattern);
        return 0;
    }
    result = regexec (&regex , string , 0 , NULL , 0);
    regfree (&regex);
    return result == 0;
}

void *immediate_addressing(CPU *cpu, const char *operand) {
    // Vérifier si l'opérande est un nombre immédiat
    int res = matches(IMMEDIATE_PATTERN, operand);
    int *value = (int *)malloc(sizeof(int));
    if (!res || sscanf(operand, "%d", value) != 1) {
        fprintf(stderr, "Invalid immediate value: %s\n", operand);
        free(value);
        return NULL;
    }
    void *data = hashmap_get(cpu->constant_pool, operand);
    if (data == NULL) {
        // Si la valeur n'est pas déjà dans le constant_pool, l'ajouter
        hashmap_insert(cpu->constant_pool, operand, value);
    } else {
        free(value);
        value = (int *)data;
    }
    return value;
}

void *register_addressing(CPU *cpu, const char *operand) {
    // Vérifier si l'opérande est un registre valide
    if (matches(REGISTER_PATTERN, operand)) {
        return hashmap_get(cpu->context, operand);
    }
    fprintf(stderr, "Invalid register: %s\n", operand);
    return NULL;
}

void *memory_direct_addressing(CPU *cpu, const char *operand) {
    // Vérifier si l'opérande est un segment valide
    if (matches(MEMORY_DIRECT_PATTERN, operand)) {
        char pos[256];
        // extraire la position entre crochets
        sscanf(operand, "[%s", pos);
        pos[strcspn(pos, "]")] = '\0';
        return load(cpu->memory_handler, "DS", atoi(pos)); //retourner la valeur stockèe dans le segment de données
    }
    fprintf(stderr, "Invalid memory segment: %s\n", operand);
    return NULL;
}

void *register_indirect_addressing(CPU *cpu, const char *operand) {
    // Vérifier si l'opérande est un registre valide
    if (! matches(REGISTER_INDIRECT_PATTERN, operand)) {
        fprintf(stderr, "Invalid register: %s\n", operand);
        return NULL;
    }
    char reg[256];
    // extraire le registre entre crochets
    sscanf(operand, "[%s", reg);
    reg[strcspn(reg, "]")] = '\0';
    void *data = register_addressing(cpu, reg);
    if (data == NULL) {
        fprintf(stderr, "Invalid register indirect addressing: %s\n", operand);
        return NULL;
    }
    return load(cpu->memory_handler, "DS", *(int *)data); //retourner la valeur stockèe dans le segment de données
}

void handle_MOV(CPU* cpu, void* src, void* dest) {
    // Vérifier si les deux opérandes sont valides
    if (src == NULL || dest == NULL) {
        fprintf(stderr, "Invalid MOV operation: src or dest is NULL\n");
        return;
    }
    // Effectuer l'opération MOV
    *(int *)dest = *(int *)src;
}

CPU* setup_test_environment () {
    // Initialiser le CPU
    CPU* cpu = cpu_init(1024);
    if (! cpu ) {
        printf("Error: CPU initialization failed\n");
        return NULL;
    }
    // Initialiser les registres avec des valeurs specifiques
    int* ax = (int *)hashmap_get(cpu-> context, "AX");
    int* bx = (int *)hashmap_get(cpu-> context, "BX");
    int* cx = (int *)hashmap_get(cpu-> context, "CX");
    int* dx = (int *)hashmap_get(cpu-> context, "DX");
    *ax = 3;
    *bx = 6;
    *cx = 100;
    *dx = 0;
    // Creer et initialiser le segment de donnees
    if (! hashmap_get(cpu-> memory_handler->allocated , "DS") ) {
        if (! create_segment(cpu-> memory_handler , "DS", 129, 20)) {
            fprintf(stderr, "Error creating data segment\n");
            return NULL;
        }
        // Initialiser le segment de donnes avec des valeurs de test
        for (int i = 0; i < 10; i ++) {
            int * value = (int *)malloc(sizeof(int));
            * value = i * 10 + 5; // Valeurs 5, 15, 25, 35...
            store(cpu-> memory_handler , "DS", i , value ) ;
        }
    }
    printf("Test environment initialized.\n") ;
    return cpu;
}

void *resolve_addressing(CPU *cpu, const char *operand) {
    void *value = NULL;
    if ( (value = immediate_addressing(cpu, operand)) )
        return value;
    if ( (value = register_addressing(cpu, operand)) )
        return value;
    if ( (value = memory_direct_addressing(cpu, operand)) )
        return value;
    if ( (value = register_indirect_addressing(cpu, operand)) )
        return value;
    if ( (value = segment_override_addressing(cpu, operand)) )
        return value;
    fprintf(stderr, "No matching addressing mode: %s\n", operand);
    return NULL;
}


//Exercice 6 :
char *trim(char *str) {
    while (*str == '`' || *str == '\t' || *str == '\n' || *str == '\r') str++;

    char *end = str + strlen(str) - 1;
    while (end > str && (*end == '`' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    return str ;
}

int search_and_replace (char **str, HashMap *values) {
    if (!str || !*str || !values ) return 0;

    int replaced = 0;
    char * input = * str ;

    // Iterate through all keys in the hashmap
    for (int i = 0; i < values->size; i ++) {
        if (values->table[i].key && values->table[i].key != (void *)-1) {
            char *key = values->table[i].key;
            //j'ai changé le cast de (int) (long) à *(int *) pour bien recuperer la valeur
            int value = *(int *)values->table[i].value;

            // Find potential substring match
            char *substr = strstr(input, key);
            if (substr) {
                // Construct replacement buffer
                char replacement[64];
                snprintf(replacement, sizeof(replacement), "%d", value);

                // Calculate lengths
                int key_len = strlen(key);
                int repl_len = strlen(replacement);
                int remain_len = strlen ( substr + key_len ) ;

                // Create new string
                char *new_str = (char *)malloc(strlen(input) - key_len + repl_len + 1);
                strncpy(new_str, input, substr - input);
                new_str[substr - input] = '\0';
                strcat(new_str, replacement);
                strcat(new_str, substr + key_len);

                // Free and update original string
                free(input);
                *str = new_str;
                input = new_str;

                replaced = 1;
            }
        }
    }

    // Trim the final string
    if (replaced) {
        char *trimmed = trim(input);
        if (trimmed != input) {
            memmove(input, trimmed, strlen(trimmed) + 1);
        }
    }

    return replaced ;
}

int resolve_constants(ParserResult *result) {
    if (!result) return 0;

    // Iterer toutes les cases de tableau code instructions
    for (int i = 0; i < result->code_count; i++) {
        Instruction *instr = result->code_instructions[i];
        if (strcmp(instr->operand1, "") != 0) {
            // Sinon, on remplace operand1 par une adresse
            if (! search_and_replace(&instr->operand1, result->memory_locations) && ! search_and_replace(&instr->operand1, result->labels)) {
                fprintf(stderr, "operand1: %s replacement fault\n", instr->operand1);
            }
        }
        if (strcmp(instr->operand2, "") != 0) {
            // Si l'opérande 2 existe, on le remplace par une addresse
            if (! search_and_replace(&instr->operand2, result->memory_locations) && ! search_and_replace(&instr->operand2, result->labels)) {
                fprintf(stderr, "operand2: %s replacement fault\n", instr->operand2);
            }
        }
    }

    return 1;
}

int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest) {
    if (strcmp(instr->mnemonic, "MOV") == 0) {
        handle_MOV(cpu, src, dest);

    } else if (strcmp(instr->mnemonic, "ADD") == 0) {
        if (src == NULL || dest == NULL) {
            fprintf(stderr, "Invalid ADD operation: src or dest is NULL\n");
            return 0;
        }
        *(int *)dest += *(int *)src;

    } else if (strcmp(instr->mnemonic, "CMP") == 0) {
        if (src == NULL || dest == NULL) {
            fprintf(stderr, "Invalid CMP operation: src or dest is NULL\n");
            return 0;
        }
        // Mettre à jour les indicateurs ZF et SF
        int *zf = (int *)hashmap_get(cpu->context, "ZF");
        int *sf = (int *)hashmap_get(cpu->context, "SF");
        *zf = (*(int *)dest == *(int *)src);
        *sf = (*(int *)dest < *(int *)src);

    } else if (strcmp(instr->mnemonic, "JMP") == 0) {
        if (dest == NULL) {
            fprintf(stderr, "Invalid JMP operation: dest is NULL\n");
            return 0;
        }
        void *ip = hashmap_get(cpu->context, "IP");
        handle_MOV(cpu, dest, ip);

    } else if (strcmp(instr->mnemonic, "JZ") == 0) {
        if (dest == NULL) {
            fprintf(stderr, "Invalid JZ operation: dest is NULL\n");
            return 0;
        }
        int *zf = (int *)hashmap_get(cpu->context, "ZF");
        if (*zf == 1) {
            void *ip = hashmap_get(cpu->context, "IP");
            handle_MOV(cpu, dest, ip);
        }

    } else if (strcmp(instr->mnemonic, "JNZ") == 0) {
        if (dest == NULL) {
            fprintf(stderr, "Invalid JNZ operation: dest is NULL\n");
            return 0;
        }
        int *zf = (int *)hashmap_get(cpu->context, "ZF");
        if (*zf == 0) {
            void *ip = hashmap_get(cpu->context, "IP");
            handle_MOV(cpu, dest, ip);
        }

    } else if (strcmp(instr->mnemonic, "PUSH") == 0) {
        int *reg;
        if (dest == NULL)
            reg = (int *)hashmap_get(cpu->context, "AX");
        else 
            reg = (int *)hashmap_get(cpu->context, instr->operand1);
        if (push_value(cpu, *reg) == -1) {
            fprintf(stderr, "Error pushing value %d onto stack\n", *(int *)reg);
            return 0;
        }

    } else if (strcmp(instr->mnemonic, "POP") == 0) {
        int *reg;
        if (dest == NULL)
            reg = (int *)hashmap_get(cpu->context, "AX");
        else 
            reg = (int *)hashmap_get(cpu->context, instr->operand1);
        if (pop_value(cpu, reg) == -1) {
            fprintf(stderr, "Error popping value %d from stack\n", *(int *)reg);
            return 0;
        }

    } else if (strcmp(instr->mnemonic, "HALT") == 0) {
        Segment *seg = hashmap_get(cpu->memory_handler->allocated, "CS");
        void *ip = hashmap_get(cpu->context, "IP");
        handle_MOV(cpu, (void *) &seg->size, ip);

    } else if (strcmp(instr->mnemonic, "ALLOC") == 0) {
        if (!alloc_es_segment(cpu))
            return 0;

    } else if (strcmp(instr->mnemonic, "FREE") == 0) {
        if (!free_es_segment(cpu))
            return 0;

    } else {
        fprintf(stderr, "Unknown instruction: %s\n", instr->mnemonic);
        return 0;
    }
    return 1;
}

int execute_instruction(CPU *cpu, Instruction *instr) {
    if (instr == NULL) return 0;
    // Résoudre les opérandes
    void *dest = resolve_addressing(cpu, instr->operand1);
    void *src = resolve_addressing(cpu, instr->operand2);

    // Traiter l'instruction
    return handle_instruction(cpu, instr, src, dest);
}

Instruction* fetch_next_instruction(CPU *cpu) {
    int *ip = (int *)hashmap_get(cpu->context, "IP");
    Segment *seg = hashmap_get(cpu->memory_handler->allocated, "CS");
    if (!seg || *ip < 0 || *ip >= seg->size) {
        fprintf(stderr, "Invalid instruction pointer or end of program\n");
        return NULL;
    }
    Instruction *instr = (Instruction *)load(cpu->memory_handler, "CS", *ip);
    (*ip)++; // Incrémenter le pointeur d'instruction
    return instr;
}

int run_program(CPU *cpu) {
    if (!cpu) return 0;

    int nReg = 10;
    char *registres[] = {"AX", "BX", "CX", "DX", "IP", "ZF", "SF", "SP", "BP", "ES"};
    char input;
    Instruction *prev = NULL, *curr = NULL, *next = NULL;
    // Afficher l'état initial du CPU
    printf("Initial CPU state:\n");

    // Afficher l'etat courant du CPU
    do {
        print_data_segment(cpu);
        printf("\n--- REGISTERS ---\n");
        for (int i = 0; i < nReg; i++) {
            int *val = (int *)hashmap_get(cpu->context, registres[i]);
            if (val) {
                printf("%s: %d\n", registres[i], *val);
            } else {
                printf("%s: NULL\n", registres[i]);
            }
        }
        printf("\n--- EXECUTION CONTEXT ---\n");
        if (prev) printf("Previous: %s %s %s\n", prev->mnemonic, prev->operand1, prev->operand2);
        else printf("Previous: No previous instruction\n");

        if (curr) printf("Current: %s %s %s\n", curr->mnemonic, curr->operand1, curr->operand2);
        else printf("Current: Beginning of program\n");

        next = fetch_next_instruction(cpu);
        if (next) printf("Next: %s %s %s\n", next->mnemonic, next->operand1, next->operand2);
        else printf("Next: End of program\n");

        printf("\n--- INTERACTIVE PROMPT ---\n");
        printf("Press ENTER to execute next instruction\nPress 'q' to quit execution\n");
        input = getchar();
        if (input == 'q' || input == 'Q')
            break;
        // Afficher l'instruction courante
        prev = curr;
        curr = next;
        execute_instruction(cpu, curr);
    } while (curr != NULL);

    // Afficher l'état final du CPU
    printf("Final CPU state:\n");
    print_data_segment(cpu);
    printf("\n--- REGISTERS ---\n");
    for (int i = 0; i < nReg; i++) {
        int *val = (int *)hashmap_get(cpu->context, registres[i]);
        if (val) {
            printf("%s: %d\n", registres[i], *val);
        } else {
            printf("%s: NULL\n", registres[i]);
        }
    }
    return 1;
}


//Exercice 7 :
int push_value(CPU *cpu, int value) {
    Segment *seg = hashmap_get(cpu->memory_handler->allocated, "SS");
    if (!seg) {
        fprintf(stderr, "Stack segment not found\n");
        return -1;
    }

    // Vérifier si le pointeur de pile est valide
    int *sp = (int *)hashmap_get(cpu->context, "SP");
    if (*sp < 0 || *sp >= seg->size) {
        fprintf(stderr, "Stack pointer out of bounds\n");
        return -1;
    }
    // Stocker value dans le segment de pile
    int *val = (int *)malloc(sizeof(int));
    *val = value;
    if (!store(cpu->memory_handler, "SS", *sp, (void *)val)) {
        free(val);
        return -1;
    }
    (*sp)--;
    return 1;
}

int pop_value(CPU *cpu, int *dest) {
    Segment *seg = hashmap_get(cpu->memory_handler->allocated, "SS");
    if (!seg) {
        fprintf(stderr, "Stack segment not found\n");
        return -1;
    }

    // Vérifier si le pointeur de pile est valide
    int *sp = (int *)hashmap_get(cpu->context, "SP");
    if ((*sp)+1 < 0 || (*sp)+1 >= seg->size) {
        fprintf(stderr, "Stack pointer out of bounds\n");
        return -1;
    }
    (*sp)++;
    // Charger la valeur du sommet de SS
    void *val = load(cpu->memory_handler, "SS", *sp);
    if (!val) {
        return -1;
    }
    *dest = *(int *)val;
    free(val);
    return 1;
}

//Exercice 8 :
void *segment_override_addressing(CPU* cpu, const char* operand) {
    // Vérifier si l'opérande est un segment valide
    if (matches(SEGMENT_OVERRIDE_PATTERN, operand)) {
        char segment[64], regist[64];
        // extraire la position entre crochets
        sscanf(operand, "[%s:%s]", segment, regist);
        segment[2] = '\0';
        regist[2] = '\0';
        int *pos = (int *)hashmap_get(cpu->context, regist);
        return load(cpu->memory_handler, segment, *pos); //retourner la valeur stockèe dans le segment de données
    }
    fprintf(stderr, "Invalid memory segment: %s\n", operand);
    return NULL;
}

int find_free_address_strategy(MemoryHandler *handler, int size, int strategy) {
    Segment *seg = handler->free_list;
    int start = -1;
    switch (strategy) {
    case 0: // First Fit
        while (seg != NULL) {
            if (seg->size >= size) {
                    start = seg->start;
                    break;
                }
            seg = seg->next;
        }
        break;
    
    case 1: // Best Fit
        int best = -1;
        while (seg != NULL) {
            if (seg->size >= size) {
                if (best == -1 || seg->size < best) {
                    best = seg->size;
                    start = seg->start;
                }
            }
            seg = seg->next;
        }
        break;

    case 2 : // Worst Fit
        int worst = -1;
        while (seg != NULL) {
            if (seg->size >= size) {
                if (seg->size > worst) {
                    worst = seg->size;
                    start = seg->start;
                }
            }
            seg = seg->next;
        }
        break;
    
    default:
        break;
    }
    
    return start; // No free address found
}

int alloc_es_segment(CPU *cpu) {
    //recupérer la taille et la stratégie de l'ES
    int *taille = (int *)hashmap_get(cpu->context, "AX");
    int *strategy = (int *)hashmap_get(cpu->context, "BX");
    int *zf = (int *)hashmap_get(cpu->context, "ZF");

    // recherche de bonne adresse et allocation de segment
    int start = find_free_address_strategy(cpu->memory_handler, *taille, *strategy);
    *zf = !create_segment(cpu->memory_handler, "ES", start, *taille);
    if (*zf == 1) {
        fprintf(stderr, "Error creating ES segment\n");
        return 0;
    }

    // Initialiser le segment ES avec des zéros
    for (int i = 0; i < *taille; i++) {
        int *val = (int *)malloc(sizeof(int));
        *val = 0;
        if (!store(cpu->memory_handler, "ES", i, val)) {
            free(val);
            fprintf(stderr, "Error storing value in ES segment at index : %d \n", i);
            return 0;
        }
    }

    // Mettre à jour le registre ES par l'adresse de début de segment ES
    int *es = (int *)hashmap_get(cpu->context, "ES");
    if (es == NULL) {
        fprintf(stderr, "Error retrieving ES segment\n");
        return 0;
    }
    *es = start;
    return 1;
}

int free_es_segment(CPU *cpu) {
    // Recuperer le segment ES
    Segment *seg = hashmap_get(cpu->memory_handler->allocated, "ES");
    if (seg == NULL) {
        fprintf(stderr, "Error retrieving ES segment\n");
        return 0;
    }
    // Libérer la mémoire allouée pour chaque case du segment et supprimer le segment de la mémoire
    for (int i = 0; i < seg->size; i++) {
        void *val = load(cpu->memory_handler, "ES", i);
        free(val);
        val = NULL;
    }

    // Supprimer le segment ES
    remove_segment(cpu->memory_handler, "ES");
    hashmap_remove(cpu->memory_handler->allocated, "ES");
    int *es = (int *)hashmap_get(cpu->context, "ES");
    *es = -1;

    return 1;
}