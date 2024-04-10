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
#include <math.h>

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
volatile signed int* ampl_pio_ptr = NULL;
#define AMPL_INIT_PIO 0x30
volatile unsigned int* row_incr_pio_ptr = NULL;
#define ROW_INCR_PIO 0x40
volatile unsigned int* rho_pio_ptr = NULL;
#define RHO_GAIN_PIO 0x50
volatile unsigned int* num_cols_pio_ptr = NULL;
#define COLS_INIT_PIO 0x60
volatile unsigned int* half_rows_pio_ptr = NULL;
#define COL_INCR_PIO 0x70
volatile unsigned char* done_signal_pio_ptr = NULL;
#define DONE_INCR_PIO 0x80

int ampl_changed = 0;
typedef signed int fix;
#define float2fix(a) (fix)(a * 131072.0) //
#define fix2float(a) ((float)((a) / 131072.0)) // 1 / 2^17
#define MIN(X,Y) ((X<Y) ? (X) : (Y))

pthread_mutex_t lock;

////////////////
// TODO:
// figure out how to read back amplitude --> gtension to switches if needed
// step size - time permitting
/////////////////

// Global variables for demonstration purposes
float temp_time = 0.0;
int temp_num_rows = 30;
int temp_num_cols = 128;
float temp_init_ampl = (1.0/8.0);
float g_tension = 8.0;
int half_rows = 256;
int half_cols = 64;
int row = 1;
int col = 1;

// scanning thread
void *scan_drum(void *arg) {
	int user_int;

	while (1) {
		// picking things to change
		printf("Display time for num of cycles(0), Change height/num of rows (1), Change init amplitude (2), Change rho gain (3): ");
    scanf("%d", &user_int);
    float incr_ampl_row = 0.0;
    float incr_ampl_col = 0.0;
		switch(user_int) {
			case 0: // display time for the number of cycles
        temp_time = 2*1000000 * (1.0 / 50000000.0);
        
				printf("Time (microsec) = %f\n", (*(counter_pio_ptr) * temp_time));
				break;

			case 1: // change the height/number of rows in the drum
				printf("Enter number of rows: ");
        //printf("%d\n", *(num_rows_pio_ptr));
        scanf("%d", &temp_num_rows);
				*(num_rows_pio_ptr) = float2fix(temp_num_rows);
        *(half_rows_pio_ptr) = float2fix((int)(temp_num_rows/2));
        //incr_ampl_row = temp_init_ampl / ((temp_num_rows/2));
				//*(row_incr_pio_ptr) = float2fix(incr_ampl_row);
        *(row_incr_pio_ptr) = float2fix(1/256.0);
       	*(rho_pio_ptr) = float2fix(1/16.0);
				*(reset_pio_ptr) = 1;
        //usleep(1000); // make sure FPGA can receive the reset signal
        *(reset_pio_ptr) = 0;
        //printf("%d\n", *(half_rows_pio_ptr));
        ampl_changed = 1;
				break;

			case 2: // change initial amplitude
				printf("Enter initial amplitude: ");
        scanf("%f", &temp_init_ampl);
        //printf("%d", temp_num_rows);
				incr_ampl_row = temp_init_ampl / (temp_num_rows/2);
				//printf("Increment Amplitude: %f\n", incr_ampl_row);
				*(row_incr_pio_ptr) = float2fix(incr_ampl_row);
				*(reset_pio_ptr) = 1;
        *(reset_pio_ptr) = 0;
        ampl_changed = 1;
				break;

			case 3: // change rho gain
				printf("Enter rho gain (Enter value btwn 8 and 32): ");
        scanf("%.6f", &g_tension); // 0.03125 (1/32) - 0.125 (1/8)
        
        float track_rho;
        track_rho = float2fix(MIN(0.49, 0.25 + pow(fix2float(*ampl_pio_ptr) * (1.0 / g_tension), 2.0)));
        //printf("%f\n", fix2float(track_rho));
        //printf("%f\n", fix2float(*ampl_pio_ptr));
        //printf("%f\n", pow(fix2float(*ampl_pio_ptr) * (1.0 / g_tension), 2));
       // printf("%f\n", fix2float(*ampl_pio_ptr) * (1.0 / g_tension));

				*(rho_pio_ptr) = track_rho;
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
  	ampl_pio_ptr = (signed int *)(h2p_lw_virtual_base + AMPL_INIT_PIO);
  	row_incr_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + ROW_INCR_PIO);
  	rho_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + RHO_GAIN_PIO);
    num_cols_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + COLS_INIT_PIO);
    half_rows_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + COL_INCR_PIO);
    
    // default values from reset
    *(row_incr_pio_ptr) = float2fix(0.125);
    *(num_rows_pio_ptr) = temp_num_rows;
    *(ampl_pio_ptr) = temp_init_ampl;
    *(rho_pio_ptr) = fix2float(1.0/16.0);
    *(half_rows_pio_ptr) = half_rows;
    *(reset_pio_ptr) = 1;
    *(reset_pio_ptr) = 0;

    // Start thread and wait for it to finish
    pthread_t drum_scan;
    pthread_create(&drum_scan, NULL, scan_drum, NULL);
    pthread_join(drum_scan, NULL);
    
    return 0;
}