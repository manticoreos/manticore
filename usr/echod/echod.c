/*
 * UDP echo server
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_PORT 7777

static void die(const char *msg)
{
	fprintf(stderr, "error: %s: %s\n", msg, strerror(errno));
	abort();
}

static void echo(int sockfd)
{
	const size_t maxbuf = 65535;
	char buf[maxbuf];
	struct sockaddr_storage addr = {};
	socklen_t addrlen = sizeof(addr);
	size_t nr = recvfrom(sockfd, buf, maxbuf, MSG_DONTWAIT, (struct sockaddr*) &addr, &addrlen);
	if (nr < 0) {
		die("recv");
	}
	if (sendto(sockfd, buf, nr, MSG_DONTWAIT, (struct sockaddr*) &addr, addrlen) != (ssize_t) nr) {
		die("send");
	}
}

int main(int argc, char *argv[])
{
	int port = DEFAULT_PORT;

	fprintf(stdout, "Echo server listening to port %d ...\n", port);

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		die("sockfd");
	}
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr.s_addr = INADDR_ANY,
	};
	if (bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
		die("bind");
	}
	int efd = epoll_create1(0);
	if (efd < 0) {
		die("epoll_create1");
	}
	struct epoll_event ev = {
		.events = EPOLLIN | EPOLLHUP,
		.data.fd = sockfd,
	};
	if (epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &ev) < 0) {
		die("epoll_ctl");
	}
	for (;;) {
		const int maxevents = 64;
		struct epoll_event events[maxevents];
		int nr = epoll_wait(efd, events, maxevents, -1);
		if (nr < 0) {
			die("epoll_wait");
		}
#if 1
		fprintf(stderr, "epoll_wait() returned %d events\n", nr);
#endif
		for (int i = 0; i < nr; i++) {
			struct epoll_event *ev = &events[i];
			echo(ev->data.fd);
		}
	}
	close(sockfd);
	return 0;
}
