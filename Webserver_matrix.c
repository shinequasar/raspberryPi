#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <wiringPi.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
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

static void *clnt_connection(void *arg);
int sendData(int fd, FILE *fp, char *file_name);
void sendOk(FILE *fp);
void sendError(FILE *fp);
void turnOnLEDMatrix(int pattern);
void startLED(int turnOnLED[][9]);

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


int main(int argc, char **argv)
{
    int serv_sock;
    pthread_t tid;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;

    if(argc != 2) {
        printf("usage : %s <PORT>\n", argv[0]);
        return -1;
    }
    
  printf ("\n\nRaspberry Pi Max7219 Test using WiringPi\n\n");
  if (wiringPiSetup () == -1) exit (1) ;
  //We need 3 output pins to control the Max7219: Data, Clock and Load
  pinMode(DATA, OUTPUT);  
  pinMode(CLOCK, OUTPUT);
  pinMode(LOAD, OUTPUT);  
  MAX7219Send(SCAN_LIMIT, 7,7,7,7);     // set up to scan all eight digits
    

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_sock < 0) {
        perror("socket");
        return -1;
    }
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    if(bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        return -1;
    }

    if(listen(serv_sock, 10) < 0) {
        perror("listen");
        return -1;
    }

    while(1) {
        int clnt_sock;
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        printf("Client IP : %s:%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

        pthread_create(&tid, NULL, clnt_connection, (void *)&clnt_sock);
        pthread_detach(tid);
    }

    return 0;
}

void *clnt_connection(void *arg)
{
    int clnt_sock = *((int *)arg), clnt_fd;
    FILE *clnt_read, *clnt_write;
    
    char reg_line[BUFSIZ], reg_buf[BUFSIZ];
    char method[10];
    char file_name[256];
    
    clnt_read = fdopen(clnt_sock, "r");
    clnt_write = fdopen(dup(clnt_sock), "w");
    clnt_fd = clnt_sock;

    fgets(reg_line, BUFSIZ, clnt_read);
    fputs(reg_line, stdout);

    strcpy(method, strtok(reg_line, " /"));
    if(strcmp(method, "POST") == 0) {
        sendOk(clnt_write);
        fclose(clnt_read);
        fclose(clnt_write);
        return NULL;
    }
    else if(strcmp(method, "GET") != 0) {
        sendError(clnt_write);
        fclose(clnt_read);
        fclose(clnt_write);
        return NULL;
    }
   
    strcpy(file_name, strtok(NULL, " /"));
    printf("file_name : %s\n", file_name);
    if(strstr(file_name, "?") != NULL) {
        // LED ��ư�� ������ submit�� �ߴٸ� ?led=On Ȥ�� ?led=Off��� �߼۵ȴ�
        char opt[8], var[8];
        strtok(file_name, "?");
        // led�� On/Off �и�
        strcpy(opt, strtok(NULL, "="));
        strcpy(var, strtok(NULL, "="));

        printf("%s=%s\n", opt, var);  //입력값 체크하는 부분
        if(!strcmp(opt, "led") && !strcmp(var, "p1")) {
            printf("start pattern : %s\n", var);
            turnOnLEDMatrix(1);
        } else if(!strcmp(opt, "led") && !strcmp(var, "p2")) {
            printf("start pattern : %s\n", var);
            turnOnLEDMatrix(2);
        } else if(!strcmp(opt, "led") && !strcmp(var, "p3")) {
            printf("start pattern : %s\n", var);
            turnOnLEDMatrix(3); 
        } else if(!strcmp(opt, "led") && !strcmp(var, "p4")) {
            printf("start pattern : %s\n", var);
            turnOnLEDMatrix(4); 
        }else if(!strcmp(opt, "led") && !strcmp(var, "p5")) {
            printf("start pattern : %s\n", var);
            turnOnLEDMatrix(5);
        }
        
    }
    
    // �� ������ 
    do {
        fgets(reg_line, BUFSIZ, clnt_read);
        fputs(reg_line, stdout);
        strcpy(reg_buf, reg_line);
    } while(strncmp(reg_line, "\r\n", 2));
    
    sendData(clnt_fd, clnt_write, file_name);
    
    fclose(clnt_read);
    fclose(clnt_write);
    return NULL;
}

