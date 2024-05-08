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
#include <pthread.h>
#define MOUSE_DEV "/dev/input/mice"
// network stuff
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <netdb.h> 

//#include "address_map_arm_brl4.h"

void *h2p_lw_virtual_base;

pthread_mutex_t lock;

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
void Game_text_clear(int, int, int, int);

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
#define gold ((24 << 11) + (50 << 5) + 0)
#define brown ((15 << 11) + (15 << 5) + 7)
#define sky_blue ((12 << 11) + (52 << 5) + 30)
#define diffgreen ((6 << 11) + (43 << 5) + 7)
#define dark_red ((22 << 11) + (8 << 5) + 3)

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

volatile unsigned int* num_wins_ptr = NULL;
volatile unsigned int* num_ties_ptr = NULL;
volatile unsigned int* card_one_ptr = NULL;
volatile unsigned int* card_two_ptr = NULL;
volatile unsigned int* card_three_ptr = NULL;
volatile unsigned int* card_four_ptr = NULL;
volatile unsigned int* card_five_ptr = NULL;
volatile unsigned int* card_six_ptr = NULL;
volatile unsigned int* card_seven_ptr = NULL;
volatile unsigned int* card_eight_ptr = NULL;
volatile unsigned int* card_nine_ptr = NULL;
volatile unsigned int* card_ten_ptr = NULL;

volatile unsigned int* simul_complete_ptr = NULL;
volatile unsigned int* mem_starting_ptr = NULL;
volatile unsigned int* dealer_top_3_ptr = NULL;

// base addr for each PIO block
#define PLAYER_INIT_HAND_PIO 0x00
#define DEALER_TOP_PIO 0x10
#define INIT_DONE_PIO 0x20

#define NUM_WINS_PIO 0x30
#define NUM_TIES_PIO 0x40
#define CARD_ONE_PIO 0x50
#define CARD_TWO_PIO 0x70
#define CARD_THREE_PIO 0x80
#define CARD_FOUR_PIO 0x90
#define CARD_FIVE_PIO 0x100
#define CARD_SIX_PIO 0x110
#define CARD_SEVEN_PIO 0x120
#define CARD_EIGHT_PIO 0x130
#define CARD_NINE_PIO 0x140
#define CARD_TEN_PIO 0x150

#define SIMUL_COMPLETE_PIO 0x160
#define MEM_STARTING_PIO 0x170
#define DEALER_TOP_3_PIO 0x180


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

#define DECK_SIZE 260 // 3 decks of cards, 52 for each
#define FACE_VALUES 13
#define CHIP_TYPES 6

//Simulation variables
#define NUM_SIMUL 6000.0 //should be 5, 10, 15
#define HPS_SIMUL 2000.0

