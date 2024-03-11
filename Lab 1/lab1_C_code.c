///////////////////////////////////////
/// 640x480 version! 16-bit color
/// This code will segfault the original
/// DE1 computer
/// compile with
/// gcc graphics_video_16bit.c -o gr -O2 -lm
///
///////////////////////////////////////

// Joash Shankar, Nikhil Pillai, Ming He
// ECE 5760 Lab 1 Spring 2024
// Hardware ODE solver with HPS control and sonification

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <sys/time.h> 
#include <math.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
//#include "address_map_arm_brl4.h"

// video display
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
#define SDRAM_SPAN			  0x04000000
// characters
#define FPGA_CHAR_BASE        0xC9000000 
#define FPGA_CHAR_END         0xC9001FFF
#define FPGA_CHAR_SPAN        0x00002000
/* Cyclone V FPGA devices */
#define HW_REGS_BASE          0xff200000
//#define HW_REGS_SPAN        0x00200000 
#define HW_REGS_SPAN          0x00005000 
#define PI 3.1415926535

// graphics primitives
void VGA_text (int, int, char *);
void VGA_text_clear();
void VGA_box (int, int, int, int, short);
void VGA_rect (int, int, int, int, short);
void VGA_line(int, int, int, int, short) ;
void VGA_Vline(int, int, int, short) ;
void VGA_Hline(int, int, int, short) ;
void VGA_disc (int, int, int, short);
void VGA_circle (int, int, int, int);
void VGA_xy(float, float);
void VGA_xz(float, float);
void VGA_yz(float, float);

char input_buffer[64];

// access to text buffer
pthread_mutex_t buffer_lock= PTHREAD_MUTEX_INITIALIZER;
// counter protection
//pthread_mutex_t count_lock= PTHREAD_MUTEX_INITIALIZER;

// the two semaphores  
sem_t enter_cond ; // tells read1 that print is done
sem_t print_cond ; // tells draw that read is done

// 16-bit primary colors
#define red  (0+(0<<5)+(31<<11))
#define dark_red (0+(0<<5)+(15<<11))
#define green (0+(63<<5)+(0<<11))
#define dark_green (0+(31<<5)+(0<<11))
#define blue (31+(0<<5)+(0<<11))
#define dark_blue (15+(0<<5)+(0<<11))
#define yellow (0+(63<<5)+(31<<11))
#define cyan (31+(63<<5)+(0<<11))
#define magenta (31+(0<<5)+(31<<11))
#define black (0x0000)
#define gray (15+(31<<5)+(51<<11))
#define white (0xffff)
int colors[] = {red, dark_red, green, dark_green, blue, dark_blue, 
		yellow, cyan, magenta, gray, black, white};

// pixel macro
#define VGA_PIXEL(x,y,color) do{\
	int  *pixel_ptr ;\
	pixel_ptr = (int*)((char *)vga_pixel_ptr + (((y)*640+(x))<<1)) ; \
	*(short *)pixel_ptr = (color);\
} while(0)

// the light weight buss base
void *h2p_lw_virtual_base;

// pixel buffer
volatile unsigned int * vga_pixel_ptr = NULL ;
void *vga_pixel_virtual_base;

// character buffer
volatile unsigned int * vga_char_ptr = NULL ;
void *vga_char_virtual_base;

// /dev/mem file id
int fd;
int xcoord = 81;
// measure time
struct timeval t1, t2;
double elapsedTime;

// x, y, z, clk, rst, xinit, yinit, zinit pointers
volatile signed int* x_pio_ptr = NULL;
volatile signed int* y_pio_ptr = NULL;
volatile signed int* z_pio_ptr = NULL;
volatile unsigned char* clk_pio_ptr = NULL;
volatile unsigned char* rst_pio_ptr = NULL;
volatile signed int* x_init_pio_ptr = NULL;
volatile signed int* y_init_pio_ptr = NULL;
volatile signed int* z_init_pio_ptr = NULL;

