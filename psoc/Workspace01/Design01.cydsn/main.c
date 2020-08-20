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
#include "project.h"
#include <math.h>
#include <stdio.h>
#define AMPLITUDE 127
#define OFFSET 127
#define FREQUENCY 10
#define PI 3.141592

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    myUSB_Start(0,myUSB_5V_OPERATION);
    myADC_SEQ_Start();
    myDAC_Start();
    
    char message[255]="hi";
    uint ii=0;
    double t=0;
    double y=0;
    uint16 value0=0;
    uint16 value1=0;
    uint16 value2=0;
    float faout=0;
    float fvalue0=0;
    float fvalue1=0;
    float fvalue2=0;
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
     
    
    t = ((float)ii)/1000;
    y = AMPLITUDE*sin(2*PI*t*FREQUENCY)+OFFSET;
    
    myDAC_SetValue((int)y);
    myADC_SEQ_StartConvert(); // Start ADC conversion
    myADC_SEQ_IsEndConversion(myADC_SEQ_WAIT_FOR_RESULT);
// Wait until conversion is completed
    value0=myADC_SEQ_GetResult16(0);
    value1=myADC_SEQ_GetResult16(1);
    value2=myADC_SEQ_GetResult16(2);
    faout = (float)y*1.024/255;
    fvalue0 = (float)value0*5/4095;
    fvalue1 = (float)value1*5/4095;
    fvalue2 = (float)value2*5/4095;
    //value = 255;
    //t=3.2;
    sprintf(message,"%.3f,%.3f,%.3f,%.3f,%.3f\r\n",t,faout,fvalue0,fvalue1,fvalue2);
    //sprintf(message,"%d\n",value);
    
    //while (!myUSB_CDCIsReady());
    //myUSB_PutString("\nvalue:");
    while (!myUSB_CDCIsReady());
    myUSB_PutString(message);
    ii++;
    CyDelay(1);
    }
}

/* [] END OF FILE */
