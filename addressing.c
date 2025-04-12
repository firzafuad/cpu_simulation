#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "addressing.h"

int matches ( const char * pattern , const char * string ) {
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

CPU* setup_test_environment () {
    // Initialiser le CPU
    CPU* cpu = cpu_init(1024);
    if (! cpu ) {
        printf("Error: CPU initialization failed\n");
        return NULL ;
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