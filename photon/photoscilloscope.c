//include math library
#include "math.h"

// set user defined constants that we will use throughout the code
#define FREQ .1
#define OFFSET 2047
#define AMPLITUDE 2000
#define PI 3.141592
#define MESSAGE_FRAMES 1
#define BUFFER_LENGTH 622

// create a constant variable for message length computed from two other constants
const uint message_byte_length = MESSAGE_FRAMES*BUFFER_LENGTH;


// create variables for iterating through for loops and similar
int ii = 0;
int jj = 0;

// define variables for raw write to DAC
uint value_out = 0;

//define variables for raw read from ADC 0-3
uint value0 = 0;
uint value1 = 0;
uint value2 = 0;

//floating point versions of above in Volts.
float fvalue_out = 0;
float fvalue0 = 0;
float fvalue1 = 0;
float fvalue2 = 0;


//define variables for time and the last time a message was sent
float t = 0;
float t_last = 0;

// intermediate variables for computing current message length
int l1 = 0;
int l2 = 0;

//the string reaading values from the current cycle
String ADC0_text ="";

//temp string variable
String dummy="";

//flag for stopping adding to string
uint done=0;

//this function is run once when the microcontroller begins
void setup()
{
    pinMode(DAC1, OUTPUT);
    Serial.begin(115200);
    Particle.publish("oscilloscope started","1");
}

//this function is run continuously, about once per ms
void loop() 
{
    //read the current microseconds and convert to t in seconds
    t = (float)micros()/1000000;
    //compute the value out
    value_out = (int)(sin(t*2*PI*FREQ)*AMPLITUDE+OFFSET);
    //write the value out
    analogWrite(DAC1, value_out);

    //read the ADC 0-3 values
    value0 = analogRead(A0);
    value1 = analogRead(A1);
    value2 = analogRead(A2);
    
    
    //convert the above values to Volts
    fvalue_out = (float)value_out*3.3/4095;
    fvalue0 = (float)value0*3.3/4095;
    fvalue1 = (float)value1*3.3/4095;
    fvalue2 = (float)value2*3.3/4095;
    
    //print to serial
    Serial.print(String::format("%.3f,%.3f,%.3f,%.3f,%.3f\r\n",t,fvalue_out,fvalue0,fvalue1,fvalue2));
    
    //compute the current length of ADC0_text
    l1 = ADC0_text.length();

    //if I am not done adding to ADC0_text,
    if (done==0)
    {
        {
            // create a new dummy string
            dummy = String::format("%.3f,%.3f;",t,fvalue0);
            //measure the length of the string
            l2 = dummy.length();
            //if the total length is not longer than the maximum message length in bytes,
            if ((l1+l2)<message_byte_length) 
            {
                //add dummy to ADC0_text
                ADC0_text.concat(dummy);
            }
            //otherwise
            else
            {
                //mark done
                done=1;
            }
        }
    }
    
    //if my current time is at least one second since I last published to the particle cloud,
    if ((t-t_last)>1)
    {
        //publish frame jj of ADC0_text to the cloud
        Particle.publish("ADC0",ADC0_text.substring(jj*BUFFER_LENGTH,jj*BUFFER_LENGTH+BUFFER_LENGTH));
        //increment frame jj
        jj++;
        //if I am at the end of my frames:
        if (jj>=MESSAGE_FRAMES)
        {
            //clear out my counters
            jj=0;
            ii=0;
            //clear out ADC0_text
            ADC0_text =String();
            //clear done flag
            done = 0;
        }
        //set t_last to current t.
        t_last = t;
    }
}