int sendData(int fd, FILE *fp, char *file_name)
{
    if(strstr(file_name, "favicon") != NULL) return 0;
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server:Netscape-Enterprise\6.0\r\n";
    char cnt_type[] = "Content-Type:text/html\r\n";
    char end[] = "\r\n";
    char buf[BUFSIZ];
    int len;

    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_type, fp);
    fputs(end, fp);
    fflush(fp);

    fd = open(file_name, O_RDWR);
    do {
        len = read(fd, buf, BUFSIZ);
        fwrite(buf, len, sizeof(char), fp);
    } while(len == BUFSIZ);

    fflush(fp);
    close(fd);

    return 0;
}

void sendOk(FILE *fp)
{
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server: Netscape-Enterprise/6.0\r\n\r\n";

    fputs(protocol, fp);
    fputs(server, fp);
    fflush(fp);
}

void sendError(FILE *fp)
{
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server:Netscape-Enterprise\6.0\r\n";
    char cnt_len[] = "Content-Length:1024\r\n";
    char cnt_type[] = "Content-Type:text/html\r\n";
    char content1[] = "<html><head><title>BAD Connection</title></head>";
    char content2[] = "<body><font size=+5>BAD Request</font></body></html>";

    printf("send_error\n");
    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_len, fp);
    fputs(cnt_type, fp);
    fputs(content1, fp);
    fputs(content2, fp);

    fflush(fp);
}

// void startLED(int turnOnLED[][9]){
//     int turnOn[9][9];      
//     memcpy(turnOn, turnOnLED, sizeof(turnOnLED));

//     for(;;){//무한루프
//     for(int i=0; i<8; i++) {
//         //intensity 1,5,10,15로 하면 "오른쪽부터" 밝기 1,5,10,15로 설정됨
//         MAX7219Send(DECODE_MODE, 0,0,0,0);   // Set BCD decode mode on
//         MAX7219Send(DISPLAY_TEST, 0,0,0,0);  // Disable test mode
//         MAX7219Send(INTENSITY, 1,1,1,1);     // set brightness 0 to 15
//         MAX7219Send(SHUTDOWN, 1,1,1,1);      // come out of shutdown mode   / turn on the digits

//         //행으로 8개 사용 for문 돌려서 내려가는거 구현한거
//         //오른쪽으로 흘려가게 해주는걸 생각해줘야함 
        
//         MAX7219Send(1, turnOn[i][0],turnOn[i][0],turnOn[i][0],turnOn[i][0]);
//         MAX7219Send(2, turnOn[i][1],turnOn[i][1],turnOn[i][1],turnOn[i][1]);
//         MAX7219Send(3, turnOn[i][2],turnOn[i][2],turnOn[i][2],turnOn[i][2]);
//         MAX7219Send(4, turnOn[i][3],turnOn[i][3],turnOn[i][3],turnOn[i][3]);
//         MAX7219Send(5, turnOn[i][4],turnOn[i][4],turnOn[i][4],turnOn[i][4]);
//         MAX7219Send(6, turnOn[i][5],turnOn[i][5],turnOn[i][5],turnOn[i][5]);
//         MAX7219Send(7, turnOn[i][6],turnOn[i][6],turnOn[i][6],turnOn[i][6]);
//         MAX7219Send(8, turnOn[i][7],turnOn[i][7],turnOn[i][7],turnOn[i][7]);

//         delay(200);
//         }
//     }
// }

