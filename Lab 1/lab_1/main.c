/************** ECE2049 LAB 1 CODE ******************/
/****************************************************/
 /*
  * Programmer: Jack Bergin
  * Date: 02/17/2021
  * Class: ECE 2049 C21'
  * Project: Lab 1 'Simon' game
 */
/****************************************************/

#include "peripherals.h"
#include "stdbool.h"
#include <msp430.h>

// These statements are the Function Prototypes
void swDelay(char numLoops);
void countDown();
void runSimon();
unsigned char simonValues(int numLoops, unsigned char *simonArray);
void simonDisplay(int numLoops, unsigned char *simonArray);
unsigned char getMyKey(void);
//Defines all loop variables
volatile unsigned int i,j,k,m,n;

void main(void)
{
    //Pre-program setup
    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer
    __disable_interrupt();
    configDisplay();
    configKeypad();

    //Paints the intro screen for 'Simon'
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, "Simon", AUTO_STRING_LENGTH, 64, 50, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "Press '*' to play!", AUTO_STRING_LENGTH, 64, 60, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);

    while (1)
    {
        unsigned char currKey = getMyKey();
        if (currKey == '#')
        {
            //Paints the intro screen after the game
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "Simon", AUTO_STRING_LENGTH, 64, 50, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "Press '*' to play!", AUTO_STRING_LENGTH, 64, 60, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
        }

        if (currKey == '*')
        {
            //Paints the screen for the count down of the game
            countDown();

            //Establish space for simon values
            unsigned char *simonArray;
            for(i = 0; i < 10; i++)
            {
                free(simonArray[i]);
            }
            int simonLength = 10;
            simonArray = (int *)malloc(simonLength * sizeof(int));


            //Sends the simonArray to this method for
            //its values to get created and set.
            simonValues(simonLength, simonArray);
            //Executes the game logic
            //This is for the 'Simon' game logic portion of the code.
                //In this there is a nested for loop to run both the graphics and game rules of the program.
                //int j = 1;
                for(j = 0; j < 10; j++){

                    //Displays each character on to the LCD
                    simonDisplay(j, simonArray);

                    //int k;
                    for(k = 0; k < j; k++){
                        //This is for my input key from keypad
                        unsigned char myKey = getMyKey();

                        if (myKey == simonArray[k]){
                            switch(myKey){
                            case '4':
                                //Paints to LCD
                                Graphics_clearDisplay(&g_sContext);
                                Graphics_drawStringCentered(&g_sContext, "4", AUTO_STRING_LENGTH, 88, 50, TRANSPARENT_TEXT);
                                Graphics_flushBuffer(&g_sContext);
                                //Turns on and off the buzzer for one second
                                BuzzerOn(4);
                                swDelay(1);
                                BuzzerOff();

                                break;
                            case'3':
                                //Paints to LCD
                                Graphics_clearDisplay(&g_sContext);
                                Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 66, 50, TRANSPARENT_TEXT);
                                Graphics_flushBuffer(&g_sContext);
                                //Turns on and off the buzzer for one second
                                BuzzerOn(3);
                                swDelay(1);
                                BuzzerOff();
                                break;
                            case '2':
                                //Paints to LCD
                                Graphics_clearDisplay(&g_sContext);
                                Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 44, 50, TRANSPARENT_TEXT);
                                Graphics_flushBuffer(&g_sContext);
                                //Turns on and off the buzzer for one second
                                BuzzerOn(2);
                                swDelay(1);
                                BuzzerOff();
                                break;
                            case '1':
                                //Paints to LCD
                                Graphics_clearDisplay(&g_sContext);
                                Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 22, 50, TRANSPARENT_TEXT);
                                Graphics_flushBuffer(&g_sContext);
                                //Turns on and off the buzzer for one second
                                BuzzerOn(1);
                                swDelay(1);
                                BuzzerOff();
                                break;
                            }
                            swDelay(1);
                        }
                        else if (myKey == 0)  {
                            //Back to loop!
                        }
                        else if((myKey != simonArray[k]) || (myKey != 0)){

                            //Force exits the loops
                            j = 11;
                            k = 12;

                            swDelay(1);

                            //Paints to the LCD screen
                            Graphics_clearDisplay(&g_sContext);
                            Graphics_drawStringCentered(&g_sContext, "Game Over!", AUTO_STRING_LENGTH, 64, 50, TRANSPARENT_TEXT);
                            Graphics_drawStringCentered(&g_sContext, "You Lost!", AUTO_STRING_LENGTH, 64, 60, TRANSPARENT_TEXT);
                            Graphics_drawStringCentered(&g_sContext, "Press '#' for home", AUTO_STRING_LENGTH, 64, 70, TRANSPARENT_TEXT);
                            Graphics_drawStringCentered(&g_sContext, "Press '*' for rematch", AUTO_STRING_LENGTH, 64, 80, TRANSPARENT_TEXT);
                            Graphics_flushBuffer(&g_sContext);
                        }
                    }

                    if(j == 10){//If the maximum array length is hit
                          Graphics_clearDisplay(&g_sContext);
                          Graphics_drawStringCentered(&g_sContext, "Congratulations!", AUTO_STRING_LENGTH, 64, 50, TRANSPARENT_TEXT);
                          Graphics_drawStringCentered(&g_sContext, "You Won!", AUTO_STRING_LENGTH, 64, 60, TRANSPARENT_TEXT);
                          Graphics_drawStringCentered(&g_sContext, "Press '#' for home", AUTO_STRING_LENGTH, 64, 70, TRANSPARENT_TEXT);
                          Graphics_drawStringCentered(&g_sContext, "Press '*' for rematch", AUTO_STRING_LENGTH, 64, 80, TRANSPARENT_TEXT);
                          Graphics_flushBuffer(&g_sContext);
                   }
                } //First for loop
            }// first if
    }  // end infinite loop
}// end main loop

