#include "math.h"

#define FREQ .1
#define OFFSET 2047
#define AMPLITUDE 2000
#define PI 3.141592
#define MESSAGE_FRAMES 1
#define BUFFER_LENGTH 622

const uint message_byte_length = MESSAGE_FRAMES*BUFFER_LENGTH;

int ii = 0;
int jj = 0;

uint value_out = 0;
uint value0 = 0;
uint value1 = 0;
uint value2 = 0;

float fvalue_out = 0;
float fvalue0 = 0;
float fvalue1 = 0;
float fvalue2 = 0;

float t = 0;
float t_last = 0;

int l1 = 0;
int l2 = 0;

String ADC0_text ="";
String dummy="";

uint done=0;

void setup()
{
    pinMode(DAC1, OUTPUT);
Serial.begin(115200);
Particle.publish("oscilloscope started","1");
}

void loop() 
{
    t = (float)micros()/1000000;
    value_out = (int)(sin(t*2*PI*FREQ)*AMPLITUDE+OFFSET);
    analogWrite(DAC1, value_out);

value0 = analogRead(A0);
value1 = analogRead(A1);
value2 = analogRead(A2);


fvalue_out = (float)value_out*3.3/4095;
fvalue0 = (float)value0*3.3/4095;
fvalue1 = (float)value1*3.3/4095;
fvalue2 = (float)value2*3.3/4095;

Serial.print(String::format("%.3f,%.3f,%.3f,%.3f,%.3f\r\n",t,fvalue_out,fvalue0,fvalue1,fvalue2));

    l1 = ADC0_text.length();

    if (done==0)
    {
        {
            //value0 = analogRead(A0);
            dummy = String::format("%.3f,%.3f;",t,fvalue0);
            l2 = dummy.length();
            if ((l1+l2)<message_byte_length) 
            {
                ADC0_text.concat(dummy);
            }
            else
            {
                done=1;
            }
        }
    }
    
    if ((t-t_last)>1)
    {
        Particle.publish("ADC0",ADC0_text.substring(jj*BUFFER_LENGTH,jj*BUFFER_LENGTH+BUFFER_LENGTH));
        jj++;
        if (jj>=MESSAGE_FRAMES)
        {
            jj=0;
            ii=0;
            ADC0_text =String();
            done = 0;
        }
        t_last = t;
    }
}