// parameters sigma, rho, beta pointers
volatile signed int* sigma_pio_ptr = NULL;
volatile signed int* rho_pio_ptr = NULL;
volatile signed int* beta_pio_ptr = NULL;

// address mapping
#define X_PIO 0x00
#define Y_PIO 0x10
#define Z_PIO 0x20
#define CLK_PIO 0x30
#define RST_PIO 0x40
#define SIGMA_PIO 0x50
#define RHO_PIO 0x60
#define BETA_PIO 0x70
#define X_INIT_PIO 0x80
#define Y_INIT_PIO 0x90
#define Z_INIT_PIO 0x100

typedef signed int fix;

#define fix2float(a) ((float)(a) / 1048576.0) // fix(a) / 2^20
#define float2fix(a) (fix)(a * 1048576.0)

int user_int = 0; // variable to track user input (number) choice
float temp_x = -1.0;
float temp_y = 0.1;
float temp_z = 25.0;
float temp_sigma = 10.0;
float temp_rho = 28.0;
float temp_beta = 2.6667;
int speed_var = 5000;
int temp_speed;
int start_stop = 1;
int change_init = 0;

// read the user input
void * read1(){
  while(1){
    // change category
    printf("Change init values (0), parameters (1), speed of animation (2), start/stop animation (3), and clearing the screen (4)");
    scanf("%i", &user_int);
    
    switch(user_int) {
      case 0: // x, y, z init values
        if (change_init == 0){
          printf("Enter x: ");
          scanf("%f", &temp_x);
          printf("Enter y: ");
          scanf("%f", &temp_y);
          printf("Enter z: ");
          scanf("%f", &temp_z);
          change_init = 1;
          *(x_init_pio_ptr) = float2fix(temp_x);
          *(y_init_pio_ptr) = float2fix(temp_y);
          *(z_init_pio_ptr) = float2fix(temp_z);
          // Sigma Rho Beta assignment
          *(sigma_pio_ptr) = float2fix(temp_sigma);
          *(rho_pio_ptr) = float2fix(temp_rho);
          *(beta_pio_ptr) = float2fix(temp_beta);
          
          /////////////////////
          // RESET OUR MODULE
          /////////////////////
          // First set clock low
          *(clk_pio_ptr) = 0;
          // Next set reset high
          *(rst_pio_ptr) = 1;
          // Next set clock high
          *(clk_pio_ptr) = 1;
          // Clear clock and reset lines
          *(clk_pio_ptr) = 0;
          *(rst_pio_ptr) = 0;  
        } else {
          printf("The initial conditions are already set! \n");
        }
        
        break;
          
      case 1: // sigma, rho, beta values
          printf("Enter sigma: ");
          scanf("%f", &temp_sigma);
          printf("Enter rho: ");
          scanf("%f", &temp_rho);
          printf("Enter beta: ");
          scanf("%f", &temp_beta);
          *(sigma_pio_ptr) = float2fix(temp_sigma);
          *(rho_pio_ptr) = float2fix(temp_rho);
          *(beta_pio_ptr) = float2fix(temp_beta);
          break;
          
      case 2: // speed up/slow down animation
          printf("Increase (1) or Decrease (0) speed?");
          scanf("%i", &temp_speed);
          if (temp_speed) { // increase speed
            if (speed_var > 1000) { // lower bound
              speed_var -= 1000; // decrease sleep time
            }
          } 
          else { // decrease speed
            if (speed_var < 30000) { // upper bound
              speed_var += 1000; // increase sleep time
            }
          }
          break;
      
      case 3: // start/stop animation
          start_stop = (start_stop == 0) ? 1:0;          
          break;
          
      case 4: // clear screen
          VGA_box(0, 0, 639, 479, black);
          break;
          
      default: // for invalid options chosen
          printf("Enter a valid option.\n");
          break;
    }
  }
}

char text_init_x[20] ;
char text_init_y[20] ;
char text_init_z[20] ;
char text_init_sigma[20] ;
char text_init_rho[20] ;
char text_init_beta[20] ;

