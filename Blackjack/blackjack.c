///////////////////////////////////////
/// Blackjack
/// compile with
/// gcc blackjack.c -o bj -o0 -std=c99
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "address_map_arm_brl4.h"

void *h2p_lw_virtual_base;

// RAM fp buffer
volatile unsigned int * fp_ram_ptr = NULL;
void *fp_ram_virtual_base;
// file id
int fd;

// fixed point
typedef signed int fix;
#define float2fix30(a) ((int)((a)*1073741824)) // 2^30

// sram pointer with base addr
//volatile unsigned int* sram_ptr = NULL;
//#define SRAM_ARR 0x00

#define DECK_SIZE 156 // 3 decks of cards, 52 for each
#define FACE_VALUES 13
#define CHIP_TYPES 6

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
int chips[] = {1, 3, 5, 10, 25, 100};
int playerChips = 1000; // Starting chips

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

    // Betting phase
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
    int dealerTotal = 0, dealerAceCount = 0, deckPosition = 0;
    for (int i = 0; i < numberOfPlayers; i++) {
        int temp = 0;
        int aceValue = 11;
        printf("Player %d, Your cards:\n",i + 1);
        temp += dealCard(&playerTotals[i], &deckPosition, &playerAceCounts[i], 1);
        printCard(&deck[deckPosition - 1]);

        temp += dealCard(&playerTotals[i], &deckPosition, &playerAceCounts[i], 1);
        printCard(&deck[deckPosition - 1]);

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
    printf("Dealer's shown card: ");
    printCard(&deck[deckPosition - 1]);
    printf("\n");
    
    for (int i = 0; i < DECK_SIZE-(numberOfPlayers*2+1); i++) { // filling memory
      *(fp_ram_ptr + i) = deck[i].value;
    }

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
                    printf("\nPlayer %d, your total is %d. Hit (h) or stand (s)? ", i + 1, playerTotals[i]);
                    char choice;
                    scanf("%c", &choice);
                    clearInputBuffer();
                    if (choice == 'h') {
                        playerTotals[i] += dealCard(&playerTotals[i], &deckPosition, &playerAceCounts[i], 0);
                        printf("\nDealted card: ");
                        printCard(&deck[deckPosition - 1]);

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
    
    ////////////
    
    /*
    while(1) {
      int in0;
      int addr;
      
      // set up parameters
      *(fp_ram_ptr) = in0;
      while(*(fp_ram_ptr) != 0) {}
      
      for (addr=0; addr<2; addr++) {
        
      }
    }
    */

    return 0;
}
