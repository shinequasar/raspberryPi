#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#define TRUE 1
#define TRIG 4 //초음파센서 출력핀
#define ECHO 5 //초음파센서 입력핀
#define BUZ 12;

void setup()
{
    if(wiringPiSetup() == -1)
    {
        fprintf(stderr, "Wpi setup failed.\n");
        exit(0);
    }
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    // TRIG pin must start LOW
    digitalWrite(TRIG, LOW);
    delay(30);
}

int getCM()
{
    // Send Trigger Pulse
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(20);
    digitalWrite(TRIG, LOW);
    // Wait for echo start
    while(digitalRead(ECHO) == LOW)
    {
        continue;
    }
    // Wait for echo end
    long startTime = micros();
    while(digitalRead(ECHO) == HIGH)
    {
        continue;
    }
    long travelTime = micros() - startTime;
    // Get distance in cm
    int distance = travelTime / 58;
    return distance;
}

int main(void)
{
    setup();
    while(1)
    {
        int dist = getCM();
        fprintf(stderr, "Distance: %d cm\n", dist);
        delay(1000);

        if(20 < dist <= 30){
            pinMode(BUZ, PWM_OUTPUT);
        } 
        if(10 < dist <= 20){
            
        }
        if(0 < dist <= 10){
            
        }



    }
    return 0;
}
