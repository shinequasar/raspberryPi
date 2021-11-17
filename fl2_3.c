#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>

#define LED1 1

void pwm(double on, double off)
{
    for(int n = 0; n < 30; n++)
    {
        digitalWrite(LED1, 1);
        delay(on * 1000);
        digitalWrite(LED1, 0);
        delay(off * 1000);
    }
}
void main(void)
{
    double sub = 0.0;
    if(wiringPiSetup() < 0){
        exit(0);
    }
    pinMode(LED1, OUTPUT);
    for(int i = 0; i < 35; i++)
    {
        sub = i * 0.05;
        pwm(2.0 - sub, 1.0);
    }
}
