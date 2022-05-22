#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <math.h>

using namespace std;

struct sockaddr_in receiver_addr;
int sock_fd;
bool openSocketAT(){
   sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   receiver_addr.sin_family = AF_INET;
	 if( inet_aton( "192.168.1.1",  &receiver_addr.sin_addr )== 0) {
	   close(sock_fd);
		return false;
	 }
   receiver_addr.sin_port = htons( 5556 );
   return true;
}


struct sockaddr_in receiver_addr_nav;
int sock_fd_nav;
bool openSocketNav(){
   sock_fd_nav = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   receiver_addr_nav.sin_family = AF_INET;
	 if( inet_aton( "192.168.1.1",  &receiver_addr_nav.sin_addr )== 0) {
	   close(sock_fd_nav);
		return false;
	 }
   receiver_addr_nav.sin_port = htons( 5554 );
   return true;
}

void sendAT(char* line){
   sendto(sock_fd, line, strlen(line), 0,(struct sockaddr*)&receiver_addr,sizeof(receiver_addr));
}


template <typename T>
void MakeValueFromOffset(T& var, unsigned char* buffer, int offset){
   memcpy(&var, buffer + offset, sizeof(var));
}

struct nav_data_struct{
   int32_t battery;
   
   // In Degrees
   float pitch;
   float roll;
   float yaw;
   
   // In meters
   float altitude;
   
   // In m/s
   float speedX;
   float speedY;
   float speedZ;
} nav_data;


void parseNavigation(unsigned char* buffer){
   static int32_t header, batteryLevel, altitude, state, sequence;
   static float pitch, roll, yaw, speedX, speedY, speedZ;
   
   MakeValueFromOffset(header, buffer, 0);
   
   if(header != 0x55667788){
      printf("ERROR: Flase Nav Data\n");
      return;
   }
   
   MakeValueFromOffset(state, buffer, 4);
   
   MakeValueFromOffset(sequence, buffer, 4);
   
   MakeValueFromOffset(batteryLevel, buffer, 24);
   MakeValueFromOffset(pitch, buffer, 28);
   MakeValueFromOffset(roll, buffer, 32);
   MakeValueFromOffset(yaw, buffer, 36);
   MakeValueFromOffset(altitude, buffer, 40);
   MakeValueFromOffset(speedX, buffer, 44);
   MakeValueFromOffset(speedY, buffer, 48);
   MakeValueFromOffset(speedZ, buffer, 52);
   
   nav_data.battery = batteryLevel;
   
   nav_data.pitch = pitch/1000.0f;
   nav_data.roll = roll/1000.0f;
   nav_data.yaw = yaw/1000.0f;
   
   nav_data.altitude = altitude/1000.0f;
   
   nav_data.speedX = speedX;
   nav_data.speedY = speedY;
   nav_data.speedZ = speedZ;
   
   //~ printf("sequence  = %u\n", sequence);
   
   printf("battery   = %u\n", nav_data.battery);
   //~ printf("altitude  = %f\n", nav_data.altitude);
   
   printf("roll      = %f\n", nav_data.roll);
   printf("pitch     = %f\n", nav_data.pitch);
   printf("yaw       = %f\n", nav_data.yaw);
   
   //~ printf("speedX    =  %f\n", nav_data.speedX);
   //~ printf("speedY    =  %f\n", nav_data.speedY);
   //~ printf("speedZ    =  %f\n", nav_data.speedZ);
   
   static int i=0;
   printf("%u\n",i++);
}


int main () {
   if(openSocketAT()){
      printf("AT:: Connection Established Successfully\n");
   }else{
      printf("AT:: Failed establish connection\n");
      return -1;
   }
   
   if(openSocketNav()){
      printf("NAV:: Connection Established Successfully\n");
   }else{
      printf("NAV:: Failed establish connection\n");
      return -1;
   }
   
   const unsigned char trigger[4] = {0x01, 0x00, 0x00, 0x00};
   sendto(sock_fd_nav, trigger, 4, 0,(struct sockaddr*)&receiver_addr_nav,sizeof(receiver_addr_nav));
   
   unsigned char navDataDemo[10240];
   unsigned int navDataLength = 10240;
   
   sendAT("AT*CONFIG=1,\"general:navdata_demo\",\"TRUE\"\r");
   
   printf("Entering Blocking code\n");
   while(1){
      recv(sock_fd_nav, navDataDemo, navDataLength, 0);
      parseNavigation(navDataDemo);
      //~ sendAT("AT*COMWDG=1\r");
      usleep(30*1000);
   }
   

   printf("Closing Socket\n");
   printf("Bye Bye\n");
   close(sock_fd);
   return 0;
}