void * draw() {
  int itr = 1;
  
  while(1){
    // send posedge
   if (start_stop == 0) {
     *(clk_pio_ptr) = 0;
   } else {
     *(clk_pio_ptr) = 1;
     *(clk_pio_ptr) = 0;
   }
		
    if(change_init){
      if(start_stop){
      itr = (itr + 1)%300;  

      VGA_xy(fix2float(*(x_pio_ptr)), fix2float(*(y_pio_ptr)));
      VGA_xz(fix2float(*(x_pio_ptr)), fix2float(*(z_pio_ptr)));
      VGA_yz(fix2float(*(y_pio_ptr)), fix2float(*(z_pio_ptr)));

      usleep(speed_var);
      }
    }
    
    sprintf(text_init_x, "%.4f", temp_x);
    sprintf(text_init_y, "%.4f", temp_y);
    sprintf(text_init_z, "%.4f", temp_z);
    sprintf(text_init_sigma, "%.4f", temp_sigma);
    sprintf(text_init_rho, "%.4f", temp_rho);
    sprintf(text_init_beta, "%.4f", temp_beta);
    
    VGA_text (5, 37, "Init x: ");
	  VGA_text (5, 39, "Init y: ");
	  VGA_text (5, 41, "Init z: ");
    VGA_text (5, 43, "Sigma: ");
	  VGA_text (5, 45, "Rho: ");
	  VGA_text (5, 47, "Beta: ");
    
    VGA_text (13, 37, text_init_x);
	  VGA_text (13, 39, text_init_y);
	  VGA_text (13, 41, text_init_z);
    VGA_text (13, 43, text_init_sigma);
	  VGA_text (13, 45, text_init_rho);
	  VGA_text (13, 47, text_init_beta);
  }
}

int main(void) {
  	
	// === need to mmap: =======================
	// FPGA_CHAR_BASE
	// FPGA_ONCHIP_BASE      
	// HW_REGS_BASE        
  
  // the thread identifiers
  pthread_t thread_read, thread_draw;
  
	// the semaphore inits
	// read is not ready becuase nothing has been input yet
	sem_init(&enter_cond, 0, 0);
	// print is ready at init time
	sem_init(&print_cond, 0, 1); 
  
  pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  
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
    
  x_pio_ptr = (signed int *)(h2p_lw_virtual_base + X_PIO);
  y_pio_ptr = (signed int *)(h2p_lw_virtual_base + Y_PIO);
  z_pio_ptr = (signed int *)(h2p_lw_virtual_base + Z_PIO);
  clk_pio_ptr = (unsigned char *)(h2p_lw_virtual_base + CLK_PIO);
  rst_pio_ptr = (unsigned char *)(h2p_lw_virtual_base + RST_PIO);
  sigma_pio_ptr = (signed int *)(h2p_lw_virtual_base + SIGMA_PIO);
  rho_pio_ptr = (signed int *)(h2p_lw_virtual_base + RHO_PIO);
  beta_pio_ptr = (signed int *)(h2p_lw_virtual_base + BETA_PIO);
  x_init_pio_ptr = (signed int *)(h2p_lw_virtual_base + X_INIT_PIO);
  y_init_pio_ptr = (signed int *)(h2p_lw_virtual_base + Y_INIT_PIO);
  z_init_pio_ptr = (signed int *)(h2p_lw_virtual_base + Z_INIT_PIO);

	// === get VGA char addr =====================
	// get virtual addr that maps to physical
	vga_char_virtual_base = mmap( NULL, FPGA_CHAR_SPAN, ( 	PROT_READ | PROT_WRITE ), MAP_SHARED, fd, FPGA_CHAR_BASE );	
	if( vga_char_virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap2() failed...\n" );
		close( fd );
		return(1);
	}
    
 // Get the address that maps to the FPGA LED control 
	vga_char_ptr =(unsigned int *)(vga_char_virtual_base);

	// === get VGA pixel addr ====================
	// get virtual addr that maps to physical
	vga_pixel_virtual_base = mmap( NULL, SDRAM_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, SDRAM_BASE);	
	if( vga_pixel_virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap3() failed...\n" );
		close( fd );
		return(1);
	}
    
    // Get the address that maps to the FPGA pixel buffer
	vga_pixel_ptr =(unsigned int *)(vga_pixel_virtual_base);

	// ===========================================

	/* create a message to be displayed on the VGA 
          and LCD displays */
	char text_xy_yellow[40] = "X-Y Projection";
	char text_yz_red[40] = "Y-Z Projection";
	char text_xz_green[40] = "X-Z Projection";

	char num_string[20], time_string[20] ;
	char color_index = 0 ;
	int color_counter = 0 ;
	
	// position of disk primitive
	int disc_x = 0;
	// position of circle primitive
	int circle_x = 0 ;
	// position of box primitive
	int box_x = 5 ;
	// position of vertical line primitive
	int Vline_x = 350;
	// position of horizontal line primitive
	int Hline_y = 250;

	//VGA_text (34, 1, text_top_row);
	//VGA_text (34, 2, text_bottom_row);
	// clear the screen
	VGA_box (0, 0, 639, 479, 0x0000);
	// clear the text
	VGA_text_clear();
	// write text
	VGA_text (13, 9, text_xy_yellow);
	VGA_text (33, 37, text_yz_red);
	VGA_text (53, 9, text_xz_green);
 
  pthread_create(&thread_read, NULL, read1, NULL);
  pthread_create(&thread_draw,NULL, draw,NULL);
	
	// R bits 11-15 mask 0xf800
	// G bits 5-10  mask 0x07e0
	// B bits 0-4   mask 0x001f
	// so color = B+(G<<5)+(R<<11);

  pthread_join(thread_read, NULL);
  pthread_join(thread_draw, NULL);
} // end main

