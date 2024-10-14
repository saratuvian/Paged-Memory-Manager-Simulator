#include <stdio.h>
#include <stdlib.h>
#include "mem_sim.h"

int main() {
    struct sim_database *mem_sim;

    // Initialize the memory simulation system
    mem_sim = init_system("exec_file", "swap_file", 40, 40, 120);
    if (!mem_sim) {
        fprintf(stderr, "Failed to initialize memory simulation system.\n");
        return 1;
    }

    // Perform operations to test swapping
    char val;

    val = load(mem_sim,0);
    val = load(mem_sim,9);
    val = load(mem_sim,19);
    val = load(mem_sim,29);
    val = load(mem_sim,39);

    store(mem_sim,49,'A');
    val = load(mem_sim,0);
    val = load(mem_sim,9);
    val = load(mem_sim,19);
    val = load(mem_sim,29);
    val = load(mem_sim,39);
    val = load(mem_sim,49);

    print_swap(mem_sim);
    print_memory(mem_sim);
    print_page_table(mem_sim);

    // Clean up
    clear_system(mem_sim);

    return 0;
}