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
#include <pthread.h>
#define MOUSE_DEV "/dev/input/mice"

#define HW_REGS_BASE 0xff200000
#define HW_REGS_SPAN 0x00005000 

void *h2p_lw_virtual_base;
int fd;

volatile unsigned int* counter_pio_ptr = NULL;
#define COUNTER_PIO 0x00
volatile signed int* ci_init_pio_ptr = NULL;
#define CI_INIT_PIO 0x10
volatile signed int* cr_init_pio_ptr = NULL;
#define CR_INIT_PIO 0x20
volatile unsigned int* zoom_ci_pio_ptr = NULL;
#define ZOOM_CI_PIO 0x30
volatile unsigned int* zoom_cr_pio_ptr = NULL;
#define ZOOM_CR_PIO 0x40
volatile unsigned char* reset_pio_ptr = NULL;
#define RESET_PIO 0x50
volatile unsigned short* max_iter_pio_ptr = NULL;
#define ITER_PIO 0x60

typedef signed int fix;
#define float2fix(a) (fix)(a * 8388608.0)

pthread_mutex_t lock;

// Global variables for demonstration purposes
float ci_init_init = 1.0;
float cr_init_init = -2.0;
float zoom_ci_init = 0.0;
float zoom_cr_init = 0.0;

// corners of screen
float ci_init = 1.0;  // top left
float cr_init = -2.0; // top left
float ci_init2 = 1.0  ; // top right
float cr_init2 = 1.0  ; // top right
float ci_init3 = -1.0  ; // bottom right
float cr_init3 = 1.0  ; // bottom right
float ci_init4 = -1.0  ; // bottom left
float cr_init4 = -2.0  ; // bottom left

unsigned int zoom_ci = 0;
unsigned int zoom_cr = 0;

float temp_ci_init = 0.0;
float temp_cr_init = 0.0;
float temp_zoom_ci = 0.0;
float temp_zoom_cr = 0.0;
unsigned short max_iter = 1000;
volatile int scroll_flag;

float render_time;

void *handle_user_input(void *arg) {
    
    int user_int;

    while(1) {
        if(scroll_flag){
          printf("Enter Specific Coordinates (0), Max Iterations (1), or Reset (2): ");
          scanf("%d", &user_int);
          
          switch(user_int) {
              case 0:
                  printf("Enter ci init: ");
                  scanf("%f", &temp_ci_init);
                  printf("Enter cr init: ");
                  scanf("%f", &temp_cr_init);
  
                  pthread_mutex_lock(&lock);
                  *(ci_init_pio_ptr) = float2fix(temp_ci_init);
                  *(cr_init_pio_ptr) = float2fix(temp_cr_init);
                  *(zoom_ci_pio_ptr) = float2fix(temp_zoom_ci);
                  *(zoom_cr_pio_ptr) = float2fix(temp_zoom_cr);
                  scroll_flag = 0;
                  pthread_mutex_unlock(&lock);
                  break;
              case 1:
                  printf("Enter desired max iterations: ");
                  scanf("%d", &max_iter);
                  pthread_mutex_lock(&lock);
                  *(max_iter_pio_ptr) = max_iter;
                  scroll_flag = 0;
                  pthread_mutex_unlock(&lock);
                  break;
              case 2:
                  pthread_mutex_lock(&lock);
                  *(ci_init_pio_ptr) = float2fix(ci_init_init);
                  *(cr_init_pio_ptr) = float2fix(cr_init_init);
                  *(zoom_ci_pio_ptr) = float2fix(zoom_ci_init);
                  *(zoom_cr_pio_ptr) = float2fix(zoom_cr_init);
                  *(max_iter_pio_ptr) = max_iter;
                  *(reset_pio_ptr) = 1;
                  *(reset_pio_ptr) = 0;
                   
                  ci_init = 1.0;
                  cr_init = -2.0;
                  zoom_ci = 0;
                  zoom_cr = 0;
                  
                  scroll_flag = 0;
                  pthread_mutex_unlock(&lock);
                  break;
              default:
                  printf("Invalid option.\n");
                  break;
          }
        } else {
          render_time = ((float)(*counter_pio_ptr) * 1 );
        
         // printf("\033[2J\033[H");    // clear serial screen and move the cursor to top left corner
          printf("zoom level=%d, max iterations=%d, render time=%f\n", zoom_ci, max_iter, render_time);
          printf("Corners: (top left ci=%.2f, top left cr=%.2f), (top right ci=%.2f, top right cr=%.2f), (bottom right ci=%.2f, bottom right cr=%.2f), (bottom left ci=%.2f, bottom left cr=%.2f)\n", ci_init, cr_init, ci_init2, cr_init2, ci_init3, cr_init3, ci_init4, cr_init4);
        }
    }

    return NULL;
}

void *handle_mouse_input(void *arg) {
    int fd, bytes;
    unsigned char data[4];

    fd = open(MOUSE_DEV, O_RDONLY);
    if (fd == -1) {
        printf("ERROR: Could not open %s. Make sure you have the necessary permissions.\n", MOUSE_DEV);
        return NULL;
    }

    while (1) {
        bytes = read(fd, data, sizeof(data));

        if (bytes > 0) {
            int left_click = data[0] & 0x1;
            int right_click = data[0] & 0x2;
            signed char x_movement = -(signed char)data[1];
            signed char y_movement = -(signed char)data[2];
            int scroll_click = (int)data[0] & 0x4;

            pthread_mutex_lock(&lock);
            
            ci_init += (float)y_movement * 0.0005; 
            cr_init += (float)x_movement * 0.0005;
            
            if (left_click) {
                zoom_ci += 1; 
                zoom_cr += 1;
                
                ci_init -= (120.0 * (2.0/480.0)/zoom_ci);
                cr_init += (160.0 * (3.0/640.0)/zoom_cr);
            }

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
            
            *(zoom_ci_pio_ptr) = (zoom_ci);
            *(zoom_cr_pio_ptr) = (zoom_cr);
            *(ci_init_pio_ptr) = float2fix(ci_init);
            *(cr_init_pio_ptr) = float2fix(cr_init);
            //printf("Updated FPGA: ci_init=%.2f, cr_init=%.2f, zoom_ci=%d, zoom_cr=%d, temp_ci_val=%f\n", ci_init, cr_init, zoom_ci, zoom_cr, temp_ci_init);
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
    counter_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + COUNTER_PIO);
   // past_time = ((float)(*counter_pio_ptr) * 1 );
    
    ci_init_pio_ptr = (signed int *)(h2p_lw_virtual_base + CI_INIT_PIO);
    cr_init_pio_ptr = (signed int *)(h2p_lw_virtual_base + CR_INIT_PIO);
    zoom_ci_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + ZOOM_CI_PIO);
    zoom_cr_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + ZOOM_CR_PIO);
    max_iter_pio_ptr = (unsigned short *)(h2p_lw_virtual_base + ITER_PIO);
    reset_pio_ptr = (unsigned int *)(h2p_lw_virtual_base + RESET_PIO);
    
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