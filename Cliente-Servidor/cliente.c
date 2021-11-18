#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

void *GerenciaThread(void *sock){

	int sock2 = *((int *)sock);
	char msg[500];
	int len;

	while((len = recv(sock2,msg,500,0)) > 0){
		msg[len] = '\0';
		fputs(msg, stdout);
		memset(msg,'\0', sizeof(msg));
	}
}

int main(int argc, char *argv[]){

	struct sockaddr_in end2;
	int sock1;
	int sock2;
	int end2_size;
	int numport;
	pthread_t tenvia, trecebe;
	char msg[500];
	char usuario[100];
	char res[600];
	char ip[INET_ADDRSTRLEN];
	char ipserv[20];
	int len;

	if(argc > 4){
		printf("Muitos argumentos!");
		exit(1);
	}

	numport = atoi(argv[2]);
	strcpy(ipserv, argv[3]);
	strcpy(usuario,argv[1]);
	sock1 = socket(AF_INET, SOCK_STREAM,0);
	memset(end2.sin_zero,'\0', sizeof(end2.sin_zero));
	end2.sin_family = AF_INET;
	end2.sin_port = htons(numport);
	end2.sin_addr.s_addr = inet_addr(ipserv);

	if(connect(sock1,(struct sockaddr *)&end2, sizeof(end2)) < 0){
		perror("Conexão não estabelecida");
		exit(1);
	}

	inet_ntop(AF_INET, (struct sockaddr *)&end2, ip, INET_ADDRSTRLEN);
	printf("%s [IP: %s] conectado na porta %d \nIniciando o Chat!\n", usuario, ipserv, numport);

	pthread_create(&trecebe, NULL, GerenciaThread, &sock1);

	while(fgets(msg, 500, stdin) > 0){
        if(!strcmp(msg, "SAIR\n")){
			close(sock1);
			exit(0);
		}

		strcpy(res, usuario);
		strcat(res, ": ");
		strcat(res, msg);

		printf("%s", res);

		len = write(sock1, res, strlen(res));

		if(len < 0){
			perror("Mensagem não enviada");
			exit(1);
		}

		memset(msg,'\0', sizeof(msg));
		memset(res,'\0', sizeof(res));
	}

	pthread_join(trecebe, NULL);
	close(sock1);

}
