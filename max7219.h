// define pins for Raspberry PI
#define DATA        12
#define CLOCK       14
#define LOAD        10
 
// MAX7219 Registers
#define DECODE_MODE   0x09
#define INTENSITY     0x0a
#define SCAN_LIMIT    0x0b
#define SHUTDOWN      0x0c
#define DISPLAY_TEST  0x0f
 
void send_SPI_16bits(unsigned short data)
{
    for (int i = 16; i > 0; i--)
    {
        /* bitmask */
        unsigned short mask = 1 << (i - 1);
 
        /* send data */
        digitalWrite(CLOCK, 0);
        digitalWrite(DATA, (data & mask) ? 1 : 0);
        digitalWrite(CLOCK, 1);
    
        /* no receive data */
        //digitalWrite(CLOCK, 0);
    }
}
 
void send_MAX7219(unsigned short reg_number, unsigned short data1, unsigned short data2)
{
    digitalWrite(LOAD, 1);
    // send_SPI_16bits((reg_number << 8) + data2);
    send_SPI_16bits((reg_number << 8) + data1);
    digitalWrite(LOAD, 0); // to latch
    digitalWrite(LOAD, 1);
}