//Gives the countdown at the beginning of the game
void countDown(){
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, "Ready?", AUTO_STRING_LENGTH, 64, 50, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);
    swDelay(2);

    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, "3...", AUTO_STRING_LENGTH, 64, 50, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);
    swDelay(2);

    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, "2...", AUTO_STRING_LENGTH, 64, 50, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);
    swDelay(2);

    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, "1...", AUTO_STRING_LENGTH, 64, 50, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);
    swDelay(2);

    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, "Go!", AUTO_STRING_LENGTH, 64, 50, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);
    swDelay(2);
}

//Assigns the proper values and size to the simon array
unsigned char simonValues(int numLoops, unsigned char *simonArray){
    //Generates values between 1-4
    //int m;
    for (m = 0; m < numLoops; m++) {
      simonArray[m] = (rand() % (4) + 1) + 0x30;
    }
    //Properly 'sizes' the array
    simonArray[numLoops] = '\0';
    return simonArray[numLoops];
}

//Displays each individual simonArray element
void simonDisplay(int numLoops, unsigned char *simonArray){
    //int n;
    Graphics_clearDisplay(&g_sContext);
    for (n= 0; n < numLoops; n++) {
        unsigned char simonVal = simonArray[n];
        if(simonVal == '4') {
            //Paints the character to the LCD
            Graphics_clearDisplay(&g_sContext);
            Graphics_drawStringCentered(&g_sContext, "4", AUTO_STRING_LENGTH, 65, 50, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            //Turns on and off the buzzer for one second
            BuzzerOn(4);
            swDelay(1);
            BuzzerOff();
        }
         if(simonVal == '3') {
            //Paints the character to the LCD
            Graphics_clearDisplay(&g_sContext);
            Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 65, 50, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            //Turns on and off the buzzer for one second
            BuzzerOn(3);
            swDelay(1);
            BuzzerOff();
        }
         if(simonVal == '2') {
            //Paints the character to the LCD
            Graphics_clearDisplay(&g_sContext);
            Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 65, 50, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            //Turns on and off the buzzer for one second
            BuzzerOn(2);
            swDelay(1);
            BuzzerOff();
        }
         if(simonVal == '1') {
            //Paints the character to the LCD
            Graphics_clearDisplay(&g_sContext);
            Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 65, 50, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            //Turns on and off the buzzer for one second
            BuzzerOn(1);
            swDelay(1);
            BuzzerOff();
        }
        Graphics_clearDisplay(&g_sContext);
        Graphics_flushBuffer(&g_sContext);
    }
}

unsigned char getMyKey(void){
    //returns my key input in a more direct way
    char primer = 0;
    while(primer == 0){
        unsigned char returnKey = getKey();
        if (returnKey == 0){
                    //Stalls
        }
        if (returnKey != 0){
            primer = 1;
            return returnKey;
        }
    }
}

void swDelay(char numLoops){
    //This is for the delays within the code.
    //It loops a lot to waste time and takes about a second when an input of 1 is given.
    volatile unsigned int i,j;  // volatile to prevent removal in optimization
                                // by compiler. Functionally this is useless code
    for (j=0; j<numLoops; j++)
    {
        i = 50000 ;                 // SW Delay
        while (i > 0)               // could also have used while (i)
           i--;
    }
}
