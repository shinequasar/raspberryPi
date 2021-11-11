#include <stdio.h>
#include <wiringPi.h>

#define SW1 10
#define SW2 11
int main(void)
{
    int rcv1, rcv2 = 0;
    If (wiringPiSetup() < 0) {
        exit(0);
    }

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(SW1, INPUT);
    pinMode(SW2, INPUT);

    while (1) {
        rcv1 = digitalRead(SW1);
        digitalWrite(LED1, rcv1);
        rcv2 = digitalRead(SW2);
        digitalWrite(LED2, rcv2);
    }
    return 0;
}