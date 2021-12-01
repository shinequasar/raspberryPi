#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#define TRUE 1
#define TRIG 4 //초음파센서 출력핀
#define ECHO 5 //초음파센서 입력핀
#define BUZZER 12;

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

void makeSound(int time){
    digitalWrite(BUZZER,LOW);
    delay(time);
    digitalWrite(BUZZER,HIGH);
    delay(time);           
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
            makeSound(7);
        } 
        else if(10 < dist <= 20){
             makeSound(4);
        }
        else if(0 < dist <= 10){
             makeSound(1);
        }else digitalWrite(BUZZER,LOW);
    }
    return 0;
}
