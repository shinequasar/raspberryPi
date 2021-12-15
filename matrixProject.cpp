/*

 max7219.c
 
 Raspberry Pi driving the Max7219

 to compile : gcc max7219.c -o max7219 -lwiringPi


*/
 
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


// define our pins :

#define DATA        0 // GPIO 17 (WiringPi pin num 0)  header pin 11
#define CLOCK       3 // GPIO 22 (WiringPi pin num 3)   header pin 15
#define LOAD        4 // GPIO 23 (WiringPi pin num 4)   header pin 16


// The Max7219 Registers :

#define DECODE_MODE   0x09                       
#define INTENSITY     0x0a                        
#define SCAN_LIMIT    0x0b                        
#define SHUTDOWN      0x0c                        
#define DISPLAY_TEST  0x0f                         



static void Send16bits (unsigned short output)
{
   
  unsigned char i;

  for (i=16; i>0; i--) 
  {
    unsigned short mask = 1 << (i - 1); // calculate bitmask
  
    digitalWrite(CLOCK, 0);  // set clock to 0
    
    // Send one bit on the data pin
    
    if (output & mask)   
      digitalWrite(DATA, 1);          
      else                              
      digitalWrite(DATA, 0);  
        
    digitalWrite(CLOCK, 1);  // set clock to 1
      
  }

}


// Take a reg numer and data and send to the max7219
// 8x8 한개면 data1만 하는데 8x32로 4개짜리라 data4까지
static void MAX7219Send (unsigned char reg_number, unsigned short data1,unsigned short data2,unsigned short data3,unsigned short data4)
{
  digitalWrite(LOAD, 1);  // set LOAD 1 to start
  Send16bits((reg_number << 8) + data1);   // send 16 bits ( reg number + dataout )
  Send16bits((reg_number << 8) + data2);   // send 16 bits ( reg number + dataout )
  Send16bits((reg_number << 8) + data3);   // send 16 bits ( reg number + dataout )
  Send16bits((reg_number << 8) + data4);   // send 16 bits ( reg number + dataout )
  digitalWrite(LOAD, 0);  // LOAD 0 to latch
  digitalWrite(LOAD, 1);  // set LOAD 1 to finish
}




int main (void)
{
   
  printf ("\n\nRaspberry Pi Max7219 Test using WiringPi\n\n");

  if (wiringPiSetup () == -1) exit (1) ;

  //We need 3 output pins to control the Max7219: Data, Clock and Load

  pinMode(DATA, OUTPUT);  
  pinMode(CLOCK, OUTPUT);
  pinMode(LOAD, OUTPUT);  
 
  MAX7219Send(SCAN_LIMIT, 7,7,7,7);     // set up to scan all eight digits


int heart[9][9] = { 
    {0, 16, 56, 124, 254, 238, 68, 0, 0 }, 
    { 16, 56, 124, 254, 238, 68, 0, 0 ,0 },
    { 56, 124, 254, 238, 68, 0, 0 ,0, 16 },
    { 124, 254, 238, 68, 0, 0 ,0, 16 ,56 },
    { 254, 238, 68, 0, 0 ,0, 16 ,56 ,124 },
    { 238, 68, 0, 0 ,0, 16 ,56 ,124, 254 },
    { 68, 0, 0 ,0, 16 ,56 ,124, 254, 238 },
    { 0, 0 ,0, 16 ,56 ,124, 254, 238, 68 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }};
    // 0, 16, 56, 124, 254, 238, 68, 0, 0 //하트

for(;;){//무한루프
    for(int i=0; i<8; i++) {

        //intensity 1,5,10,15로 하면 "오른쪽부터" 밝기 1,5,10,15로 설정됨
        MAX7219Send(DECODE_MODE, 0,0,0,0);   // Set BCD decode mode on
        MAX7219Send(DISPLAY_TEST, 0,0,0,0);  // Disable test mode
        MAX7219Send(INTENSITY, 1,1,1,1);     // set brightness 0 to 15
        MAX7219Send(SHUTDOWN, 1,1,1,1);      // come out of shutdown mode   / turn on the digits

        //행으로 8개 사용 for문 돌려서 내려가는거 구현한거
        //오른쪽으로 흘려가게 해주는걸 생각해줘야함 
        
        MAX7219Send(1, heart[i][0],heart[i][0],heart[i][0],heart[i][0]);
        MAX7219Send(2, heart[i][1],heart[i][1],heart[i][1],heart[i][1]);
        MAX7219Send(3, heart[i][2],heart[i][2],heart[i][2],heart[i][2]);
        MAX7219Send(4, heart[i][3],heart[i][3],heart[i][3],heart[i][3]);
        MAX7219Send(5, heart[i][4],heart[i][4],heart[i][4],heart[i][4]);
        MAX7219Send(6, heart[i][5],heart[i][5],heart[i][5],heart[i][5]);
        MAX7219Send(7, heart[i][6],heart[i][6],heart[i][6],heart[i][6]);
        MAX7219Send(8, heart[i][7],heart[i][7],heart[i][7],heart[i][7]);

        // 이렇게 하면 첫번째 8x8 하트만 아래로 흘러감
        //   MAX7219Send(1, 0,0,0,heart[i][0]);
        //   MAX7219Send(2, 0,0,0,heart[i][1]);
        //   MAX7219Send(3, 0,0,0,heart[i][2]);
        //   MAX7219Send(4, 0,0,0,heart[i][3]);
        //   MAX7219Send(5, 0,0,0,heart[i][4]);
        //   MAX7219Send(6, 0,0,0,heart[i][5]);
        //   MAX7219Send(7, 0,0,0,heart[i][6]);
        //   MAX7219Send(8, 0,0,0,heart[i][7]);


        delay(1000);
    }
}

  return 0;
}