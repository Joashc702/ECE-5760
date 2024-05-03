///////////////////////////////////////
/// Blackjack
/// compile with
/// gcc blackjack.c -o bt -lm -std=c99
///////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
// interprocess comm
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <time.h>
// network stuff
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <netdb.h> 

//#include "address_map_arm_brl4.h"

void *h2p_lw_virtual_base;

// RAM fp buffer
volatile unsigned int * fp_ram_ptr = NULL;
volatile unsigned int * fp_ram_ptr_seed = NULL;
void *fp_ram_virtual_base;
// file id
int fd;

// pixel buffer
volatile unsigned int * vga_pixel_ptr = NULL ;
void *vga_pixel_virtual_base;

// character buffer
volatile unsigned int * vga_char_ptr = NULL ;
void *vga_char_virtual_base;

// shared memory 
key_t mem_key=0xf0;
int shared_mem_id; 
int *shared_ptr;
int shared_time;
int shared_note;
char shared_str[64];

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

// 16-bit color
//#define rgb(r,g,b) ((((r)&7)<<5) | (((g)&7)<<2) | (((b)&3)))
#define red  (0+(0<<5)+(31<<11))
#define dark_red (0+(0<<5)+(15<<11))
#define green (0+(63<<5)+(0<<11))
#define dark_green ((5 << 11) + (28 << 5) + 0)
#define blue (31+(0<<5)+(0<<11))
#define dark_blue (15+(0<<5)+(0<<11))
#define yellow (0+(63<<5)+(31<<11))
#define cyan (31+(63<<5)+(0<<11))
#define magenta (31+(0<<5)+(31<<11))
#define black (0x0000)
#define gray (15+(31<<5)+(51<<11))
#define white (0xffff)
#define orange (0 + (31 << 5) + (31<<11))
#define purple ((19 << 11) + (0 << 5) + 31)
#define gold ((29 << 11) + (50 << 5) + 0)
#define brown ((15 << 11) + (15 << 5) + 7)
#define sky_blue ((12 << 11) | (52 << 5) | 30)

int colors[] = {red, dark_red, green, dark_green, blue, dark_blue, 
		yellow, cyan, magenta, gray, black, white, orange, purple, gold, brown};

// pixel macro
#define VGA_PIXEL(x,y,color) do{\
	int  *pixel_ptr ;\
	pixel_ptr = (int*)((char *)vga_pixel_ptr + (((y)*640+(x))<<1)) ; \
	*(short *)pixel_ptr = (color);\
} while(0)

// pointers
volatile unsigned char* player_init_hand_ptr = NULL;
volatile unsigned char* dealer_top_ptr = NULL;
volatile unsigned char* init_done_ptr = NULL;
volatile unsigned char* shared_write_ptr = NULL;
volatile unsigned char* read_addr_ptr = NULL;
volatile unsigned int* output_random_ptr = NULL;

volatile unsigned char* draw_dealer_1_ptr = NULL;
volatile unsigned char* draw_dealer_2_ptr = NULL;
volatile unsigned char* draw_dealer_3_ptr = NULL;
volatile unsigned char* draw_player_1 = NULL;
volatile unsigned char* draw_player_2 = NULL;
volatile unsigned char* draw_player_3 = NULL;
volatile unsigned char* test_3_ptr = NULL;

// base addr for each PIO block
#define PLAYER_INIT_HAND_PIO 0x00
#define DEALER_TOP_PIO 0x10
#define INIT_DONE_PIO 0x20
#define SHARED_WRITE_PIO 0x30
#define READ_ADDR_PIO 0x40
#define OUTPUT_RANDOM_PIO 0x50

#define DRAW_DEALER_1_PIO 0x70
#define DRAW_DEALER_2_PIO 0x80
#define DRAW_DEALER_3_PIO 0x90
#define DRAW_PLAYER_1_PIO 0x100
#define DRAW_PLAYER_2_PIO 0x110
#define DRAW_PLAYER_3_PIO 0x120
#define TEST_3_PIO 0x130

/* Memory */
#define DDR_BASE              0x00000000
#define DDR_END               0x3FFFFFFF
#define A9_ONCHIP_BASE        0xFFFF0000
#define A9_ONCHIP_END         0xFFFFFFFF
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
#define SDRAM_SPAN			  0x04000000
//
#define FPGA_ONCHIP_BASE      0xC4000000
#define FPGA_ONCHIP_END       0xC500270F
// modified for 640x480
// #define FPGA_ONCHIP_SPAN      0x00040000
#define FPGA_ONCHIP_SPAN      0x02000000
//
#define FPGA_CHAR_BASE        0xC9000000 
#define FPGA_CHAR_END         0xC9001FFF
#define FPGA_CHAR_SPAN        0x00002000