/****************************************************************************************
 * Draw x, y, and z output
****************************************************************************************/
void VGA_xy(float x, float y) {
  VGA_PIXEL(160 - (int)(x*3), 160 - (int)(y*3), yellow);
}

void VGA_xz(float x, float z) {
  VGA_PIXEL(480 - (int)(x*3), 240 - (int)(z*3), green);
}

void VGA_yz(float y, float z) {
  VGA_PIXEL(320 - (int)(y*3), 450 - (int)(z*3), red);
}

/****************************************************************************************
 * Subroutine to send a string of text to the VGA monitor 
****************************************************************************************/
void VGA_text(int x, int y, char * text_ptr)
{
  	volatile char * character_buffer = (char *) vga_char_ptr ;	// VGA character buffer
	int offset;
	/* assume that the text string fits on one line */
	offset = (y << 7) + x;
	while ( *(text_ptr) )
	{
		// write to the character buffer
		*(character_buffer + offset) = *(text_ptr);	
		++text_ptr;
		++offset;
	}
}

/****************************************************************************************
 * Subroutine to clear text to the VGA monitor 
****************************************************************************************/
void VGA_text_clear()
{
  	volatile char * character_buffer = (char *) vga_char_ptr ;	// VGA character buffer
	int offset, x, y;
	for (x=0; x<79; x++){
		for (y=0; y<59; y++){
	/* assume that the text string fits on one line */
			offset = (y << 7) + x;
			// write to the character buffer
			*(character_buffer + offset) = ' ';		
		}
	}
}

/****************************************************************************************
 * Draw a filled rectangle on the VGA monitor 
****************************************************************************************/
#define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0) 

void VGA_box(int x1, int y1, int x2, int y2, short pixel_color)
{
	char  *pixel_ptr ; 
	int row, col;

	/* check and fix box coordinates to be valid */
	if (x1>639) x1 = 639;
	if (y1>479) y1 = 479;
	if (x2>639) x2 = 639;
	if (y2>479) y2 = 479;
	if (x1<0) x1 = 0;
	if (y1<0) y1 = 0;
	if (x2<0) x2 = 0;
	if (y2<0) y2 = 0;
	if (x1>x2) SWAP(x1,x2);
	if (y1>y2) SWAP(y1,y2);
	for (row = y1; row <= y2; row++)
		for (col = x1; col <= x2; ++col)
		{
			//640x480
			//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
			// set pixel color
			//*(char *)pixel_ptr = pixel_color;	
			VGA_PIXEL(col,row,pixel_color);	
		}
}

