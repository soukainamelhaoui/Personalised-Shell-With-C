#include<stdio.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include <stdlib.h>
#include<time.h>
int h, m;
void chrono (int sig)
{
system("paplay ./alarm1.wav");
}
void main()
{
// Current Time
time_t now = time(NULL);
struct tm *tm_struct = localtime(&now);
int hour = tm_struct->tm_hour;
int minute = tm_struct->tm_min;
// Alarm Time
printf("Entrer l'heure: \n");
scanf("%d", &h);
printf("Entrer minute: \n");
scanf("%d", &m);
printf("Alarm is set for %d:%d...\n", h, m);
// Turn to Sec
int second = hour*3600 + minute*60;
int s = h*3600 + m*60;
signal(SIGALRM, chrono);
alarm(s-second);
pause();
}
