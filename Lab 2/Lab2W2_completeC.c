// Joash Shankar, Ming He, Nikhil Pillai
// ECE 5760 Lab 2 Spring 2024
// Mandelbrot Set Visualizer
// mandelbrot.c

///////////////////////////////////////
/// 640x480 version!
/// change to fixed point 
/// compile with:
/// gcc mandelbrot_video_fix.c -o mandel -Os
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

/* Cyclone V FPGA devices */
#define HW_REGS_BASE          0xff200000
//#define HW_REGS_SPAN        0x00200000 
#define HW_REGS_SPAN          0x00005000 
// the light weight buss base
void *h2p_lw_virtual_base;

// /dev/mem file id
int fd;

// counter
volatile unsigned int* counter_pio_ptr = NULL;
#define COUNTER_PIO 0x00

float render_time;
float past_time;

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
  past_time = ((float)(*counter_pio_ptr) * 1 );
  while(1) {
    //render_time = ((float)(*counter_pio_ptr) * 1 / 100000);
    render_time = ((float)(*counter_pio_ptr) * 1 / 100000);
    if (past_time != render_time) {
      printf("Render time in ms: %f\n", render_time);
      past_time = render_time;
    }
  }
}