/****************************************************************************************
 * Draw a outline rectangle on the VGA monitor 
****************************************************************************************/
#define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0) 

void VGA_rect(int x1, int y1, int x2, int y2, short pixel_color)
{
	char  *pixel_ptr ; 
	int row, col;

	/* check and fix box coordinates to be valid */
	if (x1>639) x1 = 639;
	if (y1>479) y1 = 479;
	if (x2>639) x2 = 639;
	if (y2>479) y2 = 479;
	if (x1<0) x1 = 0;
	if (y1<0) y1 = 0;
	if (x2<0) x2 = 0;
	if (y2<0) y2 = 0;
	if (x1>x2) SWAP(x1,x2);
	if (y1>y2) SWAP(y1,y2);
	// left edge
	col = x1;
	for (row = y1; row <= y2; row++){
		//640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;	
		VGA_PIXEL(col,row,pixel_color);		
	}
		
	// right edge
	col = x2;
	for (row = y1; row <= y2; row++){
		//640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;	
		VGA_PIXEL(col,row,pixel_color);		
	}
	
	// top edge
	row = y1;
	for (col = x1; col <= x2; ++col){
		//640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;	
		VGA_PIXEL(col,row,pixel_color);
	}
	
	// bottom edge
	row = y2;
	for (col = x1; col <= x2; ++col){
		//640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;
		VGA_PIXEL(col,row,pixel_color);
	}
}

/****************************************************************************************
 * Draw a horixontal line on the VGA monitor 
****************************************************************************************/
#define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0) 

void VGA_Hline(int x1, int y1, int x2, short pixel_color)
{
	char  *pixel_ptr ; 
	int row, col;

	/* check and fix box coordinates to be valid */
	if (x1>639) x1 = 639;
	if (y1>479) y1 = 479;
	if (x2>639) x2 = 639;
	if (x1<0) x1 = 0;
	if (y1<0) y1 = 0;
	if (x2<0) x2 = 0;
	if (x1>x2) SWAP(x1,x2);
	// line
	row = y1;
	for (col = x1; col <= x2; ++col){
		//640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;	
		VGA_PIXEL(col,row,pixel_color);		
	}
}

/****************************************************************************************
 * Draw a vertical line on the VGA monitor 
****************************************************************************************/
#define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0) 

void VGA_Vline(int x1, int y1, int y2, short pixel_color)
{
	char  *pixel_ptr ; 
	int row, col;

	/* check and fix box coordinates to be valid */
	if (x1>639) x1 = 639;
	if (y1>479) y1 = 479;
	if (y2>479) y2 = 479;
	if (x1<0) x1 = 0;
	if (y1<0) y1 = 0;
	if (y2<0) y2 = 0;
	if (y1>y2) SWAP(y1,y2);
	// line
	col = x1;
	for (row = y1; row <= y2; row++){
		//640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10)    + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;	
		VGA_PIXEL(col,row,pixel_color);			
	}
}


/****************************************************************************************
 * Draw a filled circle on the VGA monitor 
****************************************************************************************/

void VGA_disc(int x, int y, int r, short pixel_color)
{
	char  *pixel_ptr ; 
	int row, col, rsqr, xc, yc;
	
	rsqr = r*r;
	
	for (yc = -r; yc <= r; yc++)
		for (xc = -r; xc <= r; xc++)
		{
			col = xc;
			row = yc;
			// add the r to make the edge smoother
			if(col*col+row*row <= rsqr+r){
				col += x; // add the center point
				row += y; // add the center point
				//check for valid 640x480
				if (col>639) col = 639;
				if (row>479) row = 479;
				if (col<0) col = 0;
				if (row<0) row = 0;
				//pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;
				// set pixel color
				//*(char *)pixel_ptr = pixel_color;
				VGA_PIXEL(col,row,pixel_color);	
			}
					
		}
}

