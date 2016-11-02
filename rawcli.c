#include <stdio.h>
#include <stdlib.h> /* exit(), atoi() */
#include <sys/socket.h>
#include <string.h> /* bzero, bcopy, strlen */
#include <linux/if_ether.h> /* ETH_P_ALL */
#include <linux/if_packet.h>
#include <linux/if_arp.h>
#include <netinet/in.h>
#include <unistd.h>

void error(const char* msg);

int main(short argc, char* argv[]) {
  int sockfd, j;
  struct sockaddr_ll servaddr;
  char check[] = "123";
  char data[256 - sizeof(check)];
  char packet[256];
  char answer[1000];
  unsigned char self_mac[6] = {0x00, 0x01, 0x02, 0xFA, 0x70, 0xAA};
  unsigned char dest_mac[6] = {0x00, 0x04, 0x75, 0xC8, 0x28, 0xE5};

  /* creating */
  sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if(sockfd < 0)
    error("Error on creating a socket");

  /* setting the things up */
  servaddr.sll_family = PF_PACKET;
  servaddr.sll_protocol = htons(ETH_P_IP);
  servaddr.sll_ifindex = 2;
  servaddr.sll_hatype = ARPHRD_ETHER;
  servaddr.sll_pkttype = PACKET_OTHERHOST;
  servaddr.sll_halen = ETH_ALEN;
  
  for(j = 0; j < 6; j++)
    servaddr.sll_addr[j] = dest_mac[j];

  servaddr.sll_addr[6] = 0x00;
  servaddr.sll_addr[7] = 0x00;
 
  printf("Please enter the command: ");
  bzero(data, 256);
  fgets(data, 255, stdin);

  strcpy(packet, check);
  strcat(packet, data);

  /* sending */
  for(j = 0; j < 30; j++) {
    printf("%i) Sending...\n", j);

    if(sendto(sockfd, packet, 256, 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
      error("Error on sending");
    sleep(1);
  }

  /* receiving answer */
  socklen_t l = sizeof servaddr;

  for(j = 0; j < 200; j++) {
    printf("%i) Waiting for answer...\n", j);

    if(recvfrom(sockfd, answer, 1000, 0, (struct sockaddr*)&servaddr, &l) < 0)
      error("Error on receiving");
    if(strncmp(check, answer, strlen(check)) == 0)
      break;
    strcpy(answer, "123Could not receive the answer");
    sleep(1);

  }

  memmove(answer, answer + sizeof(check) - 1, 
    sizeof(answer) - sizeof(check) + 1);
  printf("Answer: %s\n", answer);

  close(sockfd);

  return 0;
}

void error(const char* msg) {
  perror(msg);
  exit(1);
}
