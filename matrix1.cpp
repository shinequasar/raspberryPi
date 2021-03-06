#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
 
#include <wiringPi.h>
 
#include "matrix_h.h"
 
void intHandler(int dummy)
{
    send_MAX7219(SHUTDOWN, 0, 0);
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
 
    send_MAX7219(SCAN_LIMIT, 7, 7);
    send_MAX7219(DECODE_MODE, 0, 0);
    send_MAX7219(INTENSITY, 1, 1);
    send_MAX7219(SHUTDOWN, 1, 1);
 
    int digit_table[11] = { 126, 48, 109, 121, 51, 91, 95, 114, 127, 123, 0 };
    int dot_table[9][9] =
    { // 128 64 32 16 8 4 2 1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // N
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 2
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 3
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 4
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 5
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 6
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 7
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // R
    };
    
    send_MAX7219(1, digit_table[0], dot_table[0][1]);
    send_MAX7219(2, digit_table[0], dot_table[0][2]);
    send_MAX7219(3, digit_table[0], dot_table[0][3]);
    send_MAX7219(4, digit_table[0], dot_table[0][4]);
    send_MAX7219(5, digit_table[0], dot_table[0][5]);
    send_MAX7219(6, digit_table[0], dot_table[0][6]);
    send_MAX7219(7, digit_table[0], dot_table[0][7]);
    send_MAX7219(8, digit_table[0], dot_table[0][8]);
 
    //intHandler(0);
    return 0;
}