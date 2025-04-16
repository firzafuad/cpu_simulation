#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "cpu_core.h"

#define IMMEDIATE_PATTERN "^[0-9]+$"
#define REGISTER_PATTERN "^[ABCD]X$"
#define MEMORY_DIRECT_PATTERN "^\\[([0-9])+\\]$"
#define REGISTER_INDIRECT_PATTERN "^\\[(AX|BX|CX|DX)\\]$"

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
        return hashmap_get(cpu-> context, operand);
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
    void *data = hashmap_get(cpu->context, reg);
    if (data == NULL) {
        fprintf(stderr, "Invalid register indirect addressing: %s\n", reg);
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
        create_segment(cpu-> memory_handler , "DS", 0 , 20) ;
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
                snprintf(replacement, sizeof(replacement), "[%d]", value);

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
        if (strcmp(instr->operand2, "") != 0) {
            if (! search_and_replace(&instr->operand2, result->memory_locations)) {
                fprintf(stderr, "error: replacement fault at index %d\n", i);
            }
        } else {
            if (! search_and_replace(&instr->operand1, result->labels)) {
                fprintf(stderr, "error: replacement fault at index %d\n", i);
            }
            char addr[64];
            sscanf(instr->operand1, "[%s", addr);
            addr[strcspn(addr, "]")] = '\0';
            free(instr->operand1);
            instr->operand1 = strdup(addr);
        }
    }

    return 1;
}

int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest) {
    if (strcmp(instr->mnemonic, "MOV") == 0) {
        handle_MOV(cpu, src, dest);

    } else if (strcmp(instr->mnemonic, "ADD") == 0) {
        if (src == NULL || dest == NULL) {
            printf("Invalid ADD operation: src or dest is NULL\n");
            return 0;
        }
        *(int *)dest += *(int *)src;

    } else if (strcmp(instr->mnemonic, "CMP") == 0) {
        if (src == NULL || dest == NULL) {
            printf("Invalid CMP operation: src or dest is NULL\n");
            return 0;
        }
        // Mettre à jour les indicateurs ZF et SF
        int *zf = (int *)hashmap_get(cpu->context, "ZF");
        int *sf = (int *)hashmap_get(cpu->context, "SF");
        *zf = (*(int *)dest == *(int *)src);
        *sf = (*(int *)dest < *(int *)src);

    } else if (strcmp(instr->mnemonic, "JMP") == 0) {
        void *ip = hashmap_get(cpu->context, "IP");
        handle_MOV(cpu, dest, ip);

    } else if (strcmp(instr->mnemonic, "JZ") == 0) {
        int *zf = (int *)hashmap_get(cpu->context, "ZF");
        if (*zf == 1) {
            void *ip = hashmap_get(cpu->context, "IP");
            handle_MOV(cpu, dest, ip);
        }

    } else if (strcmp(instr->mnemonic, "JNZ") == 0) {
        int *zf = (int *)hashmap_get(cpu->context, "ZF");
        if (*zf == 0) {
            void *ip = hashmap_get(cpu->context, "IP");
            handle_MOV(cpu, dest, ip);
        }

    } else if (strcmp(instr->mnemonic, "HALT") == 0) {
        Segment *seg = hashmap_get(cpu->memory_handler->allocated, "CS");
        handle_MOV(cpu, (void *) &seg->size, hashmap_get(cpu->context, "IP"));

    } else {
        fprintf(stderr, "Unknown instruction: %s\n", instr->mnemonic);
        return 0;
    }
    return 1;
}

int execute_instruction(CPU *cpu, Instruction *instr) {
    if (!instr) return 0;
    // Résoudre les opérandes
    void *dest = resolve_addressing(cpu, instr->operand1);
    void *src = resolve_addressing(cpu, instr->operand2);

    if (!src && !dest) {
        fprintf(stderr, "Invalid operands for instruction: %s\n", instr->mnemonic);
        return 0;
    }

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

    // Afficher l'état initial du CPU
    printf("Initial CPU state:\n");
    print_data_segment(cpu);
    char *registres[] = {"AX", "BX", "CX", "DX", "IP", "ZF", "SF"};
    for (int i = 0; i < 7; i++) {
        int *val = (int *)hashmap_get(cpu->context, registres[i]);
        if (val) {
            printf("%s: %d\n", registres[i], *val);
        } else {
            printf("%s: NULL\n", registres[i]);
        }
    }

    // Exécuter le programme
    char input;
    printf("Press enter to execute instruction or 'q' to quit ");
    Instruction *instr;
    do {
        input = getchar();
        if (input == 'q')
            break;
        // Afficher l'instruction courante
        instr = fetch_next_instruction(cpu);
        printf("Executing: %s %s %s\n", instr->mnemonic, instr->operand1, instr->operand2);
        if (!execute_instruction(cpu, instr)) {
            fprintf(stderr, "Error executing instruction %s\n", instr->mnemonic);
            return 0;
        }
    } while (instr != NULL);

    // Afficher l'état final du CPU
    printf("Final CPU state:\n");
    print_data_segment(cpu);
    for (int i = 0; i < 7; i++) {
        int *val = (int *)hashmap_get(cpu->context, registres[i]);
        if (val) {
            printf("%s: %d\n", registres[i], *val);
        } else {
            printf("%s: NULL\n", registres[i]);
        }
    }
    return 1;
}