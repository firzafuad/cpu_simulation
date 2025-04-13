#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "addressing.h"

int main() {
    CPU *cpu = setup_test_environment();
    if (!cpu)
        return 1;
    
    // Test immediate addressing
    printf("Testing immediate addressing...\n");
    char *operand1 = "42", *operand2 = "100";
    void *source = immediate_addressing(cpu, operand1);
    void *destination = immediate_addressing(cpu, operand2);
    if (source && destination) {
        printf("Immediate addressing:\nsource:%d destination:%d\n", *(int *)source, *(int *)destination);
        handle_MOV(cpu, source, destination); // Simulate MOV instruction
        printf("After MOV:\nsource:%d destination:%d\n", *(int *)source, *(int *)destination);
    } else {
        printf("Invalid immediate addressing\n");
    }

    // Test register addressing
    printf("\nTesting register addressing...\n");
    operand1 = "AX", operand2 = "CX";
    source = register_addressing(cpu, operand1);
    destination = register_addressing(cpu, operand2);
    if (source && destination) {
        printf("Register addressing:\nsource:%d destination:%d\n", *(int *)source, *(int *)destination);
        handle_MOV(cpu, source, destination); // Simulate MOV instruction
        printf("After MOV:\nsource:%d destination:%d\n", *(int *)source, *(int *)destination);
    } else {
        printf("Invalid register addressing\n");
    }

    // Test memory direct addressing
    printf("\nTesting memory direct addressing...\n");
    operand1 = "[0]", operand2 = "[1]";
    source = memory_direct_addressing(cpu, operand1);
    destination = memory_direct_addressing(cpu, operand2);
    if (source && destination) {
        printf("Memory direct addressing:\nsource:%d destination:%d\n", *(int *)source, *(int *)destination);
        handle_MOV(cpu, source, destination); // Simulate MOV instruction
        printf("After MOV:\nsource:%d destination:%d\n", *(int *)source, *(int *)destination);
    } else {
        printf("Invalid memory direct addressing\n");
    }

    // Test register indirect addressing
    printf("\nTesting register indirect addressing...\n");
    operand1 = "[BX]", operand2 = "[DX]";
    source = register_indirect_addressing(cpu, operand1);
    destination = register_indirect_addressing(cpu, operand2);
    if (source && destination) {
        printf("Register indirect addressing:\nsource:%d destination:%d\n", *(int *)source, *(int *)destination);
        handle_MOV(cpu, source, destination); // Simulate MOV instruction
        printf("After MOV:\nsource:%d destination:%d\n", *(int *)source, *(int *)destination);
    } else {
        printf("Invalid register indirect addressing\n");
    }
    
    cpu_destroy(cpu);
    return 0;
}