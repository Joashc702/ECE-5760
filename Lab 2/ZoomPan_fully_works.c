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


typedef signed int fix;
#define float2fix(a) (fix)(a * 8388608.0)

pthread_mutex_t lock;

// Global variables for demonstration purposes
float ci_init_init = 1.0;
float cr_init_init = -2.0;
float zoom_ci_init = 0.0;
float zoom_cr_init = 0.0;

float ci_init = 1.0;
float cr_init = -2.0;
unsigned int zoom_ci = 0;
unsigned int zoom_cr = 0;

void *handle_user_input(void *arg) {
    float temp_ci_init = 1.0;
    float temp_cr_init = -2.0;
    float temp_zoom_ci = 0.0;
    float temp_zoom_cr = 0.0;
    int user_int;

    while(1) {
        printf("Panning (0), Zooming (1), Reset (2): ");
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
                pthread_mutex_unlock(&lock);
                break;
            case 1:
                printf("Enter zoom level for ci: ");
                scanf("%f", &temp_zoom_ci);
                printf("Enter zoom level for cr: ");
                scanf("%f", &temp_zoom_cr);

                pthread_mutex_lock(&lock);
                *(ci_init_pio_ptr) = float2fix(temp_ci_init);
                *(cr_init_pio_ptr) = float2fix(temp_cr_init);
                *(zoom_ci_pio_ptr) = (int)temp_zoom_ci;
                *(zoom_cr_pio_ptr) = (int)temp_zoom_cr;
                pthread_mutex_unlock(&lock);
                break;
            case 2:
                pthread_mutex_lock(&lock);
                *(ci_init_pio_ptr) = float2fix(ci_init_init);
                *(cr_init_pio_ptr) = float2fix(cr_init_init);
                *(zoom_ci_pio_ptr) = float2fix(zoom_ci_init);
                *(zoom_cr_pio_ptr) = float2fix(zoom_cr_init);
                *(reset_pio_ptr) = 1;
                *(reset_pio_ptr) = 0;
                pthread_mutex_unlock(&lock);
                break;
            default:
                printf("Invalid option.\n");
                break;
        }
    }

    return NULL;
}

void *handle_mouse_input(void *arg) {
    int fd, bytes;
    unsigned char data[3];

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

            pthread_mutex_lock(&lock);
            ci_init += (float)y_movement * 0.0001; 
            cr_init += (float)x_movement * 0.0001;
            *(ci_init_pio_ptr) = float2fix(ci_init + ci_init);
            *(cr_init_pio_ptr) = float2fix(cr_init + cr_init);
            
            if (left_click) {
                zoom_ci += 1; 
                zoom_cr += 1; 
            }

            if (right_click) {
                if (zoom_ci == 0 && zoom_cr == 0){
                    zoom_ci = zoom_ci; 
                    zoom_cr = zoom_cr; 
                }
                else {
                  zoom_ci -= 1; 
                  zoom_cr -= 1; 
                }
            }
            *(zoom_ci_pio_ptr) = (zoom_ci);
            *(zoom_cr_pio_ptr) = (zoom_cr);
            printf("Updated FPGA: ci_init=%.2f, cr_init=%.2f, zoom_ci=%d, zoom_cr=%d\n", ci_init, cr_init, zoom_ci, zoom_cr);
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