/*
// Main
void main(void){

    unsigned char currKey=0, dispSz = 3;
    unsigned char dispThree[3];

    int simonArray [10];
    unsigned char printArray[3];


    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
                                 // You can then configure it properly, if desire

    // Useful code starts here
    configDisplay();
    configKeypad();

    // *** Intro Screen ***
    Graphics_clearDisplay(&g_sContext); // Clear the display

    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, "SIMON", AUTO_STRING_LENGTH, 64, 50, TRANSPARENT_TEXT);

    Graphics_flushBuffer(&g_sContext);

    dispThree[0] = ' ';
    dispThree[2] = ' ';



    while (1)    // Forever loop
    {
        // Check if any keys have been pressed on the 3x4 keypad
        currKey = getKey();
        if (currKey == '*')
        {
            //Give the 3... 2... 1...
            intro();

            //Defines the array
            int upper = 52, lower = 49, i, j, k, l; //Comparing the ascii values of 1-4 so 49-52
            //Has all 10 of its values from 1-4 in ascii
            for (i = 0; i < 10; i++){
              simonArray[i] = (rand() % (upper - lower + 1)) + lower;
            }

            //Will run through the 10 iterations of this game
            for(j = 10; j > 0; j--){
                //Will handle all of the array print outs
                for (k = j; k < 10; k++){
                    printArray[1] = simonArray[k-j];
                    Graphics_clearDisplay(&g_sContext); // Clear the display
                    Graphics_drawStringCentered(&g_sContext, printArray, dispSz, 64, 90, OPAQUE_TEXT);
                    Graphics_flushBuffer(&g_sContext);
                    swDelay(2);
                    Graphics_clearDisplay(&g_sContext); // Clear the display
                    Graphics_flushBuffer(&g_sContext);
                 }

                //Will handle comparing the array prints
                for (l = j; l < 10; l++){
                      if (currKey){
                         //swDelay(3);
                         if(currKey == simonArray[l-j]){
                             Graphics_clearDisplay(&g_sContext); // Clear the display

                             // Draw the new character to the display
                             Graphics_drawStringCentered(&g_sContext, dispThree, dispSz, 64, 90, OPAQUE_TEXT);
                             Graphics_flushBuffer(&g_sContext);
                             buzzer();

                             // wait awhile before clearing LEDs
                             Graphics_clearDisplay(&g_sContext); // Clear the display
                             Graphics_flushBuffer(&g_sContext);
                      }
                      else {
                         Graphics_clearDisplay(&g_sContext); // Clear t````````````````````he display
                         Graphics_flushBuffer(&g_sContext);
                         Graphics_drawStringCentered(&g_sContext, "You lost!", AUTO_STRING_LENGTH, 64, 50, TRANSPARENT_TEXT);
                         Graphics_flushBuffer(&g_sContext);
                      }

                      }
                 }


            }
          }

        if (currKey == '#')
        {
            homeScreen();
        }
    }  // end while (1)
}
*/
