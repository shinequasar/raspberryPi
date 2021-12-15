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
        if(!strcmp(opt, "led") && !strcmp(var, "pattern1")) {
            turnOnLEDMatrix(1);
        } else if(!strcmp(opt, "led") && !strcmp(var, "pattern2")) {
            turnOnLEDMatrix(2);  
        } else if(!strcmp(opt, "led") && !strcmp(var, "pattern3")) {
            turnOnLEDMatrix(3);  
        } else if(!strcmp(opt, "led") && !strcmp(var, "pattern4")) {
            turnOnLEDMatrix(4); 
        }else if(!strcmp(opt, "led") && !strcmp(var, "pattern5")) {
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

void turnOnLEDMatrix(int pattern){
    switch(pattern){
        case 1 : //하트
            int turnOn[9][9] = { 
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
            break;
        case 2 : 
            int turnOn[9][9] ={ {0, 0, 56, 124, 0, 0, 68, 0, 0 }, 
                { 16, 56, 124, 0, 238, 68, 0, 0 ,0 },
                { 56, 124, 254, 0, 68, 0, 0 ,0, 16 },
                { 124, 254, 238, 68, 0, 0 ,0, 16 ,56 },
                { 254, 0, 68, 0, 0 ,0, 16 ,56 ,124 },
                { 0, 68, 0, 0 ,0, 16 ,56 ,124, 254 },
                { 68, 0, 0 ,0, 16 ,56 ,124, 254, 238 },
                { 0, 0 ,0, 16 ,56 ,124, 254, 238, 68 },
                { 0, 0, 0, 0, 0, 0, 0, 0, 0 }};
            break;  
        case 3 : 
            int turnOn[9][9] ={ {0, 16, 23, 124, 43, 1, 2, 0, 0 }, 
                { 16, 56, 124, 254, 238, 68, 0, 0 ,0 },
                { 56, 124, 254, 23, 68, 0, 0 ,0, 16 },
                { 124, 254, 238, 68, 0, 0 ,0, 16 ,56 },
                { 254, 238, 68, 0, 0 ,0, 16 ,56 ,124 },
                { 238, 68, 0, 0 ,0, 16 ,56 ,124, 254 },
                { 68, 0, 0 ,0, 16 ,56 ,124, 254, 238 },
                { 0, 0 ,0, 16 ,56 ,124, 254, 238, 68 },
                { 0, 0, 0, 0, 0, 0, 0, 0, 0 }};
            break; 
        case 4 : 
            int turnOn[9][9] ={ {0, 6, 4, 4, 54, 38, 8, 0, 0 }, 
                { 16, 56, 124, 254, 238, 68, 0, 0 ,0 },
                { 56, 124, 254, 2, 68, 0, 0 ,0, 16 },
                { 124, 254, 2, 68, 0, 0 ,0, 16 ,56 },
                { 254, 238, 68, 0, 0 ,0, 16 ,56 ,124 },
                { 238, 68, 0, 0 ,0, 16 ,0 ,124, 254 },
                { 68, 0, 0 ,0, 16 ,56 ,124, 254, 238 },
                { 0, 0 ,0, 16 ,56 ,124, 254, 238, 68 },
                { 0, 0, 0, 0, 0, 0, 0, 0, 0 }};
            break; 
        case 5 : 
            int turnOn[9][9] ={{ 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 {0, 0, 0, 0, 0, 0, 0, 0, 0 }};
            break; 
        default :
            printf(">>>> erroer"); 
            int turnOn[9][9] ={{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                 {0, 0, 0, 0, 0, 0, 0, 0, 0 }};   
    }

    for(;;){//무한루프
    for(int i=0; i<8; i++) {
        //intensity 1,5,10,15로 하면 "오른쪽부터" 밝기 1,5,10,15로 설정됨
        MAX7219Send(DECODE_MODE, 0,0,0,0);   // Set BCD decode mode on
        MAX7219Send(DISPLAY_TEST, 0,0,0,0);  // Disable test mode
        MAX7219Send(INTENSITY, 1,1,1,1);     // set brightness 0 to 15
        MAX7219Send(SHUTDOWN, 1,1,1,1);      // come out of shutdown mode   / turn on the digits

        //행으로 8개 사용 for문 돌려서 내려가는거 구현한거
        //오른쪽으로 흘려가게 해주는걸 생각해줘야함 
        
        MAX7219Send(1, turnOn[i][0],turnOn[i][0],turnOn[i][0],turnOn[i][0]);
        MAX7219Send(2, turnOn[i][1],turnOn[i][1],turnOn[i][1],turnOn[i][1]);
        MAX7219Send(3, turnOn[i][2],turnOn[i][2],turnOn[i][2],turnOn[i][2]);
        MAX7219Send(4, turnOn[i][3],turnOn[i][3],turnOn[i][3],turnOn[i][3]);
        MAX7219Send(5, turnOn[i][4],turnOn[i][4],turnOn[i][4],turnOn[i][4]);
        MAX7219Send(6, turnOn[i][5],turnOn[i][5],turnOn[i][5],turnOn[i][5]);
        MAX7219Send(7, turnOn[i][6],turnOn[i][6],turnOn[i][6],turnOn[i][6]);
        MAX7219Send(8, turnOn[i][7],turnOn[i][7],turnOn[i][7],turnOn[i][7]);

        delay(200);
        }
    }
}