/* Cyclone V FPGA devices */
#define HW_REGS_BASE          0xff200000
//#define HW_REGS_SPAN        0x00200000 
#define HW_REGS_SPAN          0x00005000 

// temp var for init done and shared write
unsigned int init_done_temp = 0;
unsigned int shared_write_temp = 0;

// fixed point
typedef signed int fix;
#define float2fix30(a) ((int)((a)*1073741824)) // 2^30

// sram pointer with base addr
//volatile unsigned int* sram_ptr = NULL;
//#define SRAM_ARR 0x4000000
#define SEED_SRAM_ARR 0x1000000
//#define SDRAM_OFFSET 0x04000000

#define DECK_SIZE 52 // 3 decks of cards, 52 for each
#define FACE_VALUES 13
#define CHIP_TYPES 6

//Simulation variables
#define NUM_SIMUL 100.0 //should be 5, 10, 15

// VGA functions
/* function prototypes */
void draw_card(int, int, char[], char[]);
void display_card(int, int, char[], char[], int);

// Card structure
typedef struct {
    char *face;
    char *suit;
    int value;
} Card;

// Deck global variable
Card deck[DECK_SIZE];
char *faces[] = {"Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace"};
char *suits[] = {"Hearts", "Diamonds", "Clubs", "Spades"};
int values[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 11};

