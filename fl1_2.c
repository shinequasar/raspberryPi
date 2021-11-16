#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>


#define SW1 10
#define SW2 11
#define LED1 1
#define LED2 4
#define LED3 5
#define LED4 6

int main(void)
{
    int rcv1, rcv2 = 0;
    if(wiringPiSetup() < 0){
        exit(0);
    }

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);

    pinMode(SW1, INPUT);
    pinMode(SW2, INPUT);

    while(1){
        rcv1 = digitalRead(SW1);
        digitalWrite(LED1, rcv1);
        digitalWrite(LED2, rcv1);
        rcv2 = digitalRead(SW2);
        digitalWrite(LED3, rcv2);
        digitalWrite(LED4, rcv2);
    }
    return 0;
}