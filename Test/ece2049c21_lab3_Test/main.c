/******************************************************/
/*** LIBRARIES, FUNCTION PROTOtimeTypeS, GLOBAL VAR ***/
/******************************************************/

#include <msp430.h>
#include "peripherals.h"
#include<stdio.h>

// Function PrototimeTypes
void initTimer(void);
void configMonth(long unsigned int day);
void configDaysHrsMinSec(long unsigned int number,int timeType, unsigned char*timeArray);
void displayTime(long unsigned int initialTime);
void defineDays(long unsigned int number, long unsigned int firstDig, long unsigned int secondDig, long unsigned int days);

// Temperature Sensor Calibration = Reading at 30 & 85 degrees C is stored at addr 0x1A1A, and 1A1Ch
#define CALADC12_15V_30C  *((unsigned int *)0x1A1A)
#define CALADC12_15V_85C  *((unsigned int *)0x1A1C)

// Declare globals here
int counter = 0;
unsigned char timeArray[15];
unsigned char tempArray[7];
unsigned char tempAvgArray[7];
unsigned char month[4];
long unsigned int initialTime = 5184000;
volatile float inputTemperature;
float potVolts = 0;
volatile float tempAverageC;
volatile float tempAverageF;
volatile float temperatureDegC;
volatile float temperatureDegF;
int averageCount = 1;

/****************************************************
************* MAIN METHOD FOR PROGRAM ***************
****************************************************/

void main(void){
    int currcount = 0;
    int currcount2 = 0;

    // Stop watchdog timer. Always need to stop this!!
    WDTCTL = WDTPW | WDTHOLD;
    initTimer();
    configDisplay();

    //Enables global interrupts
    __enable_interrupt();
    _BIS_SR(GIE);

    //Gets the display primed for temp and timer display
    Graphics_clearDisplay(&g_sContext);
    Graphics_flushBuffer(&g_sContext);


    //For modification we can begin here
    while(1){
        currcount = counter;
        if((counter) < (currcount+1)){
            //Initializes and defines the global timeArray
            displayTemp(inputTemperature);

            //Initializes and defines the global tempArray
            displayTime(initialTime);

            //Program printout for time, and temperature
            Graphics_drawStringCentered(&g_sContext, timeArray, AUTO_STRING_LENGTH, 46, 25, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, tempArray, AUTO_STRING_LENGTH, 48, 55, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, tempAvgArray, AUTO_STRING_LENGTH, 48, 85, TRANSPARENT_TEXT);
        }

        Graphics_flushBuffer(&g_sContext);

        while((counter) < (currcount +1)){
            //Stall
        }

        Graphics_clearDisplay(&g_sContext);
        currcount2 = 1;
        initialTime = initialTime + currcount2;

    }
}

/****************************************************
************* METHODS FOR THE INTERRUPT *************
****************************************************/

void initTimer(void){
    TA2CTL = TASSEL_1+ID_0+MC_1;

    TA2CCR0 = 32767;

    TA2CCTL0 = CCIE;
}

#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMERINTERUPT(void){
    counter++;
}

/****************************************************
**************** METHODS FOR THE TIMER **************
****************************************************/