// VGA functions
/* function prototypes */
void draw_card(int, int, char[], char[]);
void display_card(int, int, char[], char[], int);
void display_dealer_card(int, char[], char[], int);
void display_result(int, int, int, int);

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
//char *track_bet[4];
int values[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 11};
char *card_val_count[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
int card_counter[10];
// TODO: make sure three 10s show Jack, Queen, King, ACE instead of 11
char *char_vals[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"}; 

int chips[] = {1, 3, 5, 10, 25, 100};
int playerChips = 1000; // Starting chips

int num_cards_players[4] = {0, 0, 0, 0};
volatile int track_dealer_cards = 0;
int num_results_players[4] = {0, 0, 0, 0};

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
        
        int value_betx = 17;
        
        char track_bet1[50];
        sprintf(track_bet1, "%d", bets[0]);
        VGA_text(value_betx,48,track_bet1);
        
        char track_bet2[50];
        if (bets[1]) {
          sprintf(track_bet2, "%d", bets[1]);
          VGA_text(value_betx + 20,48,track_bet2);
        }
        
        char track_bet3[50];
        if (bets[2]) {
          sprintf(track_bet3, "%d", bets[2]);
          VGA_text(value_betx + 40,48,track_bet3);
        }
        
        char track_bet4[50];
        if (bets[3]) {
          sprintf(track_bet4, "%d", bets[3]);
          VGA_text(value_betx + 60,48,track_bet4);
        }
        
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
    //printf("Dealer's shown card: ");
    //printCard(&deck[deckPosition - 1]);
    //printf("\n");
    
    // Display Dealer cards
    display_dealer_card(deck[deckPosition - 1].value, deck[deckPosition - 1].face, deck[deckPosition - 1].suit, track_dealer_cards);
    track_dealer_cards++;
    display_dealer_card('?', (int)'NULL', '?', track_dealer_cards);
   
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
    
    // TODO when it's less than 3, should redraw
    for (int i = (numberOfPlayers*2+1); i < DECK_SIZE; i++) { // filling memory
      printf("RAM addr=%d contents=%d \n\r", i, *(fp_ram_ptr+i));
    }
  
    //usleep(5000000); // delay before shared write
    int sampl_counter = 0;
    usleep(100000);
    
    int net_win_count = (*num_wins_ptr);
    int net_ties_count = (*num_ties_ptr);
    unsigned int sum_net = 0;
    for (int k = 0; k < HPS_SIMUL; k++){
      *init_done_ptr = 0;
      *(player_init_hand_ptr) = playerTotals[0]; // TODO handle multiple players init hands
      *(dealer_top_ptr) = dealerTotal;
      for (int i = 0; i < 300; i++) { // filling memory
        *(fp_ram_ptr_seed + i) = (rand() % DECK_SIZE);
      }
      *init_done_ptr = 1;
      while((*simul_complete_ptr) != 1);
      //printf("This is the current net_win_count_test: %d %d %d %d\n", (*num_wins_ptr), k, net_win_count, net_ties_count);
      net_win_count += (*num_wins_ptr);
      net_ties_count += (*num_ties_ptr);
      card_counter[0] += (*card_one_ptr);
      card_counter[1] += (*card_two_ptr);
      card_counter[2] += (*card_three_ptr);
      card_counter[3] += (*card_four_ptr);
      card_counter[4] += (*card_five_ptr);
      card_counter[5] += (*card_six_ptr);
      card_counter[6] += (*card_seven_ptr);
      card_counter[7] += (*card_eight_ptr);
      card_counter[8] += (*card_nine_ptr);
      card_counter[9] += (*card_ten_ptr);
      //sum_net += (card_counter[0] +  card_counter[1] +  card_counter[2] + card_counter[3] + card_counter[4] + card_counter[5] + card_counter[6] + card_counter[7] + card_counter[8] + card_counter[9]);
      //usleep(100000);
    }
    sum_net = (card_counter[0] +  card_counter[1] +  card_counter[2] + card_counter[3] + card_counter[4] + card_counter[5] + card_counter[6] + card_counter[7] + card_counter[8] + card_counter[9]);
    usleep(1000);
    int num_games = NUM_SIMUL;
    int num_hps = HPS_SIMUL;
    printf("Total games played %d \n", num_games * num_hps);
    printf("Total sum of all cards %d \n", sum_net);
    
    while (sampl_counter <= 5) {
      printf("Next hit 1: %d, Next hit 2: %d, Next hit 3: %d\n", card_counter[0], card_counter[1], card_counter[2]);
      printf("Next hit 4: %d, Next hit 5: %d, Next hit 6: %d\n", (*card_four_ptr), (*card_five_ptr), (*card_six_ptr));
      printf("Next hit 7: %d, Next hit 8: %d, Next hit 9: %d\, Next hit 10: %d\n", (*card_seven_ptr), (*card_eight_ptr), (*card_nine_ptr), (*card_ten_ptr));
      printf("num wins: %d, num ties: %d\n", net_win_count, net_ties_count);        
      printf("all simulations are done: %d, where mem starting: %d, what FPGA received about mem starting: %d\n", (*simul_complete_ptr), (*mem_starting_ptr), (*dealer_top_3_ptr));
      sampl_counter++;
    }
    display_top_cards(sum_net);
    printf("Dealer's shown card: ");
    printCard(&deck[deckPosition - 1]);
    printf("\n");
    display_top_cards(sum_net);
    //printf("Init done %d\n", (int)(*init_done_ptr));
    //printf("Shared Write Done Signal 2nd check %d\n", (int)(*shared_write_ptr));
    dealerTotal += dealCard(&dealerTotal, &deckPosition, &dealerAceCount, 1);
    
    if(dealerAceCount > 0 && dealerTotal == 21){
        printf("Dealer hits BlackJack!\n");
        display_dealer_card(deck[deckPosition - 1].value, deck[deckPosition - 1].face, deck[deckPosition - 1].suit, track_dealer_cards);
        track_dealer_cards++;
        VGA_box(245, 60, 365, 110, black);
        //track_dealer_cards++;
        Game_text_clear(31, 45, 7, 13);
        VGA_text(33,10,"BLACKJACK!");
        dealerBJ++;
    }
    if (dealerTotal >= 17 && dealerTotal < 21){
        display_dealer_card(deck[deckPosition - 1].value, deck[deckPosition - 1].face, deck[deckPosition - 1].suit, track_dealer_cards);
        track_dealer_cards++;
    }
    if (!dealerBJ){
        usleep(500);
        // Player decision phase
        for (int i = 0; i < numberOfPlayers; i++) {
            while (true) {
                if (blackjack[i] != 1){
                    Game_text_clear(55,68,10,14);
                    Game_text_clear(55,68,20,24);
                
                    int number_wins = (*num_wins_ptr);
                    printf("Num wins huge: %d\n", number_wins);
                    float prob_win = (((float)(net_win_count))/(num_games * num_hps)) * 100;
                    printf("wins prob %f\n", prob_win);
                    char prob_win_str[30];
                    sprintf(prob_win_str, "%.2f%%",prob_win);
                    printf(prob_win_str); 

                    VGA_text(59,12,prob_win_str);
                    printf("\nDealt card: ");
                    printCard(&deck[deckPosition - 1]);
                    printf("wins %d\n", (*num_wins_ptr));
                    
                    int num_ties = (*num_ties_ptr);
                    float prob_tie = (net_ties_count/(HPS_SIMUL * NUM_SIMUL)) * 100;
                    char prob_tie_str[30];
                    sprintf(prob_tie_str, "%.2f%%",prob_tie);
                    printf(prob_tie_str); 
                    VGA_text(59,22,prob_tie_str);
                    printf("\nDealt card: ");
                    printCard(&deck[deckPosition - 1]);
                    printf("ties %d\n", (*num_ties_ptr));        
                    
                    printf("\nPlayer %d, your total is %d. Hit (h) or stand (s)? ", i + 1, playerTotals[i]);
                    char choice;
                    scanf("%c", &choice);
                    clearInputBuffer();
                    
                    
                    if (choice == 'h') {
                       
                        playerTotals[i] += dealCard(&playerTotals[i], &deckPosition, &playerAceCounts[i], 0);
                
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
                        net_win_count = 0;
                        net_ties_count = 0;
                        card_counter[0] = 0;
                        card_counter[1] = 0;
                        card_counter[2] = 0;
                        card_counter[3] = 0;
                        card_counter[4] = 0;
                        card_counter[5] = 0;
                        card_counter[6] = 0;
                        card_counter[7] = 0;
                        card_counter[8] = 0;
                        card_counter[9] = 0;
                        sum_net = 0;
                        for (int k = 0; k < HPS_SIMUL; k++){
                          *init_done_ptr = 0;
                          *(player_init_hand_ptr) = playerTotals[i]; // TODO handle multiple players init hands
                          *(dealer_top_ptr) = dealerTotal;
                          for (int i = 0; i < 300; i++) { // filling memory
                            *(fp_ram_ptr_seed + i) = (rand() % DECK_SIZE);
                          }
                          *init_done_ptr = 1;
                          while((*simul_complete_ptr) != 1);
                          //printf("This is the current net_win_count_test: %d %d %d %d\n", (*num_wins_ptr), k, net_win_count, net_ties_count);
                          net_win_count += (*num_wins_ptr);
                          net_ties_count += (*num_ties_ptr);
                          card_counter[0] += (*card_one_ptr);
                          card_counter[1] += (*card_two_ptr);
                          card_counter[2] += (*card_three_ptr);
                          card_counter[3] += (*card_four_ptr);
                          card_counter[4] += (*card_five_ptr);
                          card_counter[5] += (*card_six_ptr);
                          card_counter[6] += (*card_seven_ptr);
                          card_counter[7] += (*card_eight_ptr);
                          card_counter[8] += (*card_nine_ptr);
                          card_counter[9] += (*card_ten_ptr);
                          printf("Inside This is the current net_win_count_test: %d %d %d %d %d\n", card_counter[1], card_counter[2], card_counter[3], card_counter[4], *(card_six_ptr));
                          usleep(1000);
                        }
                        sum_net = (card_counter[0] +  card_counter[1] +  card_counter[2] + card_counter[3] + card_counter[4] + card_counter[5] + card_counter[6] + card_counter[7] + card_counter[8] + card_counter[9]);
                        display_top_cards(sum_net);
                        /*
                        *init_done_ptr = 0;
                        *(player_init_hand_ptr) = playerTotals[i];
                        *(dealer_top_ptr) = dealerTotal;
                        *init_done_ptr = 1;*/
                        
                        usleep(100000);
                        /*
                        sampl_counter = 0;
                        while (sampl_counter <= 50) {
                          printf("Next hit 1: %d, Next hit 2: %d, Next hit 3: %d\n", (*card_one_ptr), (*card_two_ptr), (*card_three_ptr));
                          printf("Next hit 4: %d, Next hit 5: %d, Next hit 6: %d\n", (*card_four_ptr), (*card_five_ptr), (*card_six_ptr));
                          printf("Next hit 7: %d, Next hit 8: %d, Next hit 9: %d\, Next hit 10: %d\n", (*card_seven_ptr), (*card_eight_ptr), (*card_nine_ptr), (*card_ten_ptr));
                          printf("num wins: %d, num ties: %d\n", (*num_wins_ptr), (*num_ties_ptr));        
                          printf("dealer top 1st simul: %d, dealer top 2nd simul: %d,dealer top 3rd simul: %d\n", (*dealer_top_1_ptr), (*dealer_top_2_ptr), (*dealer_top_3_ptr));
                          sampl_counter++;
                        }*/
                        
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

        //display_dealer_card(deck[deckPosition - 1].value, deck[deckPosition - 1].face, deck[deckPosition - 1].suit, track_dealer_cards);   
        //track_dealer_cards++;     
        while (dealerTotal < 17) {
            printf("\nDealer's now: %d\n", dealerTotal);
            printf("\nDealer draws: ");

            printCard(&deck[deckPosition - 1]);
            display_dealer_card(deck[deckPosition - 1].value, deck[deckPosition - 1].face, deck[deckPosition - 1].suit, track_dealer_cards);
            track_dealer_cards++;
            dealerTotal += dealCard(&dealerTotal, &deckPosition, &dealerAceCount, 0);
            if (dealerTotal >= 17 && dealerTotal <= 21){
                display_dealer_card(deck[deckPosition - 1].value, deck[deckPosition - 1].face, deck[deckPosition - 1].suit, track_dealer_cards);
                track_dealer_cards++;
            }
        }

        printf("\nDealer's final total: %d\n", dealerTotal);
        if(dealerTotal > 21){
            display_dealer_card(deck[deckPosition - 1].value, deck[deckPosition - 1].face, deck[deckPosition - 1].suit, track_dealer_cards);
            track_dealer_cards++;
            printf("Dealer busted! \n");
            VGA_box(245, 60, 365, 110, black); 
            Game_text_clear(31, 45, 7, 13);
            VGA_text(31,10,"Dealer busted!");
        }
    }

    // Who wins
    for (int i = 0; i < numberOfPlayers; i++) {
        if(blackjack[i] == 1 && !dealerBJ){
            Game_text_clear(15,19,51,51);
            Game_text_clear(35,19,51,51);
            Game_text_clear(55,19,51,51);
            Game_text_clear(75,19,51,51);
        
            printf("\nPlayer %d hits BlackJack and now has %d chips.\n", i + 1, playerChips[i] + bets[i] * 3);
            playerChips[i] += bets[i] * 3;
            
            int value_balancex = 15;
        
            char track_bal1[50];
            sprintf(track_bal1, "%d", playerChips[0]);
            VGA_text(value_balancex,50,track_bal1);
            
            char track_bal2[50];
            if (playerChips[1]) {
              sprintf(track_bal2, "%d", playerChips[1]);
              VGA_text(value_balancex + 20,50,track_bal2);
            }
            
            char track_bal3[50];
            if (playerChips[2]) {
              sprintf(track_bal3, "%d", playerChips[2]);
              VGA_text(value_balancex + 40,50,track_bal3);
            }
            
            char track_bal4[50];
            if (playerChips[3]) {
              sprintf(track_bal4, "%d", playerChips[3]);
              VGA_text(value_balancex + 60,50,track_bal4);
            }
            
            //Game_text_clear(3, 76, 31, 37);
            Game_text_clear(2 + (20*i), 16 + (20*i), 31, 37);
            display_result(i, 4, num_results_players[i], bets[i]);
            //bets[i]++;
            num_results_players[i]++;
        }
        if (playerTotals[i] > 21) {
            printf("\nPlayer %d loses their bet of %d chips.", i + 1, bets[i]);
            printf(" Player %d chip count is now %d.\n", i + 1, playerChips[i]);
            //Game_text_clear(3, 76, 31, 37);
            Game_text_clear(2 + (20*i), 16 + (20*i), 31, 37);
            display_result(i, 2, num_results_players[i], bets[i]);
            //bets[i]++;
            num_results_players[i]++;
        } else if (playerTotals[i] <= 21 && dealerTotal <= 21 && dealerTotal > playerTotals[i]) {
            printf("\nPlayer %d loses their bet of %d chips.", i + 1, bets[i]);
            printf(" Player %d chip count is now %d.\n", i + 1, playerChips[i]);
            //Game_text_clear(3, 76, 31, 37);
            Game_text_clear(2 + (20*i), 16 + (20*i), 31, 37);
            display_result(i, 5, num_results_players[i], bets[i]);
            //bets[i]++;
            num_results_players[i]++;
        } else if ((dealerTotal > 21 && playerTotals[i] <= 21) || (playerTotals[i]<=21 && playerTotals[i] > dealerTotal)) {
            printf("\nPlayer %d wins and now has %d chips.\n", i + 1, playerChips[i] + bets[i] * 2);
            playerChips[i] += bets[i] * 2;
            //Game_text_clear(3, 76, 31, 37);
            Game_text_clear(2 + (20*i), 16 + (20*i), 31, 37);
            display_result(i, 1, num_results_players[i], bets[i]);
            //bets[i]++;
            num_results_players[i]++;
        } else {
            printf("\nPlayer %d ties and keeps their bet of %d chips.\n", i + 1, bets[i]);
            playerChips[i] += bets[i];
            //Game_text_clear(3, 76, 31, 37);
            Game_text_clear(2 + (20*i), 16 + (20*i), 31, 37);
            display_result(i, 3, num_results_players[i], bets[i]);
            //bets[i]++;
            num_results_players[i]++;
        }
        //Game_text_clear(3, 76, 31, 37);
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

//VGA_box(238, 40, 273, 85, red);
//draw_card(47, 22, "4", 'h'); // ace of hearts
//VGA_box(255, 48, 290, 93, red);
//draw_card(49, 23, "4", 'h'); // ace of hearts

void display_dealer_card(int val, char face[], char suit[], int ind){
    int box_width = 30;
    int box_height = 45;
    int card_spacing_x = 16;  
    int card_spacing_y = 8;  
    int box_x, box_y, value_x1, value_y1;
    box_x = 242 + (ind * card_spacing_x);
    box_y = 40 + (ind * card_spacing_y);

    // Draw card box
    VGA_box(box_x, box_y, box_x + box_width, box_y + box_height, red);

    value_x1 = 48;
    value_y1 = 22;
    // Draw card value and suit
    char value_dealer[10];
    switch (val) {
      case 2:
        strcpy(value_dealer, "2");
        break;
      case 3:
        strcpy(value_dealer, "3");
        break;
      case 4:
        strcpy(value_dealer, "4");
        break;
      case 5:
        strcpy(value_dealer, "5");
        break;
      case 6:
        strcpy(value_dealer, "6");
        break;
      case 7:
        strcpy(value_dealer, "7");
        break;
      case 8:
        strcpy(value_dealer, "8");
        break;
      case 9:
        strcpy(value_dealer, "9");
        break;
      case 10:
        if (face == "Ten"){
          strcpy(value_dealer, "10");
        }else if (face == "King"){
          strcpy(value_dealer, "K");
        }else if (face == "Queen"){
          strcpy(value_dealer, "Q");
        }else if (face == "Jack"){
          strcpy(value_dealer, "J");
        }
        break;
      case 1:
        strcpy(value_dealer, "A");
        break;
      case 11:
        strcpy(value_dealer, "A");
        break;
      default:
        strcpy(value_dealer, "?");
        break;
    }
    draw_card(value_x1 + (2*ind), value_y1 + 1.5*ind, value_dealer, suit);
}

int compare(const void* a, const void* b){
    int val_1 = *((int*) a);
    int val_2 = *((int*) b);
    
    return card_counter[val_1] - card_counter[val_2];
}

void display_top_cards(int sum_net){
  /*
  VGA_text(4,17,"1");
  VGA_text(10,17,"2");
  VGA_text(16,17,"3");
  VGA_text(22,17,"4");
  VGA_text(27,17,"10");
  VGA_text(4,22,"97%");
  VGA_text(9,22,"12%");
  VGA_text(15,22,"14%");
  VGA_text(20,22,"15%");
  VGA_text(26,22,"34%");*/
  Game_text_clear(2,31,15,25);
  
  //char *card_val_count[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
  int card_index[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 // int card_counter[10];
  printf("This is the current net_win_count_test: %d %d %d %d\n", card_counter[1], card_counter[2], card_counter[3], card_counter[4]);
  
  qsort(card_index, 10, sizeof(int), compare);
  char percent_text[30];
  
  VGA_text(5,17, card_val_count[card_index[5]]);
  VGA_text(10,17,card_val_count[card_index[6]]);
  VGA_text(16,17,card_val_count[card_index[7]]);
  VGA_text(22,17,card_val_count[card_index[8]]);
  VGA_text(27,17,card_val_count[card_index[9]]);
  
  float card_prob = ((float) card_counter[card_index[5]])/((float) sum_net) * 100;
  sprintf(percent_text, "%.2f",card_prob);
  VGA_text(3,22,percent_text);
  card_prob = ((float) card_counter[card_index[6]])/((float) sum_net) * 100;
  sprintf(percent_text, "%.2f",card_prob);
  VGA_text(9,22,percent_text);
  card_prob = ((float) card_counter[card_index[7]])/((float) sum_net) * 100;
  sprintf(percent_text, "%.2f",card_prob);
  VGA_text(15,22,percent_text);
  card_prob = ((float) card_counter[card_index[8]])/((float) sum_net) * 100;
  sprintf(percent_text, "%.2f",card_prob);
  VGA_text(20,22,percent_text);
  card_prob = ((float) card_counter[card_index[9]])/((float) sum_net) * 100;
  sprintf(percent_text, "%.2f",card_prob);
  VGA_text(25,22,percent_text);
  
  if ((card_counter[card_index[5]] || card_counter[card_index[6]] || card_counter[card_index[7]] || card_counter[card_index[8]] || card_counter[card_index[9]]) == 0) {
    sprintf(percent_text, "%.2f",0);
    VGA_text(3,22,percent_text);
    VGA_text(9,22,percent_text);
    VGA_text(15,22,percent_text);
    VGA_text(20,22,percent_text);
    VGA_text(25,22,percent_text);
    
    VGA_box(260, 150, 410, 200, black); 
    VGA_text(35,19,"Suggested move");
    VGA_text(39,22,"STAND");  
  }
  
  // TODO clear STAND
  //Game_text_clear(35,39,19,22);
  
  for (int j = 0; j < 10; j++){
    printf("index_val: %d, card_count: %d\n", card_index[j], card_counter[card_index[j]]);
  }
}

// player 1 box - 15, 250, 135, 300
// player 1 text - 3, 34
// player 2 box - 175, 250, 295, 300
// player 2 text - 23, 34
// player 3 box - 335, 250, 455, 300
// player 3 text - 43, 34
// player 4 box - 495, 250, 615, 300
// player 4 text - 63, 34
void display_result(int player, int val, int ind, int bets){
    int box_width = 120;
    int box_height = 50;
    int card_spacing_x = 160;  
    int card_spacing_y = 0;  
    int box_x, box_y, value_x, value_y;
    box_x = 15 + (player*card_spacing_x);
    box_y = 250;

    // Draw result box
    VGA_box(box_x, box_y, box_x + box_width, box_y + box_height, black);

    value_x = 3 + (player*20);
    value_y = 34;
    
    char track_bet[50];
    switch (val) {
      case 1: // winning
        VGA_text(value_x+ind,value_y,"You won $");
        sprintf(track_bet, "%d", bets);
        VGA_text((value_x+9),value_y,track_bet);
        break;
      case 2: // busted lost
        VGA_text(value_x+ind,(value_y-1),"You busted!");
        VGA_text(value_x+ind,(value_y+1),"You lost $");
        sprintf(track_bet, "%d", bets);
        VGA_text((value_x+10),(value_y+1),track_bet);
        break;
      case 3: // tie
        VGA_text(value_x,value_y,"You tied");
        break;
      case 4: // blackjack
        VGA_text(value_x+ind,(value_y-1),"BLACKJACK");
        VGA_text(value_x+ind,(value_y+1),"You won $");
        sprintf(track_bet, "%d", bets);
        VGA_text((value_x+9),(value_y+1),track_bet);
        break;
      case 5: // lost
        VGA_text(value_x+ind,value_y,"You lost $");
        sprintf(track_bet, "%d", bets);
        VGA_text((value_x+10),value_y,track_bet);
        break;
    }
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
void VGA_text(int x, int y, char * text_ptr) {
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


void Game_text_clear(int a, int b, int c, int d)
{
  	volatile char * character_buffer = (char *) vga_char_ptr ;	// VGA character buffer
	int offset, x, y;
	for (x=a; x<b; x++){
		for (y=c; y<d; y++){
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
  VGA_text(8,4,"BLACKJACK");
  VGA_box(46, 16, 154, 61, black);
  int heartTitle = 3;
  int spadeTitle = 6;
  char heart_title, spade_title;
  heart_title = (char) heartTitle;
  spade_title = (char) spadeTitle;
  
  // convert suit char to symbol
  static char heart_symbol[1];
  static char spade_symbol[1];
  *(heart_symbol) = (int)heart_title;
  *(spade_symbol) = (int)spade_title;
  
  VGA_text(6,4,heart_symbol);
  VGA_text(18,4,spade_symbol);
  
  //VGA_box(21, 16, 56, 61, black);
  //VGA_rect(21, 16, 56, 61,white);
  /*
  VGA_box(140, 16, 175, 61, black);
  VGA_rect(140, 16, 175, 61,white);
  draw_card(21, 19, "J", "Spades");
  draw_card(36, 19, "J", "Hearts");
  */
  
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
  
  // track max and min bets
  VGA_text(55,4,"Min Bet: $3");
  VGA_text(55,6,"Max Bet: $1000");
  
  // Suggested move
  /*
  VGA_box(260, 150, 410, 200, black); 
  VGA_text(35,19,"Suggested move");
  VGA_text(39,22,"STAND");
  */
  
  // Prob
  VGA_box(20, 70, 240, 200, diffgreen);
  VGA_text(8,10,"Most likely next");
  VGA_text(5,12,"card value + percentage:");
  VGA_disc(40, 142, 16, dark_red);
  VGA_disc(85, 142, 16, dark_red);
  VGA_disc(130, 142, 16, dark_red);
  VGA_disc(175, 142, 16, dark_red);
  VGA_disc(220, 142, 16, dark_red);
  
  /*
  VGA_text(4,17,"1");
  VGA_text(10,17,"2");
  VGA_text(16,17,"3");
  VGA_text(22,17,"4");
  VGA_text(27,17,"10");
  VGA_text(4,22,"97%");
  VGA_text(9,22,"12%");
  VGA_text(15,22,"14%");
  VGA_text(20,22,"15%");
  VGA_text(26,22,"34%");
  */
  
  // win/tie prob
  VGA_box(550, 70, 440, 120, colors[9]); //gray
  VGA_text(58,9,"Win Prob");
  //VGA_text(59,12,"20.13%");
  VGA_box(550, 150, 440, 200, colors[9]); //gray
  VGA_text(58,19,"Tie Prob");
  //VGA_text(59,22,"3.16%");
  
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
  
/*VGA_box(238, 40, 273, 85, red);
draw_card(47, 22, "4", 'h'); // ace of hearts
VGA_box(255, 48, 290, 93, red);
draw_card(49, 23, "4", 'h'); // ace of hearts
VGA_box(229, 56, 259, 101, red);
draw_card(46, 24, "4", 'h'); // ace of hearts
VGA_box(246, 64, 276, 109, red);
draw_card(48, 25, "4", 'h'); // ace of hearts
VGA_box(263, 72, 293, 117, red);
draw_card(50, 26, "4", 'h'); // ace of hearts
VGA_box(280, 80, 310, 124, red);
draw_card(52, 27, "4", 'h'); // ace of hearts
  */
  
  /*
  // winning example on 1st
  VGA_box(15, 250, 135, 300, black);
  VGA_text(3,34,"You won $1000");
  // lost example on 2nd
  VGA_box(175, 250, 295, 300, black);
  VGA_text(23,33,"You busted!");
  VGA_text(23,35,"You lost $1000");
  // tie example on 3rd
  VGA_box(335, 250, 455, 300, black);
  VGA_text(43,34,"You tied");
  // BJ example on fourth
  VGA_box(495, 250, 615, 300, black);
  VGA_text(63,33,"BLACKJACK");
  VGA_text(63,35,"You won $1000");
  */
  
  // clear dealer results
  //VGA_box(245, 60, 365, 110, black);
  //VGA_text(31,7,"X");
  //VGA_text(45,13,"X");
  
  // clear player results
  //VGA_text(3,30,"X");
  //VGA_text(76,37,"X");
  
  // Hit/stand option + probability 
  //VGA_disc(180,175,25,brown); // brown
  //VGA_text(21,21,"HIT");
  
  /* OLD WIN PROB
  VGA_line(210, 150, 210, 200, red);
  VGA_box(210, 150, 300, 200, colors[9]); //gray
  VGA_text(28,19,"Win Prob");
  */
  
  //VGA_text(29,22,"97.00%");
  
  //VGA_disc(450,175,25,brown); // brown
  //VGA_text(54,21,"STAND");
  //VGA_line(410, 150, 410, 200, red);
  //VGA_box(328, 150, 418, 200, colors[9]); // gray
  //VGA_text(53,19,"Tie Prob"); // old position
  //VGA_text(43,19,"Tie Prob");
  //VGA_text(54,22,"3.00%"); // old position

  // TODO probabilities of next number being drawn
  
  // hit and stand
  VGA_disc(104,452,25,brown); // brown
  VGA_text(12,56,"HIT");
  VGA_disc(546,452,25,brown); // brown
  VGA_text(66,56,"STAND");
  
  // Dynamic 1-4 players with their balances
  /*
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
  */
  
  // testing clear player results 
  /*
  VGA_box(15, 250, 135, 300, black);
  VGA_box(175, 250, 295, 300, black);
  VGA_box(335, 250, 455, 300, black);
  VGA_box(495, 250, 615, 300, black);
  VGA_text(2,31,"X");
  VGA_text(16,37,"X");
  VGA_text(22,31,"X");
  VGA_text(36,37,"X");
  VGA_text(42,31,"X");
  VGA_text(56,37,"X");
  VGA_text(62,31,"X");
  VGA_text(76,37,"X");
  */
  
  /* clear most likely and win/tie prob
  VGA_text(2,15,"X");
  VGA_text(31,25,"X");
  VGA_text(55,10,"X");
  VGA_text(68,14,"X");
  VGA_text(55,20,"X");
  VGA_text(68,24,"X");
  */
  
  // Dealer
  VGA_text(35,2,"Dealer");
  //VGA_box(250, 40, 280, 85, red);
  //draw_card(49, 22, "4", 'h'); // ace of hearts
  //VGA_box(267, 50, 297, 95, red);
  //VGA_text(34,8," ?");  
}

void main_menu(){
	VGA_box (0, 0, 639, 479, dark_green);
	VGA_text_clear();
 
  // Title
  VGA_text(8,4,"BLACKJACK");
  VGA_box(21, 16, 56, 61, black);
  VGA_rect(21, 16, 56, 61,white);
  VGA_box(140, 16, 175, 61, black);
  VGA_rect(140, 16, 175, 61,white);
  draw_card(21, 19, "J", "Spades");
  draw_card(36, 19, "J", "Hearts");
  
  // text with descriptions
  VGA_text(30,10,"Welcome to Blackjack!");
  VGA_text(15,15,"Goal: Beat the dealer by having a hand as close to 21 as");
  VGA_text(21,17,"possible without going over.");
  VGA_text(15,20,"Gameplay:");
  VGA_text(21,22,"Each player starts with 2 cards, and the dealer has");
  VGA_text(21,24,"one card face up and one face down.");
  VGA_text(21,26,"Players can choose to Hit (take another card) or");
  VGA_text(21,28, "Stand (keep hand).");
  VGA_text(15,32,"This rendition of Blackjack shows a Monte Carlo simulation");
  VGA_text(15,34,"of your odds to win, lets you know the top 5 most likely");
  VGA_text(15,36,"next values of cards, and assist you for the best result.");
  VGA_text(23,50,"Select the number of players to start");
  
  // num of players
  VGA_disc(180,442,20,red);
  VGA_text(22,55,"1");
  VGA_disc(280,442,20,red);
  VGA_text(34,55,"2");
  VGA_disc(380,442,20,red);
  VGA_text(47,55,"3");
  VGA_disc(480,442,20,red);
  VGA_text(59,55,"4");
}

volatile int scroll_flag;
// thread to handle mouse input
void *handle_mouse_input(void *arg) {
    int fd, bytes;         // define file descriptor for mouse and track bytes read
    unsigned char data[4]; // store data read from mouse
    int arrow_position = 37; // Initial position of arrow
    int prev_arrow_pos;
    char arrow_char[] = {"*"}; // Arrow symbol
    int bet_amounts[8] = {0, 1, 5, 10, 25, 50, 100, 0}; // Bet amounts for each position
    int total_bet = 0; // Total bet amount
    
    fd = open(MOUSE_DEV, O_RDONLY);
    if (fd == -1) {
        printf("ERROR: Could not open %s. Make sure you have the necessary permissions.\n", MOUSE_DEV);
        return NULL;
    }

    while (1) {
        bytes = read(fd, data, sizeof(data)); // read data from mouse into the buffer

        if (bytes > 0) {
            signed char x_movement = -(signed char)data[1];
            int left_click = data[0] & 0x1;
            int right_click = data[0] & 0x2;

            pthread_mutex_lock(&lock);
            
            // Update arrow position based on mouse movement
            arrow_position -= (int)x_movement;
            prev_arrow_pos = arrow_position;
            // Limit arrow movement within valid range
            if (arrow_position < 37)
                arrow_position = 37;
            else if (arrow_position > 57)
                arrow_position = 57;
            
            // Handle left click (add chip to bet or hit)
            if (left_click && arrow_position >= 37 && arrow_position <= 52) {
                int chip_index = (arrow_position - 37) / 3; // Calculate chip index
                total_bet += bet_amounts[chip_index]; // Add chip value to total bet
                // Update total bet display on VGA screen
                // Implement VGA_text function to display text on the screen
                // VGA_text(x, y, "Total Bet: ");
                // VGA_text(x + strlen("Total Bet: "), y, total_bet_string);
            }
            
            // Handle right click (remove chip from bet or stand)
            if (right_click && arrow_position >= 37 && arrow_position <= 52) {
                int chip_index = (arrow_position - 37) / 3; // Calculate chip index
                total_bet -= bet_amounts[chip_index]; // Subtract chip value from total bet
                if (total_bet < 0) total_bet = 0; // Ensure total bet is not negative
                // Update total bet display on VGA screen
                // Implement VGA_text function to display text on the screen
                // VGA_text(x, y, "Total Bet: ");
                // VGA_text(x + strlen("Total Bet: "), y, total_bet_string);
            }
            Game_text_clear(prev_arrow_pos, arrow_position+1, 53,54);
            // Display arrow at new position
            VGA_text(arrow_position, 53, arrow_char);

            pthread_mutex_unlock(&lock);
        }
    }
    close(fd);
    return NULL;
}

// int main(void) {
int main(int argc, char** argv) {
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
    
    num_wins_ptr = (unsigned int *)(h2p_lw_virtual_base + NUM_WINS_PIO);
    num_ties_ptr = (unsigned int *)(h2p_lw_virtual_base + NUM_TIES_PIO);
    card_one_ptr = (unsigned int *)(h2p_lw_virtual_base + CARD_ONE_PIO);
    card_two_ptr = (unsigned int *)(h2p_lw_virtual_base + CARD_TWO_PIO);
    card_three_ptr = (unsigned int *)(h2p_lw_virtual_base + CARD_THREE_PIO);
    card_four_ptr = (unsigned int *)(h2p_lw_virtual_base + CARD_FOUR_PIO);
    card_five_ptr = (unsigned int *)(h2p_lw_virtual_base + CARD_FIVE_PIO);
    card_six_ptr = (unsigned int *)(h2p_lw_virtual_base + CARD_SIX_PIO);
    card_seven_ptr = (unsigned int *)(h2p_lw_virtual_base + CARD_SEVEN_PIO);
    card_eight_ptr = (unsigned int *)(h2p_lw_virtual_base + CARD_EIGHT_PIO);
    card_nine_ptr = (unsigned int *)(h2p_lw_virtual_base + CARD_NINE_PIO);
    card_ten_ptr = (unsigned int *)(h2p_lw_virtual_base + CARD_TEN_PIO);
    
    simul_complete_ptr = (unsigned int *)(h2p_lw_virtual_base + SIMUL_COMPLETE_PIO);
    mem_starting_ptr = (unsigned int *)(h2p_lw_virtual_base + MEM_STARTING_PIO);
    dealer_top_3_ptr = (unsigned int *)(h2p_lw_virtual_base + DEALER_TOP_3_PIO);
    
    *(mem_starting_ptr) = 5;
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
    
    //main_menu();
    graphics_main();
    

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

            int x_base = 5;
            int y_base = 50;
            int x_offset = 20;
            
            for (int i = 0; i < numberOfPlayers; i++) {
                char bet_area[20];
                char player_text[20];
                char balance_text[20];
              
                sprintf(bet_area, "Bet Area: $");  
                sprintf(player_text, "Player %d", i + 1);
                sprintf(balance_text, "Balance: $1000");
              
                VGA_text(x_base + (i*x_offset), y_base - 2, bet_area);
                VGA_text(x_base + (i*x_offset), y_base, player_text);
                VGA_text(x_base + (i*x_offset), y_base + 1, balance_text);
            }
            
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

    // Initialize mutex
    pthread_mutex_init(&lock, NULL);

    // Start threads
    pthread_t mouse_input_thread;
    pthread_create(&mouse_input_thread, NULL, handle_mouse_input, NULL);

    // Wait for threads to finish
    pthread_join(mouse_input_thread, NULL);

    // Cleanup
    pthread_mutex_destroy(&lock);
    
    return 0;
}
