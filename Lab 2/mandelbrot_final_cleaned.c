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

void *h2p_lw_virtual_base;
int fd;

// pointers to interact with VGA screen
volatile unsigned int* counter_pio_ptr = NULL;
volatile signed int* ci_init_pio_ptr = NULL;
volatile signed int* cr_init_pio_ptr = NULL;
volatile unsigned int* zoom_ci_pio_ptr = NULL;
volatile unsigned int* zoom_cr_pio_ptr = NULL;
volatile unsigned char* reset_pio_ptr = NULL;
volatile unsigned short* max_iter_pio_ptr = NULL;

// base addresses for respective PIO block
#define COUNTER_PIO 0x00
#define CI_INIT_PIO 0x10
#define CR_INIT_PIO 0x20
#define ZOOM_CI_PIO 0x30
#define ZOOM_CR_PIO 0x40
#define RESET_PIO 0x50
#define ITER_PIO 0x60

// float2fix conversion macro
typedef signed int fix;
#define float2fix(a) (fix)(a * 8388608.0)

pthread_mutex_t lock;

// Global variables for tracking default state
float ci_init_init = 1.0;
float cr_init_init = -2.0;
float zoom_ci_init = 0.0;
float zoom_cr_init = 0.0;

// corners of screen
float ci_init = 1.0;   // top left
float cr_init = -2.0;  // top left
float ci_init2 = 1.0;  // top right
float cr_init2 = 1.0;  // top right
float ci_init3 = -1.0; // bottom right
float cr_init3 = 1.0;  // bottom right
float ci_init4 = -1.0; // bottom left
float cr_init4 = -2.0; // bottom left

// store current zoom level for ci and cr
// increasing this zooms in, decreasing zooms out
unsigned int zoom_ci = 0;
unsigned int zoom_cr = 0;

// store user inputs for init values of ci and cr before actually updating the ci_init and cr_init var
float temp_ci_init = 0.0;
float temp_cr_init = 0.0;

// hold new zoom levels input by user before updating zoom_ci and zoom_cr
float temp_zoom_ci = 0.0;
float temp_zoom_cr = 0.0;

unsigned short max_iter = 1000; // represent max number of iterations 
float render_time;              // track and display render time
 
// flag to indicate when user wants to interact with Mandelbrot Set (upon clicking scroll wheel)
volatile int scroll_flag;

// thread to handle user (keyboard) input
void *handle_user_input(void *arg) {
    int user_int; // store user input

    while(1) {
        if(scroll_flag) { // check if scroll_flag is set
          printf("Enter Specific Coordinates (0), Max Iterations (1), or Reset (2): "); // prompt user to enter choice
          scanf("%d", &user_int);
          
          switch(user_int) { // process user input
              case 0: // allow user to enter new init ci and cr values
                  printf("Enter ci init: ");
                  scanf("%f", &temp_ci_init);
                  printf("Enter cr init: ");
                  scanf("%f", &temp_cr_init);
  
                  // update hardware reg and global var with new ci and cr values 
                  pthread_mutex_lock(&lock);
                  *(ci_init_pio_ptr) = float2fix(temp_ci_init);
                  *(cr_init_pio_ptr) = float2fix(temp_cr_init);
                  ci_init = temp_ci_init;
                  cr_init = temp_cr_init;
                  
                  scroll_flag = 0; // reset flag
                  pthread_mutex_unlock(&lock);
                  break;
              case 1: // allow user to enter new value for max iterations
                  printf("Enter desired max iterations: ");
                  scanf("%d", &max_iter);
                  pthread_mutex_lock(&lock);
                  *(max_iter_pio_ptr) = max_iter;
                  scroll_flag = 0; // reset flag
                  pthread_mutex_unlock(&lock);
                  break;
              case 2: // reset init ci, cr, zoomci, and zoomcr to init values
                  pthread_mutex_lock(&lock);
                  *(ci_init_pio_ptr) = float2fix(ci_init_init);
                  *(cr_init_pio_ptr) = float2fix(cr_init_init);
                  *(zoom_ci_pio_ptr) = float2fix(zoom_ci_init);
                  *(zoom_cr_pio_ptr) = float2fix(zoom_cr_init);
                  *(max_iter_pio_ptr) = max_iter;
                  
                  // reset
                  *(reset_pio_ptr) = 1;
                  *(reset_pio_ptr) = 0;
                   
                  // reset global var
                  ci_init = 1.0;
                  cr_init = -2.0;
                  zoom_ci = 0;
                  zoom_cr = 0;
                  
                  scroll_flag = 0;
                  pthread_mutex_unlock(&lock);
                  break;
              default: // handle invalid options entered by user
                  printf("Invalid option.\n");
                  break;
          }
        } else { // if flag isn't set, clear screen and print the current parameters and render time
          printf("\033[2J\033[H");    // ANSI escape codes to clear the screen and move cursor to top-left corner
          usleep(900); // delay for screen refresh
          render_time = ((float)(*counter_pio_ptr) * 1 / 75000); // calc and display render time onto serial
          printf("zoom level=%d, max iterations=%d, render time=%fms\n", zoom_ci, max_iter, render_time);
          printf("Corners - Top left: (ci=%.2f, cr=%.2f), Top right: (ci=%.2f, cr=%.2f), Bottom right: (ci=%.2f, cr=%.2f), Bottom left: ci=%.2f, cr=%.2f)\n", ci_init, cr_init, ci_init2, cr_init2, ci_init3, cr_init3, ci_init4, cr_init4);
        }
    }
    return NULL;
}

