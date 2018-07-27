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

static int listen_socket(int port);
static void echo(int sock, char opt[]);
static void do_command(char* cmd, char *res);
int main(int argc, char * argv[]){
	int i;
	int sock, server;
	char opt[128];
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof addr;
	server = listen_socket(argc > 1? atoi(argv[1]): DEFAULT_PORT);
	
	for(i = 2; i < argc; i++){
		strcat(opt, " ");
		strcat(opt, argv[i]);
	}
	while(1){
		sock = accept(server, (struct sockaddr*)&addr, &addrlen);
		if(sock <0){
			perror("accept(2)");
			exit(1);
		}
		if(fork()==0){
			echo(sock, opt);
			exit(0);
		}else{
			close(sock);
		}
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

static void do_command(char *command, char *msg){
	int i;
	FILE *fp;
	char c;
	if((fp=popen(command, "r")) == NULL){
		perror("cowsay is not be able to execute");
		exit(EXIT_FAILURE);
	}
	for(i = 0; (c = getc(fp)) != EOF ; i++){
		msg[i] = c;
	}
	fclose(fp);
} 

static void echo(int sock, char opt[]){
	while(1){
		char get_string[64] = "", *it;
		int i, rec_size=0;
		rec_size = recv(sock, get_string, 128, 0);
		if(rec_size == 0) break;
		else if(rec_size == -1){
			perror("recieve error");
			exit(EXIT_FAILURE);
		}else{
			printf("%s", get_string);
			char say[128] = "", msg[512]="", cmd[256+20]="";
			for(i = 0, it = get_string; *it != '\0'; it++){
				if(*it == '$'||*it == '`' || *it == '"' || *it == '\\')
					say[i++] = '\\';
				say[i++] = *it;
			} 
			strcat(cmd, "/usr/games/cowsay \" ");
			strcat(cmd, say);
			strcat(cmd, " \"");
			strcat(cmd, opt);
			do_command(cmd, msg);
			write(sock, msg, sizeof(msg));
		}
	}
	close(sock);
}
