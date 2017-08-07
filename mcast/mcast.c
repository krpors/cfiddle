#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

static void multicast_send() {
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("Open socket");
		exit(2);
	}
	printf("Socket opened\n");

	struct sockaddr_in groupsock;
	memset(&groupsock, 0, sizeof(struct sockaddr_in));
	groupsock.sin_family = AF_INET;
	groupsock.sin_port = htons(4321);
	groupsock.sin_addr.s_addr = inet_addr("239.100.100.100");

	struct in_addr localIf;
	localIf.s_addr = INADDR_ANY;

	if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &localIf, sizeof(struct in_addr)) == -1) {
		perror("Unable to set socket option...");
		close(sockfd);
		exit(1);
	}

	printf("Socket option set!\n");

	char* buf = "Hello there";
	if (sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&groupsock, sizeof(struct sockaddr_in)) < 0) {
		perror("Could not send shit.");
	}

	close(sockfd);
}

static void multicast_recv() {
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("Open socket");
		exit(2);
	}
	printf("Socket opened\n");

	const int reuse = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
		perror("Could not set SO_REUSEADDR");
		close(sockfd);
		exit(1);
	}

	struct sockaddr_in localsock;
	memset(&localsock, 0, sizeof(struct sockaddr_in));
	localsock.sin_family = AF_INET;
	localsock.sin_port = htons(4321);
	localsock.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr*)&localsock, sizeof(struct sockaddr_in)) == -1) {
		perror("Can't bind");
		exit(1);
	}
	printf("Socket bound\n");

	struct ip_mreqn req;
	inet_pton(AF_INET, "239.100.100.100", &(req.imr_multiaddr.s_addr));
	req.imr_address.s_addr = INADDR_ANY;
	req.imr_ifindex = 0;

	if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &req, sizeof(struct ip_mreqn)) == -1) {
		perror("Whoops.");
		exit(1);
	}
	printf("Joined multicast group and now receiving...\n");

	char buf[1024] = {0};
	while (true) {
		int len = recv(sockfd, &buf, 1024, 0);
		if (len == -1) {
			perror("Could not receive...");
			close(sockfd);
			exit(1);
		}
		printf("Buf is %s\n", buf);
	}

	close(sockfd);
}


int main(int argc, char* argv[]) {
	(void)(argc);
	(void)(argv);

	if (argc != 2) {
		fprintf(stderr, "gimme -receiver or -sender\n");
		exit(1);
	}
	
	if (strcmp(argv[1], "receiver") == 0) {
		multicast_recv();
	}
	if (strcmp(argv[1], "sender") == 0) {
		multicast_send();
	}
}