// thread to handle mouse input
void *handle_mouse_input(void *arg) {
    int fd, bytes;         // define file descriptor for mouse and track bytes read
    unsigned char data[4]; // store data read from mouse

    fd = open(MOUSE_DEV, O_RDONLY);
    if (fd == -1) {
        printf("ERROR: Could not open %s. Make sure you have the necessary permissions.\n", MOUSE_DEV);
        return NULL;
    }

    while (1) {
        bytes = read(fd, data, sizeof(data)); // read data from mouse into the buffer

        if (bytes > 0) {
            // extract mouse button states for left and right buttons on mouse
            int left_click = data[0] & 0x1;
            int right_click = data[0] & 0x2;
            
            signed char x_movement = -(signed char)data[1];
            signed char y_movement = -(signed char)data[2];
            int scroll_click = (int)data[0] & 0x4;

            pthread_mutex_lock(&lock);
            
            // update ci_init and cr_init based on mouse movements for panning
            ci_init += (float)y_movement * 0.0005; 
            cr_init += (float)x_movement * 0.0005;
            
            // increment zoom levels on left click (closer view)
            if (left_click) {
                zoom_ci += 1; 
                zoom_cr += 1;
            }

            // decrement zoom levels on right click (wider view)
            // adjust ci_init and cr_init to maintain zoom center
            if (right_click) {
                if (zoom_ci == 0 && zoom_cr == 0){
                    zoom_ci = zoom_ci; 
                    zoom_cr = zoom_cr; 
                }
                else {
                  zoom_ci -= 1; 
                  zoom_cr -= 1; 
                  ci_init += (120.0 * (2.0/480.0)/(zoom_ci+1));
                  cr_init -= (160.0 * (3.0/640.0)/(zoom_cr+1));
                }
            }
            
            // trigger user input handling
            if (scroll_click){
              scroll_flag = 1;
            }
            
            // top right;
            ci_init2 = ci_init;
            cr_init2 = cr_init + (3.0/(zoom_cr+1));
            // bottom right
            ci_init3 = ci_init - (2.0/(zoom_ci+1));
            cr_init3 = cr_init2;
            // bottom left
            ci_init4 = ci_init - (2.0/(zoom_ci+1));
            cr_init4 = cr_init;
            
            // update hardware reg for zoom levels with new zoom values
            *(zoom_ci_pio_ptr) = (zoom_ci);
            *(zoom_cr_pio_ptr) = (zoom_cr);
            
            // adjust ci_init and cr_init if there was a left click to ensure the zoom centers correctly
            if (left_click){
              ci_init -= (120.0 * (2.0/480.0)/zoom_ci);
              cr_init += (160.0 * (3.0/640.0)/zoom_cr);
            }

            // update hardware reg for init ci and cr values with newly calc values
            *(ci_init_pio_ptr) = float2fix(ci_init);
            *(cr_init_pio_ptr) = float2fix(cr_init);
            
            pthread_mutex_unlock(&lock);
        }
    }
    close(fd);
    return NULL;
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
    
    // pointers are assigned by adding defined offset defined above based on memory locations from Platform Designer
    counter_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + COUNTER_PIO);
    ci_init_pio_ptr = (signed int *)(h2p_lw_virtual_base + CI_INIT_PIO);
    cr_init_pio_ptr = (signed int *)(h2p_lw_virtual_base + CR_INIT_PIO);
    zoom_ci_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + ZOOM_CI_PIO);
    zoom_cr_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + ZOOM_CR_PIO);
    max_iter_pio_ptr = (unsigned short *)(h2p_lw_virtual_base + ITER_PIO);
    reset_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + RESET_PIO);
    
    // default values for reset
    *(ci_init_pio_ptr) = float2fix(ci_init_init);
    *(cr_init_pio_ptr) = float2fix(cr_init_init);
    *(zoom_ci_pio_ptr) = float2fix(zoom_ci_init);
    *(zoom_cr_pio_ptr) = float2fix(zoom_cr_init);
    *(max_iter_pio_ptr) = max_iter;
    *(reset_pio_ptr) = 1;
    *(reset_pio_ptr) = 0;
    
    // Initialize mutex
    pthread_mutex_init(&lock, NULL);

    // Start threads
    pthread_t user_input_thread, mouse_input_thread;
    pthread_create(&user_input_thread, NULL, handle_user_input, NULL);
    pthread_create(&mouse_input_thread, NULL, handle_mouse_input, NULL);

    // Wait for threads to finish
    pthread_join(user_input_thread, NULL);
    pthread_join(mouse_input_thread, NULL);

    // Cleanup
    pthread_mutex_destroy(&lock);

    return 0;
}