void configMonth(long unsigned int day){
    if (day <= 31){
        month[0] = 'J';
        month[1] = 'A';
        month[2] = 'N';

    }
    else if (day <= 59){
        month[0] = 'F';
        month[1] = 'E';
        month[2] = 'B';
    }
    else if (day <= 90){
        month[0] = 'M';
        month[1] = 'A';
        month[2] = 'R';
    }
    else if (day <= 120){
        month[0] = 'A';
        month[1] = 'P';
        month[2] = 'R';
    }
    else if (day <= 151){
        month[0] = 'M';
        month[1] = 'A';
        month[2] = 'Y';
    }
    else if (day <= 181){
        month[0] = 'J';
        month[1] = 'U';
        month[2] = 'N';
    }
    else if (day <= 212){
        month[0] = 'J';
        month[1] = 'U';
        month[2] = 'L';
    }
    else if (day <= 243){
        month[0] = 'A';
        month[1] = 'U';
        month[2] = 'G';
    }
    else if (day <= 273){
        month[0] = 'S';
        month[1] = 'E';
        month[2] = 'P';
    }
    else if (day <= 304){
        month[0] = 'O';
        month[1] = 'C';
        month[2] = 'T';
    }
    else if (day <= 335){
        month[0] = 'N';
        month[1] = 'O';
        month[2] = 'V';
    }
    else if (day > 335){
        month[0] = 'D';
        month[1] = 'E';
        month[2] = 'C';
    }

    timeArray[0] = month[0];
    timeArray[1] = month[1];
    timeArray[2] = month[2];
}

void configDaysHrsMinSec(long unsigned int number,int timeType, unsigned char*timeArray){
    long unsigned int firstDig;
    long unsigned int secondDig;

    firstDig = number /10;
    secondDig = number %10;

    char firstNum = (char)(firstDig+'0');
    char secondNum = (char)(secondDig+'0');

    //Days
    if (timeType ==  3){
        if (number <= 31){
            timeArray[3] = (char)(58);
            timeArray[4] = firstNum;
            timeArray[5] = secondNum;
        }
        else if (number <= 59){
            //Value for last variable in method: Jan = 31 days = 31
            defineDays(number, firstDig, secondDig, 31);
        }
        else if (number <= 90){
            //Value for last variable in method: Feb = 28 days; 31+28 = 59
            defineDays(number, firstDig, secondDig, 59);
        }
        else if (number <= 120){
            //Value for last variable in method: March = 31 days; 59+31 = 90
            defineDays(number, firstDig, secondDig, 90);
        }
        else if (number <= 151){
            //Value for last variable in method: April = 30 days; 90+30 = 120
            defineDays(number, firstDig, secondDig, 120);
        }
        else if (number <= 181){
            //Value for last variable in method: May = 31 days; 120+31 = 151
            defineDays(number, firstDig, secondDig, 151);
        }
        else if (number <= 212){
            //Value for last variable in method: June = 30 days; 151+30 = 181
            defineDays(number, firstDig, secondDig, 181);
        }
        else if (number <= 243){
            //Value for last variable in method: July = 31 days; 181+31 = 212
            defineDays(number, firstDig, secondDig, 212);
        }
        else if (number <= 273){
            //Value for last variable in method: August = 31 days; 212+31 = 243
            defineDays(number, firstDig, secondDig, 243);
        }
        else if (number <= 304){
            //Value for last variable in method: September = 30 days; 243+30 = 273
            defineDays(number, firstDig, secondDig, 273);
        }
        else if (number <= 335){
            //Value for last variable in method: October = 31 days; 273+31 = 304
            defineDays(number, firstDig, secondDig, 304);
        }
        else if (number > 335){
            //Value for last variable in method: December = 31 days; 304+31 = 335
            defineDays(number, firstDig, secondDig, 335);
        }
    }
    //Hours
    if (timeType ==  4){
        timeArray[6] = (char)(58);
        timeArray[7] = firstNum;
        timeArray[8] = secondNum;
    }
    //Minutes
    if (timeType ==  5){
        timeArray[9] = (char)(58);
        timeArray[10] = firstNum;
        timeArray[11] = secondNum;
    }
    //Seconds
    if (timeType ==  6){
        timeArray[12] = (char)(58);
        timeArray[13] = firstNum;
        timeArray[14] = secondNum;
    }
}

void defineDays(long unsigned int number, long unsigned int firstDig, long unsigned int secondDig, long unsigned int days){
    number = number - days;

    firstDig = number /10;
    secondDig = number %10;

    char firstNum = (char)(firstDig+'0');
    char secondNum = (char)(secondDig+'0');

    timeArray[3] = (char)(58);
    timeArray[4] = firstNum;
    timeArray[5] = secondNum;
}

