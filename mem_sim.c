#include "mem_sim.h"

#define OFFSET_MASK (PAGE_SIZE - 1) // Mask to extract the offset

int frame_out=0;
int swap_loc=0;
int cur_loc =0;

// Initialize the simulation environment
sim_database* init_system(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_heap_stack_size) {
    if (exe_file_name == NULL || swap_file_name == NULL) {
        fprintf(stderr, "Error: Filename cannot be NULL.\n");
        return NULL;
    }

    sim_database *db = (sim_database *)malloc(sizeof(sim_database));
    if (!db) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Open the program file for reading and writing
    db->program_fd = open(exe_file_name, O_RDWR, S_IRUSR | S_IWUSR);
    if (db->program_fd < 0) {
        perror("Failed to open program file");
        free(db);
        return NULL;
    }

    // Open the swap file for reading and writing
    db->swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (db->swapfile_fd < 0) {
        perror("Failed to open swap file");
        close(db->program_fd);
        free(db);
        return NULL;
    }

    // Truncate the swap file to SWAP_SIZE bytes
    if (ftruncate(db->swapfile_fd, SWAP_SIZE) == -1) {
        perror("Failed to truncate swap file");
        // Handle error: close resources and free memory
        close(db->program_fd);
        close(db->swapfile_fd);
        free(db);
        return NULL;
    }

    db->text_size = text_size;
    db->data_size = data_size;
    db->bss_heap_stack_size = bss_heap_stack_size;

    // Initialize main memory with zeros
    memset(db->main_memory, '0', MEMORY_SIZE);

    // Initialize the swap file with zeros
    char zero_buffer[PAGE_SIZE];
    memset(zero_buffer, '0', PAGE_SIZE);
    for (int i = 0; i < SWAP_SIZE/PAGE_SIZE; i++) {
        if (write(db->swapfile_fd, zero_buffer, PAGE_SIZE) != PAGE_SIZE) {
            perror("Failed to initialize swap file with zeros");
            close(db->program_fd);
            close(db->swapfile_fd);
            free(db);
            return NULL;
        }
    }

    for (int i = 0; i < NUM_OF_PAGES; i++) {
        db->page_table[i].V = 0;
        db->page_table[i].D = 0;
        db->page_table[i].P = (i < text_size / PAGE_SIZE) ? 1 : 0;
        db->page_table[i].frame_swap = -1;
    }

    return db;
}

void clear_page_in_swap(int swapfile_fd, int page_number) {
    char zero_buffer[PAGE_SIZE];
    memset(zero_buffer, '0', PAGE_SIZE);

    off_t swap_offset = page_number * PAGE_SIZE;
    lseek(swapfile_fd, swap_offset, SEEK_SET);

    if (write(swapfile_fd, zero_buffer, PAGE_SIZE) != PAGE_SIZE) {
        perror("Failed to clear page in swap file");
        close(swapfile_fd);
        exit(1); // Handle error appropriately
    }
}

// Helper function to load a page into memory
void load_page_into_memory(sim_database *mem_sim, int page_number, int frame_number) {
    page_descriptor *page = &mem_sim->page_table[page_number];

    if (page->D == 1) { // Dirty page, read from the swap file
        lseek(mem_sim->swapfile_fd, swap_loc * PAGE_SIZE, SEEK_SET); // Use saved frame position
        read(mem_sim->swapfile_fd, mem_sim->main_memory + swap_loc * PAGE_SIZE, PAGE_SIZE);
        clear_page_in_swap(mem_sim->swapfile_fd,swap_loc);
    } else { // Clean page, allocate new page or read from the program file
        if (page_number < ((mem_sim->text_size + mem_sim->data_size) / PAGE_SIZE)) {
            lseek(mem_sim->program_fd, page_number * PAGE_SIZE, SEEK_SET);
            read(mem_sim->program_fd, mem_sim->main_memory + frame_number * PAGE_SIZE, PAGE_SIZE);
        } else {
            memset(mem_sim->main_memory + frame_number * PAGE_SIZE, '0', PAGE_SIZE); // page number 10 and above - load new empty page
        }
    }

    page->V = 1; // Mark the page as valid
    page->frame_swap = frame_number; // Store the frame number
}

// Load a page into memory and return the value at the address
char load(struct sim_database *mem_sim, int address) {
    int page_number = address>>3;
    int offset = address & OFFSET_MASK;
    page_descriptor *page = &mem_sim->page_table[page_number];
    int frame_number;
    frame_number=frame_out;

    if (page->V == 0) { // Page Fault: Page is not in memmory
        for(int i=0; i<PAGE_SIZE; i++){ //fine old frame and make as unvalid
                if(mem_sim->page_table[i].V==1 && mem_sim->page_table[i].frame_swap == frame_number){
                    if (mem_sim->page_table[i].D == 1) {
                        // Write the dirty page to the swap file
                        lseek(mem_sim->swapfile_fd, frame_number* PAGE_SIZE, SEEK_SET);
                        write(mem_sim->swapfile_fd, mem_sim->main_memory + frame_number * PAGE_SIZE, PAGE_SIZE);
                        swap_loc =  cur_loc;
                        mem_sim->page_table[i].frame_swap=swap_loc;
                        cur_loc++;
                    }
                    mem_sim->page_table[i].V=0;
                }
        }

        // Load the requested page into memory
        load_page_into_memory(mem_sim, page_number, frame_number);
    }
    else {
        frame_number = page->frame_swap;
    }

    frame_out++;

    if(frame_out>4){
        frame_out=0;
    }

    return mem_sim->main_memory[frame_number * PAGE_SIZE + offset];
}

// Store a value at a specific address
void store(struct sim_database *mem_sim, int address, char value) {
    int page_number = address>>3;
    int offset = address & OFFSET_MASK;
    page_descriptor *page = &mem_sim->page_table[page_number];
    int frame_number;
    frame_number=frame_out;

    if (page->V == 0) { // Page Fault: Page is not in memory
        load(mem_sim, address);
    } else {
        frame_number = page->frame_swap;
        }

    if (page->P == 1) { // Read-only page, cannot store
        fprintf(stderr, "Error: Attempt to write to a read-only page.\n");
        return;
    }
    // Store the value at the address
    mem_sim->main_memory[frame_number * PAGE_SIZE + offset] = value;
    page->D = 1; // Mark the page as dirty
}

//-------------------------------------------------------------------------------------------------------

// Clear the simulation environment
void clear_system(sim_database *db) {
    if (db) {
        close(db->program_fd);
        close(db->swapfile_fd);
        free(db);
    }
}

void print_memory(sim_database* mem_sim) {
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", mem_sim->main_memory[i]);
    }
}

void print_swap(sim_database* mem_sim) {
    char str[PAGE_SIZE];
    int i;
    printf("\n Swap memory\n");
    lseek(mem_sim->swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(mem_sim->swapfile_fd, str, PAGE_SIZE) == PAGE_SIZE) {
        for(i = 0; i < PAGE_SIZE; i++) {
            printf("[%c]\t", str[i]);
        }
        printf("\n");
    }
}

void print_page_table(sim_database* mem_sim) {
    int i;
    printf("\n page table \n");
    printf("Valid|Dirty|Permission|Frame_swap\n");
    for(i = 0; i < NUM_OF_PAGES; i++) {
        printf("[%d]\t   [%d]\t   [%d]\t   [%d]\n", mem_sim->page_table[i].V,
               mem_sim->page_table[i].D,
               mem_sim->page_table[i].P, mem_sim->page_table[i].frame_swap);
    }
}
