/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

//Include standard libraries
#include <math.h>
#include <stdio.h>
#include "project.h"

//Define Constants for the script
#define AMPLITUDE 127
#define OFFSET 127
#define FREQUENCY 10
#define PI 3.141592
#define DACBITS 255
#define ADCBITS 4095
#define ADCREF 5
#define DACREF 1.024

int main(void)
{
    //the following code gets run once
    
    //enable global interrupts
    CyGlobalIntEnable;
    //Start the USB subsystem
    myUSB_Start(0,myUSB_5V_OPERATION);
    //Start the ADC Sequencer
    myADC_SEQ_Start();
    //Start the DAC subsystem
    myDAC_Start();
    
    //create a new empty message string
    char message[255]="";
    //create a counter ii for iterating
    uint ii=0;
    //creaate variable t for counting time
    double t=0;
    //create variable y for storing the DAC value to write
    double y=0;
    //create variables to hold ADC read    
    uint16 value0=0;
    uint16 value1=0;
    uint16 value2=0;
    
    //create floating point variables to store floating poing voltages.
    float faout=0;
    float fvalue0=0;
    float fvalue1=0;
    float fvalue2=0;

    for(;;)
    {
        //define t as a function of the number of loops divided by 1000
        t = ((float)ii)/1000;
        
        //compute y as a function of sine
        y = AMPLITUDE*sin(2*PI*t*FREQUENCY)+OFFSET;
        
        //set the DAC value
        myDAC_SetValue((int)y);
        
        //Begin ADC read
        myADC_SEQ_StartConvert();
        //wait for adc conversion to complete.
        myADC_SEQ_IsEndConversion(myADC_SEQ_WAIT_FOR_RESULT);

        //save ADC reads to local variables
        value0=myADC_SEQ_GetResult16(0);
        value1=myADC_SEQ_GetResult16(1);
        value2=myADC_SEQ_GetResult16(2);
        
        //convert uint to float for writing to serial
        faout = (float)y*DACREF/DACBITS;
        fvalue0 = (float)value0*ADCREF/ADCBITS;
        fvalue1 = (float)value1*ADCREF/ADCBITS;
        fvalue2 = (float)value2*ADCREF/ADCBITS;
        
        //convert floating point values to string and store in message pointer
        sprintf(message,"%.3f,%.3f,%.3f,%.3f,%.3f\r\n",t,faout,fvalue0,fvalue1,fvalue2);

        //wait until myUSB subsystem is ready
        while (!myUSB_CDCIsReady());
        //write message to USB
        myUSB_PutString(message);
        
        //increment ii
        ii++;
        //pause 1 ms
        CyDelay(1);
    }
}

/* [] END OF FILE */