void turnOnLEDMatrix(int pattern){
    switch(pattern){
        case 1 : {
            bool onoff = true;
            printf(">>>> case 1"); 
            int heart[15][8] = { 
                    {0, 16, 56, 124, 254, 238, 68, 0}, 
                    {0, 32, 112, 248, 252, 220, 136, 0},
                    {0, 64, 224, 240, 248, 184, 16, 0},
                    {0, 128, 192, 224, 240, 112, 32, 0},
                    {0, 0, 128, 192, 224, 224, 64, 0},
                    {0, 0, 0, 128, 192, 192, 128, 0},
                    {0, 0, 0, 0, 128, 128, 0, 0},

                    {0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 1, 1, 0, 0},
                    {0, 0, 0, 1, 3, 3, 1, 0}, 
                    {0, 0, 1, 3, 7, 7, 2, 0},
                    {0, 1, 3, 7, 15, 14, 4, 0},
                    {0, 2, 7, 15, 31, 29, 8, 0},
                    {0, 4, 14, 31, 63, 59, 17, 0},
                    {0, 8, 28, 62, 127, 119, 34, 0},        
                    };
                while(onoff){
                    int num=0;
                    int j=0;
                    for(int i=0; i<15; i++) {                                   
                        j=num+7;
                        //intensity 1,5,10,15로 하면 "오른쪽부터" 밝기 1,5,10,15로 설정됨        
                        MAX7219Send(DECODE_MODE, 0,0,0,0);   // Set BCD decode mode on
                        MAX7219Send(DISPLAY_TEST, 0,0,0,0);  // Disable test mode
                        MAX7219Send(INTENSITY, 1,1,1,1);     // set brightness 0 to 15
                        MAX7219Send(SHUTDOWN, 1,1,1,1);      // come out of shutdown mode   / turn on the digits
                        
                        MAX7219Send(1, heart[j][0],heart[i][0],heart[j][0],heart[i][0]); // (행, 4,3,2,1)
                        MAX7219Send(2, heart[j][1],heart[i][1],heart[j][1],heart[i][1]);
                        MAX7219Send(3, heart[j][2],heart[i][2],heart[j][2],heart[i][2]);          
                        MAX7219Send(4, heart[j][3],heart[i][3],heart[j][3],heart[i][3]);        
                        MAX7219Send(5, heart[j][4],heart[i][4],heart[j][4],heart[i][4]);      
                        MAX7219Send(6, heart[j][5],heart[i][5],heart[j][5],heart[i][5]);          
                        MAX7219Send(7, heart[j][6],heart[i][6],heart[j][6],heart[i][6]);          
                        MAX7219Send(8, heart[j][7],heart[i][7],heart[j][7],heart[i][7]);  
                                
                        if(j==14){
                            num=-7;
                        }

                        delay(500);
                        num++; 
                    }
                    j--;
                    if(pattern != 1) break;
                    }
        }
            break;
        case 2 : {
        bool onoff = true;
            int heart[15][8] = { 
                {3, 3, 3, 3, 3, 3, 3, 3}, 
                {6, 6, 6, 6, 6, 6, 6, 6},
                {12, 12, 12, 12, 12, 12, 12, 12},
                {24, 24, 24, 24, 24, 24, 24, 24},
                {48, 48, 48, 48, 48, 48, 48, 48},
                {96, 96, 96, 96, 96, 96, 96, 96},
                {192, 192, 192, 192, 192, 192, 192, 192},

                {0, 0, 0, 0, 0, 0, 0, 0}, 
                {0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0, 0, 0},
                {3, 3, 3, 3, 3, 3, 3, 3},
                };
                while(onoff){
                    int num=0;
                    int j=0;
                    for(int i=0; i<15; i++) {                                   
                        j=num+7;
                        //intensity 1,5,10,15로 하면 "오른쪽부터" 밝기 1,5,10,15로 설정됨        
                        MAX7219Send(DECODE_MODE, 0,0,0,0);   // Set BCD decode mode on
                        MAX7219Send(DISPLAY_TEST, 0,0,0,0);  // Disable test mode
                        MAX7219Send(INTENSITY, 1,1,1,1);     // set brightness 0 to 15
                        MAX7219Send(SHUTDOWN, 1,1,1,1);      // come out of shutdown mode   / turn on the digits
                        
                        MAX7219Send(1, heart[j][0],heart[i][0],heart[j][0],heart[i][0]); // (행, 4,3,2,1)
                        MAX7219Send(2, heart[j][1],heart[i][1],heart[j][1],heart[i][1]);
                        MAX7219Send(3, heart[j][2],heart[i][2],heart[j][2],heart[i][2]);          
                        MAX7219Send(4, heart[j][3],heart[i][3],heart[j][3],heart[i][3]);        
                        MAX7219Send(5, heart[j][4],heart[i][4],heart[j][4],heart[i][4]);      
                        MAX7219Send(6, heart[j][5],heart[i][5],heart[j][5],heart[i][5]);          
                        MAX7219Send(7, heart[j][6],heart[i][6],heart[j][6],heart[i][6]);          
                        MAX7219Send(8, heart[j][7],heart[i][7],heart[j][7],heart[i][7]);  
                                
                        if(j==14){
                            num=-7;
                        }

                        delay(30);
                        num++; 
                    }
                    j--;
                    if(pattern != 1) break;
                    }
        }
            break;  
        case 3 : {
            bool onoff = true;
            int heart[15][8] = { 
                {0, 16, 32, 127, 32, 16, 0, 0}, 
                {0, 32, 64, 254, 64, 32, 0, 0},
                {0, 64, 128, 252, 128, 64, 0, 0},
                {0, 128, 0, 248, 0, 128, 0, 0},
                {0, 0, 0, 240, 0, 0, 0, 0},
                {0, 0, 0, 224, 0, 0, 0, 0},
                {0, 0, 0, 192, 0, 0, 0, 0},

                {0, 0, 0, 0, 0, 0, 0, 0}, 
                {0, 0, 0, 1, 0, 0, 0, 0},
                {0, 0, 1, 3, 1, 0, 0, 0},
                {0, 1, 2, 7, 2, 1, 0, 0},
                {0, 2, 4, 15, 4, 2, 0, 0},
                {0, 4, 8, 31, 8, 4, 0, 0},
                {0, 8, 16, 63, 16, 8, 0, 0},
                {0, 16, 32, 127, 32, 16, 0, 0},
                };
                
                 while(onoff){
                int num=0;
                int j=0;
                for(int i=0; i<15; i++) {                                   
                    j=num+7;
                    //intensity 1,5,10,15로 하면 "오른쪽부터" 밝기 1,5,10,15로 설정됨        
                    MAX7219Send(DECODE_MODE, 0,0,0,0);   // Set BCD decode mode on
                    MAX7219Send(DISPLAY_TEST, 0,0,0,0);  // Disable test mode
                    MAX7219Send(INTENSITY, 1,1,1,1);     // set brightness 0 to 15
                    MAX7219Send(SHUTDOWN, 1,1,1,1);      // come out of shutdown mode   / turn on the digits
                    
                    MAX7219Send(1, heart[j][0],heart[i][0],heart[j][0],heart[i][0]); // (행, 4,3,2,1)
                    MAX7219Send(2, heart[j][1],heart[i][1],heart[j][1],heart[i][1]);
                    MAX7219Send(3, heart[j][2],heart[i][2],heart[j][2],heart[i][2]);          
                    MAX7219Send(4, heart[j][3],heart[i][3],heart[j][3],heart[i][3]);        
                    MAX7219Send(5, heart[j][4],heart[i][4],heart[j][4],heart[i][4]);      
                    MAX7219Send(6, heart[j][5],heart[i][5],heart[j][5],heart[i][5]);          
                    MAX7219Send(7, heart[j][6],heart[i][6],heart[j][6],heart[i][6]);          
                    MAX7219Send(8, heart[j][7],heart[i][7],heart[j][7],heart[i][7]);  
                            
                    if(j==14){
                        num=-7;
                    }

                    delay(200);
                    num++; 
                }
                j--;
                if(pattern != 1) break;
                }
        }
            break; 
        case 4 : {
            bool onoff = true;
           int heart[16][8] = { 
                {0, 101, 21, 117, 87, 33, 1, 0}, 
                {0, 37, 85, 85, 37, 5, 5, 0},
                {0, 138, 85, 85, 128, 0, 0, 0},
                {0, 68, 69, 93, 68, 64, 0, 0},

                {0, 178, 138, 186, 171, 144, 128, 0},
                {0, 18, 170, 170, 18, 2, 2, 0},
                {0, 69, 170, 170, 64, 0, 0, 0},
                {0, 34, 162, 46, 34, 32, 32, 0}, 

                {0, 73, 85, 85, 73, 64, 64, 0},
                {0, 137, 85, 85, 9, 1, 1, 0},
                {0, 34, 85, 85, 32, 0, 0, 0},
                {0, 145, 81, 151, 17, 16, 16, 0},

                {0, 164, 170, 170, 164, 160, 160, 0},
                {0, 137, 85, 85, 9, 1, 1, 0},
                {0, 145, 170, 170, 144, 0, 0, 0},
                {0, 200, 168, 203, 136, 136, 136, 0},
                };
                
                while(onoff){      
                int i=0;
                for(int k=0; k<4; k++){
                    i=4*k;
                    //intensity 1,5,10,15로 하면 "오른쪽부터" 밝기 1,5,10,15로 설정됨        
                    MAX7219Send(DECODE_MODE, 0,0,0,0);   // Set BCD decode mode on
                    MAX7219Send(DISPLAY_TEST, 0,0,0,0);  // Disable test mode
                    MAX7219Send(INTENSITY, 1,1,1,1);     // set brightness 0 to 15
                    MAX7219Send(SHUTDOWN, 1,1,1,1);      // come out of shutdown mode   / turn on the digits
                    
                    MAX7219Send(1, heart[i+3][0],heart[i+2][0],heart[i+1][0],heart[i][0]); // (행, 4,3,2,1)
                    MAX7219Send(2, heart[i+3][1],heart[i+2][1],heart[i+1][1],heart[i][1]);
                    MAX7219Send(3, heart[i+3][2],heart[i+2][2],heart[i+1][2],heart[i][2]);          
                    MAX7219Send(4, heart[i+3][3],heart[i+2][3],heart[i+1][3],heart[i][3]);        
                    MAX7219Send(5, heart[i+3][4],heart[i+2][4],heart[i+1][4],heart[i][4]);      
                    MAX7219Send(6, heart[i+3][5],heart[i+2][5],heart[i+1][5],heart[i][5]);          
                    MAX7219Send(7, heart[i+3][6],heart[i+2][6],heart[i+1][6],heart[i][6]);          
                    MAX7219Send(8, heart[i+3][7],heart[i+2][7],heart[i+1][7],heart[i][7]);  

                    delay(1000);
                }
                if(pattern != 1) break;
            }
        }
           break; 
        case 5 : {
            bool onoff = true;
          int heart[9][9] =
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
                
                MAX7219Send(1, heart[0][0],heart[0][0],heart[0][0],heart[0][0]); // (행, 4,3,2,1)
                 MAX7219Send(2, heart[0][0],heart[0][0],heart[0][0],heart[0][0]);
                  MAX7219Send(3, heart[0][0],heart[0][0],heart[0][0],heart[0][0]);
                   MAX7219Send(4, heart[0][0],heart[0][0],heart[0][0],heart[0][0]);
                    MAX7219Send(5, heart[0][0],heart[0][0],heart[0][0],heart[0][0]);
                     MAX7219Send(6, heart[0][0],heart[0][0],heart[0][0],heart[0][0]);
                      MAX7219Send(7, heart[0][0],heart[0][0],heart[0][0],heart[0][0]);
                       MAX7219Send(8, heart[0][0],heart[0][0],heart[0][0],heart[0][0]);

        }
            break; 
        default :{
            printf(">>>> erroer"); 
            int heart[9][9] ={{ 0, 123, 0, 123, 0, 42, 0, 4, 0 }, 
                 { 0, 0, 43, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 234, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 344, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 3, 0, 20, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 {0, 0, 0, 0, 0, 0, 0, 0, 0 }};   
                  while(true){      
                        int i=0;
                        for(int k=0; k<4; k++){
                            i=4*k;
                            //intensity 1,5,10,15로 하면 "오른쪽부터" 밝기 1,5,10,15로 설정됨        
                            MAX7219Send(DECODE_MODE, 0,0,0,0);   // Set BCD decode mode on
                            MAX7219Send(DISPLAY_TEST, 0,0,0,0);  // Disable test mode
                            MAX7219Send(INTENSITY, 1,1,1,1);     // set brightness 0 to 15
                            MAX7219Send(SHUTDOWN, 1,1,1,1);      // come out of shutdown mode   / turn on the digits
                            
                            MAX7219Send(1, heart[i+3][0],heart[i+2][0],heart[i+1][0],heart[i][0]); // (행, 4,3,2,1)
                            MAX7219Send(2, heart[i+3][1],heart[i+2][1],heart[i+1][1],heart[i][1]);
                            MAX7219Send(3, heart[i+3][2],heart[i+2][2],heart[i+1][2],heart[i][2]);          
                            MAX7219Send(4, heart[i+3][3],heart[i+2][3],heart[i+1][3],heart[i][3]);        
                            MAX7219Send(5, heart[i+3][4],heart[i+2][4],heart[i+1][4],heart[i][4]);      
                            MAX7219Send(6, heart[i+3][5],heart[i+2][5],heart[i+1][5],heart[i][5]);          
                            MAX7219Send(7, heart[i+3][6],heart[i+2][6],heart[i+1][6],heart[i][6]);          
                            MAX7219Send(8, heart[i+3][7],heart[i+2][7],heart[i+1][7],heart[i][7]);  

                            delay(1000);
                        }
                    }
                 }
    }

}
