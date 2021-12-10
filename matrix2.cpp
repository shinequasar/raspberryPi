#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
 
#include <wiringPi.h>
 
#include "max7219.h"
 
void intHandler(int dummy)
{
    send_MAX7219(SHUTDOWN, 0);
    exit(0);
}
 
int main(int argc, char** argv)
{
    signal(SIGINT, intHandler);
    
    if( wiringPiSetup() < 0 )
    {
        return 1;
    }
 
    pinMode(DATA, OUTPUT);
    pinMode(CLOCK, OUTPUT);
    pinMode(LOAD, OUTPUT);
 
    send_MAX7219(SCAN_LIMIT, 7);
    send_MAX7219(DECODE_MODE, 0);
    send_MAX7219(INTENSITY, 1);
    send_MAX7219(SHUTDOWN, 1);
 
    int digit_table[11] = { 126, 48, 109, 121, 51, 91, 95, 114, 127, 123, 0 };
    int dot_table[9][9] =
    { // 128 64 32 16 8 4 2 1
    { 0, 0, 0, 0, 15, 0, 0, 0, 0 }, // N
    { 0, 1, 1, 1, 1, 1, 1, 1, 0 }, // 1
    { 0, 15, 1, 1, 15, 8, 8, 15, 0 }, // 2
    { 0, 15, 1, 1, 15, 1, 1, 15, 0 }, // 3
    { 0, 9, 9, 9, 15, 1, 1, 1, 0 }, // 4
    { 0, 15, 8, 8, 15, 1, 1, 15, 0 }, // 5
    { 0, 15, 8, 8, 15, 9, 9, 15, 0 }, // 6
    { 0, 15, 9, 9, 9, 1, 1, 1, 0 }, // 7
    { 0, 0, 0, 0, 15, 8, 8, 8, 0 }, // R
    };
    
    // send_MAX7219(1, digit_table[0], dot_table[0][1]);
    // send_MAX7219(2, digit_table[0], dot_table[0][2]);
    // send_MAX7219(3, digit_table[0], dot_table[0][3]);
    // send_MAX7219(4, digit_table[0], dot_table[0][4]);
    // send_MAX7219(5, digit_table[0], dot_table[0][5]);
    // send_MAX7219(6, digit_table[0], dot_table[0][6]);
    // send_MAX7219(7, digit_table[0], dot_table[0][7]);
    // send_MAX7219(8, digit_table[0], dot_table[0][8]);

    // send_MAX7219(1, digit_table[0], dot_table[1][1]);
    // send_MAX7219(2, digit_table[0], dot_table[1][2]);
    // send_MAX7219(3, digit_table[0], dot_table[1][3]);
    // send_MAX7219(4, digit_table[0], dot_table[1][4]);
    // send_MAX7219(5, digit_table[0], dot_table[1][5]);
    // send_MAX7219(6, digit_table[0], dot_table[1][6]);
    // send_MAX7219(7, digit_table[0], dot_table[1][7]);
    // send_MAX7219(8, digit_table[0], dot_table[1][8]);
 
    send_MAX7219(1, digit_table[0]);
    // send_MAX7219(2, digit_table[1]);
    // send_MAX7219(3, digit_table[2]);
    // send_MAX7219(4, digit_table[3]);
    // send_MAX7219(5, digit_table[4]);
    // send_MAX7219(6, digit_table[5]);
    // send_MAX7219(7, digit_table[6]);
    // send_MAX7219(8, digit_table[7]);


    //intHandler(0);
    return 0;
}
