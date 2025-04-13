#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "addressing.h"

#define IMMEDIATE_PATTERN "^[0-9]+$"
#define REGISTER_PATTERN "^[ABCD]X$"
#define MEMORY_DIRECT_PATTERN "^\\[([0-9])+\\]$"
#define REGISTER_INDIRECT_PATTERN "^\\[(AX|BX|CX|DX)\\]$"

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
        printf("Invalid immediate value: %s\n", operand);
        free(value);
        return NULL;
    }
    void *data = hashmap_get(cpu-> constant_pool, operand);
    if (data == NULL) {
        // Si la valeur n'est pas déjà dans le constant_pool, l'ajouter
        hashmap_insert(cpu-> constant_pool, operand, value);
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
    printf("Invalid register: %s\n", operand);
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
    printf("Invalid memory segment: %s\n", operand);
    return NULL;
}

void *register_indirect_addressing(CPU *cpu, const char *operand) {
    // Vérifier si l'opérande est un registre valide
    if (! matches(REGISTER_INDIRECT_PATTERN, operand)) {
        printf("Invalid register: %s\n", operand);
        return NULL;
    }
    char reg[256];
    // extraire le registre entre crochets
    sscanf(operand, "[%s", reg);
    reg[strcspn(reg, "]")] = '\0';
    void *data = hashmap_get(cpu-> context, reg);
    if (data == NULL) {
        printf("Invalid register indirect addressing: %s\n", reg);
        return NULL;
    }
    return load(cpu->memory_handler, "DS", *(int *)data); //retourner la valeur stockèe dans le segment de données
}

void handle_MOV(CPU* cpu, void* src, void* dest) {
    // Vérifier si les deux opérandes sont valides
    if (src == NULL || dest == NULL) {
        printf("Invalid MOV operation: src or dest is NULL\n");
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
    if (value = immediate_addressing(cpu, operand))
        return value;
    if (value = register_addressing(cpu, operand))
        return value;
    if (value = memory_direct_addressing(cpu, operand))
        return value;
    if (value = register_indirect_addressing(cpu, operand))
        return value;
    printf("No matching addressing mode: %s\n", operand);
    return NULL;
}