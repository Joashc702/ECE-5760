// Joash Shankar, Ming He, Nikhil Pillai
// ECE 5760 Lab 3 Spring 2024
// Multiprocessor Drum Synthesis
// Lab 3 Week 3 C code

///////////////////////////////////////
/// 640x480 version!
/// change to fixed point 
/// compile with:
/// gcc Lab3W3.c -o drum -lm -O3 -lpthread 
///////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <sys/time.h> 
#include <pthread.h>

#define HW_REGS_BASE 0xff200000
#define HW_REGS_SPAN 0x00005000 

void *h2p_lw_virtual_base;
int fd;

volatile unsigned int* counter_pio_ptr = NULL;
#define COUNTER_PIO 0x00
volatile unsigned int* num_rows_pio_ptr = NULL;
#define ROWS_INIT_PIO 0x10
volatile unsigned char* reset_pio_ptr = NULL;
#define RESET_PIO 0x20
volatile unsigned int* ampl_pio_ptr = NULL;
#define AMPL_INIT_PIO 0x30
volatile unsigned int* row_incr_pio_ptr = NULL;
#define ROW_INCR_PIO 0x40
volatile unsigned int* rho_pio_ptr = NULL;
#define RHO_GAIN_PIO 0x50
volatile unsigned int* num_cols_pio_ptr = NULL;
#define COLS_INIT_PIO 0x60
volatile unsigned int* col_incr_pio_ptr = NULL;
#define COL_INCR_PIO 0x70

typedef signed int fix;
#define float2fix(a) (fix)(a * 131072.0)

pthread_mutex_t lock;

// TODO test user input, add initial value pio at bottom, timing (counter) math, dynamically change num of rows

// Global variables for demonstration purposes
int temp_num_rows = 1;
int temp_num_cols = 1;
float temp_init_ampl = 1.0;
float temp_init_rho = 1.0;

// scanning thread
void *scan_drum(void *arg) {
	int user_int;

	while (1) {
		// picking things to change
		printf("Display time for num of cycles(0), Change height/num of rows (1), Change init amplitude (2), Change rho gain (3): ");
    scanf("%d", &user_int);

		switch(user_int) {
			case 0: // display time for the number of cycles
				printf("Number of Cycles = %d\n", *(counter_pio_ptr));
				break;

			case 1: // change the height/number of rows in the drum
				printf("Enter number of rows: ");
        scanf("%d", &temp_num_rows);
				*(num_rows_pio_ptr) = temp_num_rows;
        float incr_ampl_row = temp_init_ampl / ((int) (temp_num_rows/2.0));
        *(row_incr_pio_ptr) = float2fix(incr_ampl_row);
				*(reset_pio_ptr) = 1;
        *(reset_pio_ptr) = 0;
				break;

			case 2: // change initial amplitude
				printf("Enter initial amplitude: ");
        scanf("%d", &temp_init_ampl);
				float incr_ampl_row = temp_init_ampl / (temp_num_rows/2);
        float incr_ampl_col = temp_init_ampl/ (temp_num_cols/2);
				printf("Increment Amplitude: %f\n", incr_ampl_row);
				*(row_incr_pio_ptr) = float2fix(incr_ampl_row);
   	    *(col_incr_pio_ptr) = float2fix(incr_ampl_col);
				*(reset_pio_ptr) = 1;
        *(reset_pio_ptr) = 0;
				break;

			case 3: // change rho gain
				printf("Enter rho gain: ");
        scanf("%d", &temp_init_rho);
				*(rho_pio_ptr) = temp_init_rho;
				*(reset_pio_ptr) = 1;
        *(reset_pio_ptr) = 0;
				break;
        
      case 4: // change the height/number of rows in the drum
				printf("Enter number of cols: ");
        scanf("%d", &temp_num_cols);
        float incr_ampl_col = temp_init_ampl/ (temp_num_cols/2);
        *(col_incr_pio_ptr) = float2fix(incr_ampl_col);
				*(reset_pio_ptr) = 1;
        *(reset_pio_ptr) = 0;
				break;
        
      default:
        printf("Invalid option.\n");
        break;
		}
	}
}

int main(void) {
    // === get FPGA addresses ==================
      // Open /dev/mem
  	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) 	{
  		printf( "ERROR: could not open \"/dev/mem\"...\n" );
  		return( 1 );
  	}
      
    // get virtual addr that maps to physical
  	h2p_lw_virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );	
  	if( h2p_lw_virtual_base == MAP_FAILED ) {
  		printf( "ERROR: mmap1() failed...\n" );
  		close( fd );
  		return(1);
  	}
    counter_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + COUNTER_PIO);
    num_rows_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + ROWS_INIT_PIO);
  	reset_pio_ptr = (unsigned char *)(h2p_lw_virtual_base + RESET_PIO);
  	ampl_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + AMPL_INIT_PIO);
  	row_incr_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + ROW_INCR_PIO);
  	rho_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + RHO_GAIN_PIO);
    num_cols_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + COLS_INIT_PIO);
    col_incr_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + COL_INCR_PIO);
    
    // default values from reset
    *(row_incr_pio_ptr) = float2fix(temp_num_rows);

    // Start thread and wait for it to finish
    pthread_t drum_scan;
    pthread_create(&drum_scan, NULL, scan_drum, NULL);
    pthread_join(drum_scan, NULL);
    
    return 0;
}