### `read_me.md`

# Memory Simulation System

This project is a simple memory simulation system that uses a page table, main memory, and a swap file to simulate memory management. The system supports loading and storing values at specific addresses, with page replacement using a FIFO algorithm.

## File Structure

- `mem_sim.h`: Header file containing the definitions and function prototypes for the memory simulation system.
- `mem_sim.c`: Implementation of the memory simulation system.
- `main.c`: Main file containing the test cases to demonstrate the functionality of the memory simulation system.
- `run_me.sh`: Shell script to compile and run the program.

## How to Run

### Prerequisites
- GCC compiler

### Steps

1. **Compile and Run the Program**

   Use the provided shell script to compile and run the program:

   ```sh
   ./run_me.sh
   ```

   This script will compile `mem_sim.c` and `main.c` into an executable named `mem_sim` and then run it.

2. **Expected Output**

   The program will:
   - Initialize the memory simulation system.
   - Perform a series of load and store operations to test swapping.
   - Print the current state of the swap file, main memory, and page table.

   The output will show the physical memory, swap memory, and the page table's status.

## Functions and Their Purpose

### Initialization
- `sim_database* init_system(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_heap_stack_size);`
  Initializes the memory simulation system with the provided executable and swap file names, along with the sizes of text, data, and bss/heap/stack segments.

### Memory Operations
- `char load(struct sim_database *mem_sim, int address);`
  Loads a value from the specified address, handling page faults and swapping as necessary.
- `void store(struct sim_database *mem_sim, int address, char value);`
  Stores a value at the specified address, handling page faults and swapping as necessary.

### Cleanup
- `void clear_system(struct sim_database *mem_sim);`
  Cleans up and frees the resources used by the memory simulation system.

### Debugging and Visualization
- `void print_memory(sim_database* mem_sim);`
  Prints the current state of the physical memory.
- `void print_swap(sim_database* mem_sim);`
  Prints the current state of the swap memory.
- `void print_page_table(sim_database* mem_sim);`
  Prints the current state of the page table.

## Additional Notes

- The system uses a fixed page size of 8 bytes, a main memory size of 40 bytes, and a swap size of 200 bytes.
- The FIFO algorithm is used for page replacement, with a simple mechanism to track the next frame to be replaced.
- The simulation distinguishes between text, data, and bss/heap/stack segments, with text pages being read-only.

## Author

This memory simulation system was created as part of a learning exercise in memory management and operating system concepts.
