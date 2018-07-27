/*
 *cowecho.c -- original echo server
 * 
 * Copy right (c) 2018 Yohei Saito
 * This program is fork of daytimed.c
 *
 *daytimed.c -- daytime server
 *
 * Copy right (c) 2005 Minero Aoki
 * This program is free software
 * Redistribution and use in source and binary forms
 * with or withdout modification are permitted.
 */

#if defined(__digital__) && defined(__unix__)
#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#define _XOPEN_SOURCE 500
#define _OSF_SOURCE

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define DEFAULT_PORT 7

static const char* caw = 
"    \\   ^__^\n\
     \\  (oo)\\_______\n\
        (__)\\       )\\/\\\n\
             ||----w |\n\
             ||     ||\n";

static int listen_socket(int port);
void communicate(int sock){
	while(1){
		char get_string[1024] = "";
		int rec_size=0;
		int n;
		rec_size = recv(sock, get_string, 128, 0);
		get_string[rec_size - 2] = '\0';
		n = rec_size;
		if(rec_size == 0) break;
		else if(rec_size == -1){
			perror("recieve error");
			exit(EXIT_FAILURE);
		}else{
			int i;
			char* bar;
			bar = (char *) malloc(n+4);
			bar[0] = ' ';
			bar[1] = ' ';
			char send_str[512] = "";
			for(i = 0; i < n; i++)
				bar[i+2] = '-';
			strcat(bar, "\r\n");

			strcat(send_str, bar);
			strcat(send_str, " < ");
			strcat(send_str, get_string);
			strcat(send_str, " >\r\n");
			strcat(send_str, bar);
			strcat(send_str, caw);
			write(sock, send_str, sizeof(send_str));
		}
	}
	close(sock);
}
int main(int argc, char * argv[]){
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof addr;
	int sock, server;
	server = listen_socket(argc > 1? atoi(argv[1]): DEFAULT_PORT);
start:
	sock = accept(server, (struct sockaddr*)&addr, &addrlen);
	if(sock <0){
		perror("accept(2)");
		exit(1);
	}
	if(fork()==0){
		communicate(sock);
	}else{
		close(sock);
		goto start;
	}
	close(server);
	exit(0);
}

static int listen_socket(int port){
	struct addrinfo hints, *res, *ai;
	int err;
	char service[16];
	int yes = 1;

	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints. ai_flags = AI_PASSIVE;
	snprintf(service, sizeof service, "%d", port);
	if((err = getaddrinfo(NULL, service, &hints, &res)) != 0){
		fprintf(stderr, "%s\n", gai_strerror(err));
		exit(1);
	}
	for(ai = res; ai; ai = ai->ai_next){
		int sock;
		sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if(sock <0) continue;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &yes, sizeof(yes));
		if(bind(sock, ai->ai_addr, ai->ai_addrlen)<0){
			close(sock);
			continue;
		}
		if(listen(sock, 5) < 0){
			close(sock);
			continue;
		}
		freeaddrinfo(res);
		fprintf(stderr, "listing on port %d...\n", port);
		return sock;
	}
	fprintf(stderr, "cannot listen socket\n");
	exit(1);
}
