#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <stdint.h>

#define PORT 5556

using namespace std;


/* Encodes 32-bit float in 32-bin int */
uint32_t encodeFloat(float f){
   return *(uint32_t *)(&f);
}

/* Decodes 32-bit float from 32-bin int */
float decodeFloat(uint32_t i){
   return *(float *)(&i);
}

int main () {
   printf("Hello\n");
   printf("%u\n",encodeFloat(-0.3));
   printf("%f\n",decodeFloat(1061997773));
   printf("%f\n",decodeFloat(1061997773));
}
