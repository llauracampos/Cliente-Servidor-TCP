#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef struct {
	int numsock;
	char ip[INET_ADDRSTRLEN];
} Dados;

int clientes[100];
int n = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *Monitor(void *str_in){

    char buff[600];

    strcpy(buff, (char*)str_in);

    pthread_mutex_lock(&mutex);

    FILE *arquivo;
    arquivo = fopen("eventos.txt", "a");
    fprintf(arquivo,"%s", buff);
    fclose(arquivo);

    pthread_mutex_unlock(&mutex);

}

void Enviar(char *msg,int curr){

	int i = 0;
	pthread_mutex_lock(&mutex);

	while(i < n){
		if(clientes[i] != curr){
			if(send(clientes[i],msg,strlen(msg),0) < 0){
				perror("Falha no envio");
				continue;
			}
		}
		i++;
	}
	pthread_mutex_unlock(&mutex);
}

void *GerenciaThread(void *sock){

	Dados cl = *((Dados *)sock);
	char msg[500];
	char mensagem[500];
	pthread_t tmonitor;
	int len;
	int i = 0;
	int j;

	while((len = recv(cl.numsock, msg, 500, 0)) > 0){
		msg[len] = '\0';

		printf("Mensagem de [IP: %s]: %s", cl.ip, msg);

		strcpy(mensagem, "Mensagem de [IP: ");
		strcat(mensagem, cl.ip);
		strcat(mensagem, "]: ");
		strcat(mensagem, msg);
		strcat(mensagem, "\n");

		pthread_create(&tmonitor, NULL, Monitor, &mensagem);

		Enviar(msg, cl.numsock);
		memset(msg, '\0', sizeof(msg));
	}

	pthread_mutex_lock(&mutex);
	printf("Cliente [IP: %s] desconectado\n", cl.ip);

	strcpy(mensagem, "Cliente [IP: ");
	strcat(mensagem, cl.ip);
	strcat(mensagem, "] desconectado\n\n");

	pthread_create(&tmonitor, NULL, Monitor, &mensagem);

	while(i < n){
		if(clientes[i] == cl.numsock) {
			j = i;
			while(j < n-1){
				clientes[j] = clientes[j+1];
				j++;
			}
		}
		i++;
	}
	n--;

	pthread_mutex_unlock(&mutex);
}
int main(int argc,char *argv[]){

	struct sockaddr_in end1, end2;
	int sock1;
	int sock2;
	socklen_t end2_size;
	int numport;
	pthread_t tenvia, trecebe;
	pthread_t tmonitor;
	char msg[500];
	char mensagem2[500];
	int len;
	Dados cl;
	char ip[INET_ADDRSTRLEN];

	if(argc > 2){
		printf("Muitos argumentos");
		exit(1);
	}

	numport = atoi(argv[1]);
	sock1 = socket(AF_INET, SOCK_STREAM,0);
	memset(end1.sin_zero,'\0', sizeof(end1.sin_zero));
	end1.sin_family = AF_INET;
	end1.sin_port = htons(numport);
	end1.sin_addr.s_addr = inet_addr("127.0.0.1");
	end2_size = sizeof(end2);

	if(bind(sock1,(struct sockaddr *)&end1, sizeof(end1)) != 0){
		perror("Conexão sem sucesso");
		exit(1);
	}

	if(listen(sock1,5) != 0){
		perror("Escuta sem sucesso");
		exit(1);
	}

	FILE *arquivo;
	arquivo = fopen("eventos.txt", "w");
	fclose(arquivo);

	strcpy(mensagem2, "Iniciando o servidor na porta: ");
	strcat(mensagem2, argv[1]);
	strcat(mensagem2, "\n\n");
	strcat(mensagem2, "Aguardando novos clientes\n\n");

	printf("%s", mensagem2);

	pthread_create(&tmonitor, NULL, Monitor, &mensagem2);

	while(1){
		if((sock2 = accept(sock1,(struct sockaddr *)&end2, &end2_size)) < 0){
			perror("Recebimento sem sucesso");
			exit(1);
		}

		pthread_mutex_lock(&mutex);
		inet_ntop(AF_INET, (struct sockaddr *)&end2, ip, INET_ADDRSTRLEN);
		printf("Cliente [IP: %s] conectado\n", ip);

        strcpy(mensagem2, "Cliente [IP: ");
        strcat(mensagem2, ip);
        strcat(mensagem2, "] conectado\n\n");

        pthread_create(&tmonitor, NULL, Monitor, &mensagem2);

		cl.numsock = sock2;
		strcpy(cl.ip, ip);
		clientes[n] = sock2;
		n++;
		pthread_create(&trecebe, NULL, GerenciaThread, &cl);
		pthread_mutex_unlock(&mutex);
	}

	return 0;
}
