#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>

#define LED1 18
#define LED2 23
#define SW1 10

void DisplayState(int state)
{
    switch (state)
    {
    case 1:
        digitalWrite(LED1, 1);
        digitalWrite(LED2, 0);
        break;
    case 2:
        digitalWrite(LED1, 0);
        digitalWrite(LED2, 1);
        break;
    case 3:
        digitalWrite(LED1, 1);
        digitalWrite(LED2, 1);
        break;
    case 4:
        digitalWrite(LED1, 0);
        digitalWrite(LED2, 0);
    default:
        break;
    }
}
void main(void)
{
    int State = 0;
    int i = 0;
    If(wiringPiSetup() < 0)
    {
        exit(0);
    }
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(SW1, INPUT);
    State = 1;
    while (State <= 4)
    {
        DisplayState(State);
        while (1)
        {
            if (digitalRead(SW1) == 0)
            {
                break;
            }
        }
        while (1)
        {
            if (digitalRead(SW1) == 1)
            {
                break;
            }
        }
        State++;
    }
    DisplayState(1);
}