void displayTime(long unsigned int initialTime){

    long unsigned int seconds;
    long unsigned int modularSec;
    long unsigned int minutes;
    long unsigned int hour;
    long unsigned int day;

    //Seconds in a day
    day = (initialTime / 86400) +1;
    configDaysHrsMinSec(day, 3, timeArray);
    configMonth(day);
    modularSec = initialTime % 86400;

    //Seconds in an hour
    hour = modularSec / 3600;
    configDaysHrsMinSec(hour, 4,timeArray);
    modularSec = modularSec % 3600;

    //Seconds in a minute
    minutes = modularSec / 60;
    configDaysHrsMinSec(minutes, 5,timeArray);
    modularSec = modularSec % 60;

    //Seconds
    seconds = modularSec;
    configDaysHrsMinSec(seconds, 6,timeArray);
}

/****************************************************
************** METHODS FOR THE TEMP *****************
****************************************************/
void floatToString(float valueToInt, int timeType, unsigned char*tempArray){
    int value = (int)(valueToInt);
    int firstDig;
    int secondDig;

    firstDig = value/10;
    secondDig = value%10;

    char firstNum = (char)(firstDig+'0');
    char secondNum = (char)(secondDig+'0');

    if (timeType == 1){
        tempArray[0] = firstNum;
        tempArray[1] = secondNum;
        tempArray[2] = (char)(67);
    }
    else if(timeType == 2){
        tempArray[3] = (char)(32);
        tempArray[4] = firstNum;
        tempArray[5] = secondNum;
        tempArray[6] = (char)(70);
    }
}

void displayTemp(float inAvgTempC){
    volatile float degC_per_bit;
    volatile unsigned int bits30, bits85;

    //This will reset REFMSTR to hand over control of
    //the internal reference voltages to ADC12_A control registers
    REFCTL0 &= ~REFMSTR;

    //Internal reference voltage = 1.5V
    ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON;

    //This will enable the sample timer
    ADC12CTL1 = ADC12SHP;

    //Using ADC12MEM0 to store reading
    ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10;  //ADC i/p ch A10 = temp sense
    // ACD12SREF_1 = internal ref = 1.5v

    //Enables the conversion
    ADC12CTL0 |= ADC12ENC;

    //Use calibration data stored in info memory
    bits30 = CALADC12_15V_30C;
    bits85 = CALADC12_15V_85C;
    degC_per_bit = ((float)(85.0 - 30.0))/((float)(bits85-bits30));

    //This will clear the start bit
    ADC12CTL0 &= ~ADC12SC;

    //This is where the sampling and conversion start
    //with only a single conversion (single channel)
    ADC12CTL0 |= ADC12SC;

    // Poll busy bit waiting for conversion to complete
    while (ADC12CTL1 & ADC12BUSY){__no_operation();}

    // Read in results if conversion
    inputTemperature = ADC12MEM0;

    // Temp in Celsius:
    temperatureDegC = (float)((long)inputTemperature - CALADC12_15V_30C) * degC_per_bit +30.0;
    floatToString(temperatureDegC, 1, tempArray);

    // Temp in Fahrenheit:
    temperatureDegF = (float)(((9/5)*temperatureDegC) + 32);
    floatToString(temperatureDegF, 2, tempArray);


    //This is the code to create the temperature average
    tempAverageC += temperatureDegC;
    tempAverageF += temperatureDegF;

    if(averageCount == 10){
        tempAverageC = (float)(tempAverageC / 10);
        tempAverageF = (float)(tempAverageF / 10);
        floatToString(tempAverageC, 1, tempAvgArray);
        floatToString(tempAverageF, 2, tempAvgArray);
        averageCount = 1;
    }
    averageCount++;
    //Inserts a NOP instruction into the code stream.
    __no_operation();

}
