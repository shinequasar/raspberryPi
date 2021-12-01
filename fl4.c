#include <wiringPi.h>
#include <stdio.h>
#define CDS 0 /* wiringPi 0, GPIO17 */
#define LED 1 /* wiringPi 1, GPIO18 */

int cdsControl()
{
    int i;
    for (i = 0; i < 10000000; i++)
    {
        if(digitalRead(CDS) == HIGH) /* 빛이감지되면(HIGH) */
        {
            digitalWrite(LED, HIGH); /* LED On */
            delay(1000);
            digitalWrite(LED, LOW); /* LED Off */
        }
    }
    return 0;
}

int main()
{
    if(wiringPiSetup() == -1)
    {
        fprintf(stderr, "WiringPi setup failed....\n");
        exit(0);
    }
    pinMode(CDS, INPUT);  /* Pin 모드를 입력으로 설정 */
    pinMode(LED, OUTPUT); /* Pin 모드를 출력으로 설정 */
    cdsControl();
    return 0;
}