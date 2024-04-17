// sram.c
// Joash Shankar, Ming He, Nikhil Pillai
// ECE 5760 Lab 2 Spring 2024
// Mandelbrot Set Visualizer
// mandelbrot.c

///////////////////////////////////////
/// 640x480 version!
/// change to fixed point 
/// compile with:
/// gcc mandelbrot.c -o mandel -Os -lpthread
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
#define MOUSE_DEV "/dev/input/mice"

#define HW_REGS_BASE 0xff200000
#define HW_REGS_SPAN 0x00005000 

#include "address_map_arm_br14.h"

void *h2p_lw_virtual_base;

// RAM fp buffer
volatile unsigned int * fp_ram_ptr = NULL;
void *fp_ram_virtual_base;
// file id
int fd;

// float2fix conversion macro
typedef signed int fix;
#define float2fix(a) (fix)(a * 8388608.0)

// pointers to interact with VGA screen


// base addresses for respective PIO block


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
    
    // pointers are assigned by adding defined offset defined above based on memory locations from Platform Designer
    
    
    // default values for reset
    
    
	// get RAM float param addr
	fp_ram_virtual_base = mmap(NULL, FPGA_ONCHIP_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, FPGA_ONCHIP_BASE); // shared memory btwn HPS and FPGA location
	
	if (fp_ram_virtual_base == MAP_FAILED) {
		printf( "ERROR: mmap3() failed...\n" );
  		close( fd );
  		return(1);
	}
	
	// get addr that maps to RAM buffer
    fp_ram_ptr = (unsigned int *)(fp_ram_virtual_base);

    return 0;
}