// TODO: make sure three 10s show Jack, Queen, King, ACE instead of 11
char *char_vals[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"}; 

int chips[] = {1, 3, 5, 10, 25, 100};
int playerChips = 1000; // Starting chips

int num_cards_players[4] = {0, 0, 0, 0};

// Function prototypes
void fillDeck();
void shuffleDeck();
int dealCard(int *playerTotal, int *deckPosition, int *aceCount, bool firstDealt);
void playBlackjack(int* playerChips, int numberOfPlayers); 

// Initialize player chips
void initializePlayerChips(int* playerChips, int numberOfPlayers) {
    for (int i = 0; i < numberOfPlayers; i++) {
        playerChips[i] = 1000; // Starting chips for each player
    }
}

// Invalid option always shows up, so need to clear the input first before next input read
void clearInputBuffer() {
    // Clear the input buffer to remove any remaining characters
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

/*
int main() {
    srand(time(NULL)); // Seed random number generator

    int numberOfPlayers = 0;
    int* playerChips = NULL;
    char choice = 'r'; // Default to 'r'

    do {
        if (choice == 'r') {
            if (playerChips != NULL) {
                free(playerChips); // Free previous allocation if any
            }

            printf("Enter the number of players: ");
            scanf("%d", &numberOfPlayers);
            clearInputBuffer();
            // Dynamically allocate memory for each player's chips
            playerChips = malloc(numberOfPlayers * sizeof(int));
            if (!playerChips) {
                printf("Failed to allocate memory for player chips.\n");
                return 1; // Exit if allocation fails
            }

            // Initialize player chips at the start of each new game setup
            initializePlayerChips(playerChips, numberOfPlayers);
        }

        // Play the game
        playBlackjack(playerChips, numberOfPlayers);

        // Ask if users want to play again, keep playing with the same setup, or restart
        printf("\nDo you want to play again with the same players (y), restart with new players (r), or quit (q)? ");
        scanf(" %c", &choice);
        clearInputBuffer();

    } while (choice == 'y' || choice == 'r'); // Continue if users want to play again or restart

    printf("Thanks for playing!\n");

    if (playerChips != NULL) {
        free(playerChips); // Cleanup at the end
    }
    return 0;
}
*/

void fillDeck() {
    for (int i = 0; i < DECK_SIZE; i++) {
        deck[i].face = faces[i % FACE_VALUES];
        deck[i].suit = suits[(i / FACE_VALUES) % 4]; // cycle through suits every 13 cards, repeating every 4 suits for 3 decks
        deck[i].value = values[i % FACE_VALUES];
    }
}

void shuffleDeck() {
    for (int i = 0; i < DECK_SIZE; i++) {
        int j = rand() % DECK_SIZE;
        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

void printCard(const Card *c) {
    if (c->value == 11){
        printf("%s of %s\n", c->face, c->suit);
    }
    else{
        printf("%s of %s, %d\n", c->face, c->suit, c->value);
    }
}

int dealCard(int *playerTotal, int *deckPosition, int *aceCount, bool firstDealt) {
    Card dealtCard = deck[*deckPosition];
    if (firstDealt){
        if (strcmp(dealtCard.face, "Ace") == 0) {  // If the card is an Ace and it's the player's turn
            (*aceCount)++;
        }
    }
    // else{
    //     *playerTotal += dealtCard.value;
    //     if (dealtCard.value == 11 && *playerTotal > 21) {
    //         *playerTotal -= 10;
    //     }
    // }

    (*deckPosition)++;
    return dealtCard.value;

    // Card dealtCard = deck[*deckPosition];

    // if (strcmp(dealtCard.face, "Ace") == 0) {  // If the card is an Ace and it's the player's turn
    //     (*aceCount)++;
    // }
        // int aceValueChoice = 11;  // Default to 11

        // printf("An Ace! Choose its value (1 or 11): ");
        // scanf("%d", &aceValueChoice);
        // clearInputBuffer();

        // Validate the player's choice for Ace's value
    //     if (aceValueChoice != 1) {
    //         aceValueChoice = 11;  // Default to 11 if the input is invalid
    //     }

    //     *playerTotal += aceValueChoice;
    //     if (aceValueChoice == 11) {
    //         (*aceCount)++;
    //     }
    // } else {
    //     *playerTotal += dealtCard.value;
    //     if (dealtCard.value == 11) {  // Check if the dealt card is an Ace
    //         (*aceCount)++;
    //     }
    // }

    // Adjust ace value if total exceeds 21
    // while (*aceCount > 0 && *playerTotal > 21) {
    //     *playerTotal -= 10;
    //     (*aceCount)--;
    // }

    // (*deckPosition)++;
    // return dealtCard.value;
}

void playBlackjack(int* playerChips, int numberOfPlayers) {
    // Flag
    char* blackjack = malloc(numberOfPlayers * sizeof(char));
    // Memory allocation for player data
    int* playerTotals = malloc(numberOfPlayers * sizeof(int));
    int* playerAceCounts = malloc(numberOfPlayers * sizeof(int));
    int* bets = malloc(numberOfPlayers * sizeof(int));
    // char* firstDealtCard = malloc(numberOfPlayers * sizeof(char));

    if (!playerTotals || !playerAceCounts || !bets) {
        printf("Failed to allocate memory for players.\n");
        free(playerTotals);
        free(playerAceCounts);
        free(bets);
        free(blackjack);
        // free(firstDealtCard);
        return;
    }

    // Initialize player totals, ace counts to zero, and fill/shuffle deck
    for (int i = 0; i < numberOfPlayers; i++) {
        playerTotals[i] = 0;
        playerAceCounts[i] = 0;
        bets[i] = 0;
        blackjack[i] = 0;
        // firstDealtCard[i]=0;
    }

    fillDeck();
    // Shuffle many times for better randomness
    shuffleDeck();
    shuffleDeck();
    shuffleDeck();
    shuffleDeck();
    shuffleDeck();

    ///////////////////
    // Betting phase --> TODO allow player to bet and display
    //////////////////
    for (int i = 0; i < numberOfPlayers; i++) {
        printf("\nPlayer %d, you have %d chips. How much do you want to bet? ", i + 1, playerChips[i]);
        scanf("%d", &bets[i]);
        clearInputBuffer();
        if (bets[i] > playerChips[i]) {
            printf("You cannot bet more than your chip count. Betting all your chips: %d\n", playerChips[i]);
            bets[i] = playerChips[i];
        }
        if(bets[i] <=0){
            printf("The minimum bet is $1. Betting all your chips: %d\n", playerChips[i]);
            bets[i] = playerChips[i];
        }
        playerChips[i] -= bets[i];
        printf("\nPlayer %d, after placing your bet, you have %d chips left.\n\n", i + 1, playerChips[i]);
    }

    // Dealing initial two cards to each player
    // int picked_cards[numberOfPlayers+1];
    int dealerTotal = 0, dealerAceCount = 0, deckPosition = 0;
    for (int i = 0; i < numberOfPlayers; i++) {
        int temp = 0;
        int aceValue = 11;
        printf("Player %d, Your cards:\n",i + 1);
        temp += dealCard(&playerTotals[i], &deckPosition, &playerAceCounts[i], 1);
        printCard(&deck[deckPosition - 1]);
        //char curr_card[10];
        //sprintf(curr_card, "%d", deck[deckPosition - 1].value);
        //printf(curr_card);
        display_card(i, deck[deckPosition - 1].value, deck[deckPosition - 1].face, deck[deckPosition - 1].suit, num_cards_players[i]);
        num_cards_players[i]++;
        
        //picked_cards[i] = deckPosition - 1;
        temp += dealCard(&playerTotals[i], &deckPosition, &playerAceCounts[i], 1);
        //sprintf(curr_card, "%d", deck[deckPosition - 1].value);
        display_card(i, deck[deckPosition - 1].value, deck[deckPosition - 1].face, deck[deckPosition - 1].suit, num_cards_players[i]);
        num_cards_players[i]++;
        printCard(&deck[deckPosition - 1]);
        
        // TODO Ace (1 or 11)
        
        //picked_cards[i] = deckPosition - 1;
        if(playerAceCounts[i] > 0){
            printf("You got an Ace! Choose its value (1 or 11): ");
            scanf("%d", &aceValue);   
            clearInputBuffer();
            if(aceValue == 1){
                playerTotals[i] = temp - 10;
            }
            else{
                playerTotals[i] = temp;
            }
        }
        else{
            playerTotals[i] = temp;
        }

        // if(playerAceCounts[i] > 0 ) {
        //     printf("\nPlayer %d, you have an Ace, would you want it to be 1 or 11 ?", i + 1);
        // }

        if(playerTotals[i] == 21){
            blackjack[i] = 1;
            printf("\nPlayer %d, you hit BlackJack! ", i + 1);
        }
        printf("Player %d's total: %d\n\n", i + 1, playerTotals[i]);
    }
    // printf("\nDealer's card:\n");
    // Dealing two cards to the dealer, but only showing the first one
    int dealerTemp = 0;
    int dealerBJ = 0;
    dealerTotal += dealCard(&dealerTotal, &deckPosition, &dealerAceCount, 1);
    //picked_cards[i] = deckPosition - 1;
    printf("Dealer's shown card: ");
    printCard(&deck[deckPosition - 1]);
    printf("\n");
    

    *(player_init_hand_ptr) = playerTotals[0]; // TODO handle multiple players init hands
    *(dealer_top_ptr) = dealerTotal;
    
    printf("Init done %d\n", (int)(*init_done_ptr));
    //printf("Shared Write Done Signal 1st check %d\n", (int)(*shared_write_ptr));
    
     for (int i = 0; i < 2000; i++) { // filling memory
      *(fp_ram_ptr_seed + i) = (rand() % DECK_SIZE);
    }
    
    //for (int i = 0; i < 2000; i++) { // filling memory
    //  printf("SEED RAM addr=%d contents=%d \n\r", i, *(fp_ram_ptr_seed+i));
    //}
        
    //printf(DECK_SIZE);
    for (int i = (numberOfPlayers*2+1); i < DECK_SIZE; i++) { // filling memory
      *(fp_ram_ptr + i) = deck[i].value;
    }
    
    *init_done_ptr = 1; // set init done to 1 to indicate memory is full
    /*
    for (int i = (numberOfPlayers*2+1); i < DECK_SIZE; i++) { // filling memory
      printf("RAM addr=%d contents=%d \n\r", i, *(fp_ram_ptr+i));
    }*/
  
    //usleep(5000000); // delay before shared write
    int sampl_counter = 0;
    usleep(5000);
    while (sampl_counter <= 50000) {
      //printf("Output ran: %d, Dealer: %d, Player: %d\n", (int)(*shared_write_ptr), (int)(*read_addr_ptr), (int)(*output_random_ptr));
      //printf("Dealer draw 1: %d, Dealer draw 2: %d, Dealer draw 3: %d\n", (int)(*draw_dealer_1_ptr), (int)(*draw_dealer_2_ptr), (int)(    *draw_dealer_3_ptr));
      //printf("Player draw 1: %d, Player draw 2: %d, Player draw 3: %d\n", (int)(*draw_player_1), (int)(*draw_player_2), (int)(    *draw_player_3));   
      //printf("Result: %d\n", (int)( *test_3_ptr)); 
      
      printf("drum state: %d, output random: %d\n", (int)(*read_addr_ptr), (int)(*output_random_ptr));
      printf("RNG simulation 1: %d, RNG simulation 2: %d, RNG simulation 3: %d\n", (int)(*draw_dealer_1_ptr), (int)(*draw_dealer_2_ptr), (int)(    *draw_dealer_3_ptr));
      printf("RNG simulation 4: %d, RNG simulation 5: %d\n", (int)(*draw_player_1), (int)(*draw_player_2));   
      printf("lfsr test 7: %d\n", (int)( *test_3_ptr));   
      printf("card itr: %d\n", (int)(*shared_write_ptr));
      sampl_counter++;
      
    }
    
    //printf("Init done %d\n", (int)(*init_done_ptr));
    printf("Shared Write Done Signal 2nd check %d\n", (int)(*shared_write_ptr));

    dealerTotal += dealCard(&dealerTotal, &deckPosition, &dealerAceCount, 1);

    if(dealerAceCount > 0 && dealerTotal == 21){
        printf("Dealer hits BlackJack!\n");
        dealerBJ++;
    }
    if (!dealerBJ){
        // Player decision phase
        for (int i = 0; i < numberOfPlayers; i++) {
            while (true) {
                if (blackjack[i] != 1){
                    float prob_win = ((*draw_dealer_3_ptr)/NUM_SIMUL) * 100;
                    char prob_win_str[15];
                    sprintf(prob_win_str, "%.2f%%",prob_win);
                    printf(prob_win_str); 
                    VGA_text(29,22,prob_win_str);
                    printf("\nDealt card: ");
                    printCard(&deck[deckPosition - 1]);
                    printf("wins %d\n", (*draw_dealer_3_ptr));
                    
                    float prob_tie = ((*draw_player_1)/NUM_SIMUL) * 100;
                    char prob_tie_str[15];
                    sprintf(prob_tie_str, "%.2f%%",prob_tie);
                    printf(prob_tie_str); 
                    VGA_text(43,22,prob_tie_str);
                    printf("\nDealt card: ");
                    printCard(&deck[deckPosition - 1]);
                    printf("ties %d\n", (*draw_player_1));        
                    
                    printf("\nPlayer %d, your total is %d. Hit (h) or stand (s)? ", i + 1, playerTotals[i]);
                    char choice;
                    scanf("%c", &choice);
                    clearInputBuffer();
                    if (choice == 'h') {
                       
                        playerTotals[i] += dealCard(&playerTotals[i], &deckPosition, &playerAceCounts[i], 0);
                        //*init_done_ptr = 0;
                        //*(player_init_hand_ptr) = playerTotals[i];
                        //*init_done_ptr = 1;
                
                        // graphics for hitting card
                        //char curr_card[10];
                        //sprintf(curr_card, "%d", deck[deckPosition - 1].value);
                        display_card(i, deck[deckPosition - 1].value, deck[deckPosition - 1].face, deck[deckPosition - 1].suit, num_cards_players[i]);
                        num_cards_players[i]++;

                        // TODO show on the VGA to tell you bust instead of print function
                        if (playerTotals[i] > 21 && deck[deckPosition - 1].value != 11) {
                            printf("Player %d total is now %d. You busted!\n", i + 1, playerTotals[i]);
                            break;
                        }
                        else if (playerTotals[i] > 21 && deck[deckPosition - 1].value == 11){
                            playerTotals[i] -= 10;
                        }
                        
                        
                    } 
                    else if (choice == 's') {
                        printf("Player %d, chose to stand. Total is: %d\n", i + 1, playerTotals[i]);
                        break;
                    } else {
                        printf("Invalid Option!\n");
                    }
                }
                else{
                    break;
                }
            }
        }

        while (dealerTotal < 17) {
            printf("\nDealer's now: %d\n", dealerTotal);
            printf("\nDealer draws: ");
            dealerTotal += dealCard(&dealerTotal, &deckPosition, &dealerAceCount, 0);
            printCard(&deck[deckPosition - 1]);
        }
        printf("\nDealer's final total: %d\n", dealerTotal);
        if(dealerTotal > 21){
            printf("Dealer busted! \n");
        }
    }

    // Who wins
    for (int i = 0; i < numberOfPlayers; i++) {
        if(blackjack[i] == 1 && !dealerBJ){
            printf("\nPlayer %d hits BlackJack and now has %d chips.\n", i + 1, playerChips[i] + bets[i] * 3);
            playerChips[i] += bets[i] * 3;
        }
        if (playerTotals[i] > 21 || (playerTotals[i] <= 21 && dealerTotal <= 21 && dealerTotal > playerTotals[i])) {
            printf("\nPlayer %d loses their bet of %d chips.", i + 1, bets[i]);
            printf(" Player %d chip count is now %d.\n", i + 1, playerChips[i]);
        } else if ((dealerTotal > 21 && playerTotals[i] <= 21) || (playerTotals[i]<=21 && playerTotals[i] > dealerTotal)) {
            printf("\nPlayer %d wins and now has %d chips.\n", i + 1, playerChips[i] + bets[i] * 2);
            playerChips[i] += bets[i] * 2;
        } else {
            printf("\nPlayer %d ties and keeps their bet of %d chips.\n", i + 1, bets[i]);
            playerChips[i] += bets[i];
        }
    }

    // Cleanup
    free(playerTotals);
    free(playerAceCounts);
    free(bets);
    free(blackjack);
}

//////////////////////////////////////////
// GRAPHICS
//////////////////////////////////////////
/*  // dynamic player 1 deck
  int num_of_cards = 7;
  int box_width = 30;
  int box_height = 45;
  int card_spacing_x = 16;  
  int card_spacing_y = 16;  
  int box_x, box_y;
  int value_x1, value_y1, value_x2, value_y2, value_x3, value_y3, value_x4, value_y4;

  // iterate based on num_of_cards
  char *card_values[] = {"A", "5", "K", "2", "J", "A", "5", "K"};
  char card_suits[] = {'h', 'd', 's', 'c', 'h', 'h', 'd', 's'};

  int start_x1 = 18;
  int start_y1 = 215;
  
  for (int i = 0; i < num_of_cards; i++) {
      box_x = start_x1 + (i * card_spacing_x);
      box_y = start_y1 + (i * card_spacing_y);

      // Draw card box
      VGA_box(box_x, box_y, box_x + box_width, box_y + box_height, red_color);

      value_x1 = 20;
      value_y1 = 44;
      // Draw card value and suit
      draw_card(value_x1 + (2*i), value_y1 + 2*i, card_values[i], card_suits[i]);
}*/
void display_card(int player, int val, char face[], char suit[], int ind){
    int box_width = 30;
    int box_height = 45;
    int card_spacing_x = 16;  
    int card_spacing_y = 16;  
    int box_x, box_y, value_x1, value_y1;
    box_x = 18 + (player*160) + (ind * card_spacing_x);
    box_y = 215 + (ind * card_spacing_y);

    // Draw card box
    VGA_box(box_x, box_y, box_x + box_width, box_y + box_height, red);

    value_x1 = 20 + (player*20);
    value_y1 = 44;
    // Draw card value and suit
    char value[10];
    switch (val) {
      case 2:
        strcpy(value, "2");
        break;
      case 3:
        strcpy(value, "3");
        break;
      case 4:
        strcpy(value, "4");
        break;
      case 5:
        strcpy(value, "5");
        break;
      case 6:
        strcpy(value, "6");
        break;
      case 7:
        strcpy(value, "7");
        break;
      case 8:
        strcpy(value, "8");
        break;
      case 9:
        strcpy(value, "9");
        break;
      case 10:
        if (face == "Ten"){
          strcpy(value, "10");
        }else if (face == "King"){
          strcpy(value, "K");
        }else if (face == "Queen"){
          strcpy(value, "Q");
        }else if (face == "Jack"){
          strcpy(value, "J");
        }
        break;
      case 1:
        strcpy(value, "A");
        break;
      case 11:
        strcpy(value, "A");
        break;
      default:
        strcpy(value, "?");
        break;
    }
    draw_card(value_x1 + (2*ind), value_y1 + 2*ind, value, suit);
}

/****************************************************************************************
 * Draw a playing card with suit symbols
****************************************************************************************/
void draw_card(int x, int y, char value[], char suit[]) {
  int card_width = 30;
  int card_height = 44;
  short white_color = 0xFF;
  
  // draw card background
  //VGA_box(x, y, x + card_width, y + card_height, red_color);
  
  int heart = 3;
  int diamond = 4;
  int club = 5;
  int spades = 6;
  char heart_shape, diamond_shape, club_shape, spades_shape;
  heart_shape = (char) heart;
  diamond_shape = (char) diamond;
  club_shape = (char) club;
  spades_shape = (char) spades;
  
  // convert suit char to symbol
  static char suit_symbol[2];
  /*
  switch(suit) {
    case "Hearts":
      *(suit_symbol) = (int)heart_shape;
      break;
    case "Diamonds":
      *(suit_symbol) = (int) diamond_shape;  
      break;
    case "Clubs":
      *(suit_symbol) = (int) club_shape; 
      break;
    case "Spades":
      *(suit_symbol) = (int) spades_shape;
      break;
    default:
      *(suit_symbol) = '?';      
  }*/
  
  if (suit == "Hearts") {
    *(suit_symbol) = (int)heart_shape;
  }else if (suit == "Diamonds"){
    *(suit_symbol) = (int) diamond_shape;  
  }else if (suit == "Clubs"){
    *(suit_symbol) = (int) club_shape; 
  }else if (suit == "Spades"){
    *(suit_symbol) = (int) spades_shape;
  }else{
    *(suit_symbol) = '?';     
  }
  
  //////////////////////////////////////////////

  int centerX = x + card_width / 2;
  int centerY = y + card_height / 2;

  int charWidth = 2;
  int charHeight = 2;

  int valueStartX = centerX - charWidth / 2;
  int valueStartY = centerY - (charHeight); 
  int suitStartX = centerX - charWidth / 2;
  int suitStartY = centerY - charHeight / 2;

  // Draw the value and suit within the box
  VGA_text(x - 17, y - 14, value);
  VGA_text(x - 17, y - 14 - 2, &(suit_symbol[0]));
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

void graphics_main(){
  //short background_color = rgb(0,4,3);
	// clear the screen
	VGA_box (0, 0, 639, 479, dark_green);
	// clear the text
	VGA_text_clear();
 
  // Title
  VGA_text(5,5,"BLACKJACK");
  
  // dynamic player 1 deck
  int num_of_cards = 7;
  int box_width = 30;
  int box_height = 45;
  int card_spacing_x = 16;  
  int card_spacing_y = 16;  
  int box_x, box_y;
  int value_x1, value_y1, value_x2, value_y2, value_x3, value_y3, value_x4, value_y4;

  // iterate based on num_of_cards
  char *card_values[] = {"A", "5", "K", "2", "J", "A", "5", "K"};
  char card_suits[] = {'h', 'd', 's', 'c', 'h', 'h', 'd', 's'};

  int start_x1 = 18;
  int start_y1 = 215;
  /*
  for (int i = 0; i < num_of_cards; i++) {
      box_x = start_x1 + (i * card_spacing_x);
      box_y = start_y1 + (i * card_spacing_y);

      // Draw card box
      VGA_box(box_x, box_y, box_x + box_width, box_y + box_height, red_color);

      value_x1 = 20;
      value_y1 = 44;
      // Draw card value and suit
      draw_card(value_x1 + (2*i), value_y1 + 2*i, card_values[i], card_suits[i]);
  }
  */
  
  // player 2 deck
  int start_x2 = 178;
  int start_y2 = 215;
  /*
  for (int i = 0; i < num_of_cards; i++) {
      box_x = start_x2 + (i * card_spacing_x);
      box_y = start_y2 + (i * card_spacing_y);

      // Draw card box
      VGA_box(box_x, box_y, box_x + box_width, box_y + box_height, red_color);

      value_x2 = 40;
      value_y2 = 44;
      // Draw card value and suit
      draw_card(value_x2 + (2*i), value_y2 + 2*i, card_values[i], card_suits[i]);
  }
  */
  
  // player 3 deck
  //VGA_box(340, 240, 370, 285, red_color);
  //draw_card(60, 47, "3", 'c'); // 3 of clubs
  
  int start_x3 = 338;
  int start_y3 = 215;
  /*
  for (int i = 0; i < num_of_cards; i++) {
      box_x = start_x3 + (i * card_spacing_x);
      box_y = start_y3 + (i * card_spacing_y);

      // Draw card box
      VGA_box(box_x, box_y, box_x + box_width, box_y + box_height, red_color);

      value_x3 = 60;
      value_y3 = 44;
      // Draw card value and suit
      draw_card(value_x3 + (2*i), value_y3 + 2*i, card_values[i], card_suits[i]);
  }
  */
  
  // player 4 deck  
  int start_x4 = 498;
  int start_y4 = 215;
  /*
  for (int i = 0; i < num_of_cards; i++) {
      box_x = start_x4 + (i * card_spacing_x);
      box_y = start_y4 + (i * card_spacing_y);

      // Draw card box
      VGA_box(box_x, box_y, box_x + box_width, box_y + box_height, red_color);

      value_x4 = 80;
      value_y4 = 44;
      // Draw card value and suit
      draw_card(value_x4 + (2*i), value_y4 + 2*i, card_values[i], card_suits[i]);
  }
  */
  
  // TOP RIGHT
  // track ace + max and min bets
  VGA_text(55,10,"Max Bet: $1000");
  VGA_text(55,12,"Min Bet: $3");
  
  // chip values
  VGA_disc(224, 452, 15, blue);
  VGA_text(26,56,"$100");
  VGA_disc(264, 452, 15, sky_blue);
  VGA_text(32,56,"$50");
  VGA_disc(304, 452, 15, orange); // orange
  VGA_text(37,56,"$20");
  VGA_disc(344, 452, 15, gold); // yellow
  VGA_text(42,56,"$10");
  VGA_disc(384, 452, 15, purple); // purple
  VGA_text(47,56,"$5");
  VGA_disc(424, 452, 15, magenta);
  VGA_text(52,56,"$1");
  
  // incr/decr user option
  VGA_disc(164, 452, 20, colors[9]); // gray
  VGA_text(20,56,"-");
  VGA_disc(484, 452, 20, colors[9]); // gray
  VGA_text(60,56,"+");
  
  // Hit/stand option + probability 
  VGA_disc(180,175,25,brown); // brown
  VGA_text(21,21,"HIT");
  VGA_line(210, 150, 210, 200, red);
  VGA_box(210, 150, 300, 200, colors[9]); //gray
  VGA_text(28,19,"Win Prob");
  
  //VGA_text(29,22,"97.00%");
  
  VGA_disc(450,175,25,brown); // brown
  VGA_text(54,21,"STAND");
  VGA_line(410, 150, 410, 200, red);
  VGA_box(328, 150, 418, 200, colors[9]); // gray
  //VGA_text(53,19,"Tie Prob"); // old position
  VGA_text(43,19,"Tie Prob");
  //VGA_text(54,22,"3.00%"); // old position
  
  
  // Dynamic 1-4 players with their balances
  int numOfPlayers = 4;

  int x_base = 5;
  int y_base = 50;
  int x_offset = 20;
  
  for (int i = 0; i < numOfPlayers; i++) {
      char bet_area[20];
      char player_text[20];
      char balance_text[20];
    
      sprintf(bet_area, "Bet Area: ");  
      sprintf(player_text, "Player %d", i + 1);
      sprintf(balance_text, "Balance: ");
    
      VGA_text(x_base + (i*x_offset), y_base - 2, bet_area);
      VGA_text(x_base + (i*x_offset), y_base, player_text);
      VGA_text(x_base + (i*x_offset), y_base + 1, balance_text);
  }
  
  // Dealer
  VGA_text(35,2,"Dealer");
  //VGA_box(250, 40, 280, 85, red);
  //draw_card(49, 22, "4", 'h'); // ace of hearts
  //VGA_box(267, 50, 297, 95, red);
  //VGA_text(34,8," ?");  
}

int main(void) {
    // === get FPGA addresses ==================
      // Open /dev/mem
      
    //srand(time(NULL)); 
    
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
 
      
      // Get the address that maps to the FPGA pixel buffer
  	//vga_pixel_ptr =(unsigned int *)(vga_pixel_virtual_base);
    printf("ptr: %d\n", (unsigned int) vga_pixel_virtual_base);
    
    // pointers are assigned by adding defined offset defined above based on memory locations from Platform Designer
    player_init_hand_ptr = (unsigned char *)(h2p_lw_virtual_base + PLAYER_INIT_HAND_PIO);
    dealer_top_ptr = (unsigned char *)(h2p_lw_virtual_base + DEALER_TOP_PIO);
    init_done_ptr = (unsigned char *)(h2p_lw_virtual_base + INIT_DONE_PIO);
    shared_write_ptr = (unsigned char *)(h2p_lw_virtual_base + SHARED_WRITE_PIO);
    read_addr_ptr = (unsigned char *)(h2p_lw_virtual_base + READ_ADDR_PIO);
    output_random_ptr = (unsigned int *)(h2p_lw_virtual_base + OUTPUT_RANDOM_PIO);
    
    draw_dealer_1_ptr = (unsigned char *)(h2p_lw_virtual_base + DRAW_DEALER_1_PIO);
    draw_dealer_2_ptr = (unsigned char *)(h2p_lw_virtual_base + DRAW_DEALER_2_PIO);
    draw_dealer_3_ptr = (unsigned char *)(h2p_lw_virtual_base + DRAW_DEALER_3_PIO);
    draw_player_1 = (unsigned char *)(h2p_lw_virtual_base + DRAW_PLAYER_1_PIO);
    draw_player_2 = (unsigned char *)(h2p_lw_virtual_base + DRAW_PLAYER_2_PIO);
    draw_player_3 = (unsigned char *)(h2p_lw_virtual_base + DRAW_PLAYER_3_PIO);
    test_3_ptr = (unsigned char *)(h2p_lw_virtual_base + TEST_3_PIO);
    
    // default values for reset
    *(init_done_ptr) = init_done_temp;
    //*(shared_write_ptr) = shared_write_temp;
    
  	// get RAM float param addr
  	fp_ram_virtual_base = mmap(NULL, FPGA_ONCHIP_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, FPGA_ONCHIP_BASE); // shared memory btwn HPS and FPGA location
  	
  	if (fp_ram_virtual_base == MAP_FAILED) {
  		printf( "ERROR: mmap3() failed...\n" );
    		close( fd );
    		return(1);
  	}
	
	  // get addr that maps to RAM buffer
    fp_ram_ptr = (unsigned int *)(fp_ram_virtual_base);
    fp_ram_ptr_seed = (unsigned int *)(fp_ram_virtual_base + SEED_SRAM_ARR);

    srand(time(NULL)); // Seed random number generator
    
    graphics_main();
    //short background_color = rgb(0,0,0);
  	// clear the screen
  	//VGA_box (0, 0, 639, 479, 0x0000);
    //return 0;
    int numberOfPlayers = 0;
    int* playerChips = NULL;
    char choice = 'r'; // Default to 'r'

    do {
        if (choice == 'r') {
            if (playerChips != NULL) {
                free(playerChips); // Free previous allocation if any
            }

            printf("Enter the number of players: ");
            scanf("%d", &numberOfPlayers);
            clearInputBuffer();
            // Dynamically allocate memory for each player's chips
            playerChips = malloc(numberOfPlayers * sizeof(int));
            if (!playerChips) {
                printf("Failed to allocate memory for player chips.\n");
                return 1; // Exit if allocation fails
            }

            // Initialize player chips at the start of each new game setup
            initializePlayerChips(playerChips, numberOfPlayers);
        }

        // Play the game
        playBlackjack(playerChips, numberOfPlayers);

        // Ask if users want to play again, keep playing with the same setup, or restart
        printf("\nDo you want to play again with the same players (y), restart with new players (r), or quit (q)? ");
        scanf(" %c", &choice);
        clearInputBuffer();

    } while (choice == 'y' || choice == 'r'); // Continue if users want to play again or restart

    printf("Thanks for playing!\n");

    if (playerChips != NULL) {
        free(playerChips); // Cleanup at the end
    }
    
    return 0;
}
