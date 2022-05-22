#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5556

void blink(){
   struct sockaddr_in receiver_addr;
   int sock_fd;
   char line[] = "AT*LED=1,0,1073741824,4\r"; // Blinks LED
   sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   receiver_addr.sin_family = AF_INET;
	 if( inet_aton( "192.168.1.1",  &receiver_addr.sin_addr )== 0) {
		printf("Crap!, Init failed\n");
	   close(sock_fd);
		return;
	 }
   receiver_addr.sin_port = htons( PORT );
   sendto(sock_fd, line, strlen(line), 0,(struct sockaddr*)&receiver_addr,sizeof(receiver_addr));
   close(sock_fd);
}

int main () {
  printf("Start of Blinker\n");
  blink();
  printf("End of Blinker\n");
}