/****************************************************************************************
 * Draw a  circle on the VGA monitor 
****************************************************************************************/

void VGA_circle(int x, int y, int r, int pixel_color)
{
	char  *pixel_ptr ; 
	int row, col, rsqr, xc, yc;
	int col1, row1;
	rsqr = r*r;
	
	for (yc = -r; yc <= r; yc++){
		//row = yc;
		col1 = (int)sqrt((float)(rsqr + r - yc*yc));
		// right edge
		col = col1 + x; // add the center point
		row = yc + y; // add the center point
		//check for valid 640x480
		if (col>639) col = 639;
		if (row>479) row = 479;
		if (col<0) col = 0;
		if (row<0) row = 0;
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;
		VGA_PIXEL(col,row,pixel_color);	
		// left edge
		col = -col1 + x; // add the center point
		//check for valid 640x480
		if (col>639) col = 639;
		if (row>479) row = 479;
		if (col<0) col = 0;
		if (row<0) row = 0;
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;
		VGA_PIXEL(col,row,pixel_color);	
	}
	for (xc = -r; xc <= r; xc++){
		//row = yc;
		row1 = (int)sqrt((float)(rsqr + r - xc*xc));
		// right edge
		col = xc + x; // add the center point
		row = row1 + y; // add the center point
		//check for valid 640x480
		if (col>639) col = 639;
		if (row>479) row = 479;
		if (col<0) col = 0;
		if (row<0) row = 0;
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;
		VGA_PIXEL(col,row,pixel_color);	
		// left edge
		row = -row1 + y; // add the center point
		//check for valid 640x480
		if (col>639) col = 639;
		if (row>479) row = 479;
		if (col<0) col = 0;
		if (row<0) row = 0;
		//pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;
		// set pixel color
		//*(char *)pixel_ptr = pixel_color;
		VGA_PIXEL(col,row,pixel_color);	
	}
}

// =============================================
// === Draw a line
// =============================================
//plot a line 
//at x1,y1 to x2,y2 with color 
//Code is from David Rodgers,
//"Procedural Elements of Computer Graphics",1985
void VGA_line(int x1, int y1, int x2, int y2, short c) {
	int e;
	signed int dx,dy,j, temp;
	signed int s1,s2, xchange;
     signed int x,y;
	char *pixel_ptr ;
	
	/* check and fix line coordinates to be valid */
	if (x1>639) x1 = 639;
	if (y1>479) y1 = 479;
	if (x2>639) x2 = 639;
	if (y2>479) y2 = 479;
	if (x1<0) x1 = 0;
	if (y1<0) y1 = 0;
	if (x2<0) x2 = 0;
	if (y2<0) y2 = 0;
        
	x = x1;
	y = y1;
	
	//take absolute value
	if (x2 < x1) {
		dx = x1 - x2;
		s1 = -1;
	}

	else if (x2 == x1) {
		dx = 0;
		s1 = 0;
	}

	else {
		dx = x2 - x1;
		s1 = 1;
	}

	if (y2 < y1) {
		dy = y1 - y2;
		s2 = -1;
	}

	else if (y2 == y1) {
		dy = 0;
		s2 = 0;
	}

	else {
		dy = y2 - y1;
		s2 = 1;
	}

	xchange = 0;   

	if (dy>dx) {
		temp = dx;
		dx = dy;
		dy = temp;
		xchange = 1;
	} 

	e = ((int)dy<<1) - dx;  
	 
	for (j=0; j<=dx; j++) {
		//video_pt(x,y,c); //640x480
		//pixel_ptr = (char *)vga_pixel_ptr + (y<<10)+ x; 
		// set pixel color
		//*(char *)pixel_ptr = c;
		VGA_PIXEL(x,y,c);			
		 
		if (e>=0) {
			if (xchange==1) x = x + s1;
			else y = y + s2;
			e = e - ((int)dx<<1);
		}

		if (xchange==1) y = y + s2;
		else x = x + s1;

		e = e + ((int)dy<<1);
	}
}