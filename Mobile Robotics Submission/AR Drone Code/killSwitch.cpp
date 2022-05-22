/* Emergency Kill Switch
 * To Stop the Drone */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5556

struct sockaddr_in receiver_addr;
int sock_fd;

bool openSocket(){
   sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   receiver_addr.sin_family = AF_INET;
	 if( inet_aton( "192.168.1.1",  &receiver_addr.sin_addr )== 0) {
	   close(sock_fd);
		return false;
	 }
   receiver_addr.sin_port = htons( PORT );
   return true;
}

void sendAT(char* line){
   sendto(sock_fd, line, strlen(line), 0,(struct sockaddr*)&receiver_addr,sizeof(receiver_addr));
}

int main () {
   if(openSocket()){
      printf("Connection Established Successfully\n");
   }else{
      printf("Failed establish connection\n");
      return -1;
   }

  printf("Emergency Landing\n");
  
   for(int i=0;i<1000;i++){
      sendAT("AT*REF=1,290717696\r");
      usleep(20*1000);
   }
   
   printf("Ending Program");
}
