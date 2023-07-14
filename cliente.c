/* Bibliotecas Utilizadas */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 2048

int main(int argc, char **argv)
{
	/* Ponteiro para a tabela de entrada do host */
	struct hostent *hostTable;

	/* Estrutura para o servidor */
	int serverSocket;
	struct sockaddr_in serverAddress;

	/* Variáveis para salvar o nome do host e o numero da porta */
	char *host;
	short port;

	/* Buffer de envio e recebimento */
	char buffer[BUFFER_SIZE];
	char receiveBuffer[BUFFER_SIZE];

	/* Variáveis para controle da quantidade de bytes recebidos e enviados */
	int received;

	if (argc < 3)
	{
		perror("\nCÓDIGO: EC01\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n"); 
		exit(-1);
	}

	host = argv[1];
	port = atoi(argv[2]);
	if(port < 5000)
	{
		perror("\nCÓDIGO: EC02\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n"); 
		exit(-1);
	}

	/* Atribuindo o host da tabela de hosts a estrutura do servidor */
	hostTable = gethostbyname(host);
	if(hostTable == NULL)
	{
		perror("\nCÓDIGO: EC03\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n");
		exit(-1);
	}

	do
	{
		/* Cria socket IPV4 */
		serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if(serverSocket == -1)
		{
			perror("\nCÓDIGO: EC04\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n"); 
			close(serverSocket);
			exit(-1);
		}

		/* Mapeando valores da estrutura do servidor */
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(port);
		serverAddress.sin_addr.s_addr = *((unsigned long *)hostTable->h_addr);

		memset(&serverAddress.sin_zero, 0, sizeof(serverAddress.sin_zero));

		if(connect(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		{
			perror("\nCÓDIGO: EC05\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n"); 
			close(serverSocket);
			exit(-1);
		}

		printf("> ");
		fflush(stdout);
		fgets(buffer, BUFFER_SIZE, stdin);
		fflush(stdin);
		buffer[strlen(buffer)-1] = '\0';

		if(strcmp(buffer, "exit") == 0)
		{
			close(serverSocket);
			exit(-1);
		}

		send(serverSocket, buffer, strlen(buffer), 0);

		/* Parte do Servidor */
		received = recv(serverSocket, receiveBuffer, sizeof(buffer), 0);
		receiveBuffer[received] = '\0';
		printf("%s\n", receiveBuffer);

		close(serverSocket);
	} while (1);
	
	return 0;
}