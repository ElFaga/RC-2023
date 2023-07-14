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
#include <semaphore.h>

/* Definição de constantes úteis */
#define MAX_CLIENTE 500
#define MAX_FILE 50
#define BUFFER_SIZE 2048

struct File
{
    char filename[50];
    char owner[50];
};

struct Usuario
{
    char nome[50];
};

/**** Variáveis globais ****/
/* Contagem de clientes */
int clientCount = 0; 

/* Semáforo para sincronizar as threads */
sem_t sinaleiro;

/* Protótipo da função de cada thread filha */
void *clientThread(void *param);

/* Vetor global para guardar os usuários */
struct Usuario allocatedUsers[MAX_CLIENTE];

/* Vetor global para guardar as informações dos arquivos do cliente */
struct File allocatedFiles[MAX_CLIENTE];

/* Função para verificar a existência de um cliente */
char *verifyRegister(char username[])
{
    char *tempStr, tempFilename[100], tempParagraph[BUFFER_SIZE];
    int paragraphCount = 0;

    FILE *writeFile;
    tempStr = malloc(sizeof(char)*BUFFER_SIZE);

     /* Verificar se o usuário existe */
     for(int i=0; i < MAX_CLIENTE; ++i)
     {
        /* Caso o usuário não exista */
        if((strcmp(username, allocatedUsers[i].nome)) != 0)
        {
            if(i+1 == MAX_CLIENTE)
            {
                strcpy(tempStr, "L - 1\n");
                return tempStr;
            }
        }
            
        /* Caso o usuário exista */
        if(strcmp(username, allocatedUsers[i].nome) == 0)
        {
            /* Colocar na tempString o L 0 */
            strcpy(tempStr, "L 0\n");
            /* Verificar o vetor de arquivos procurando os arquivos criados pelo mesmo */
            for(int j=0; j < MAX_CLIENTE; ++j)
            {
                paragraphCount = 0;
                /* Caso o usuario seja o dono, adiciona na tempStr */
                if(strcmp(username, allocatedFiles[j].owner) == 0)
                {
                    /* Colocar na variável temporária de arquivo, o nome do mesmo conforme foi gravado */
                    memset(tempFilename, 0, sizeof(tempFilename));
                    strcpy(tempFilename, allocatedFiles[j].filename);
                    strcat(tempFilename, ".txt");

                    writeFile = fopen(tempFilename, "r");

                    /* Contagem dos parágrafos do arquivo */
                    while((fgets(tempParagraph, BUFFER_SIZE, writeFile)) != NULL)
                        ++paragraphCount;

                    fclose(writeFile);
                    char count[4];
                    sprintf(count, "%d", paragraphCount);
                    /* Inicializar tempParagraph e concatenar as informações de cada linha */
                    memset(tempParagraph, 0, sizeof(tempParagraph));
                    strcat(tempParagraph, tempFilename);
                    strcat(tempParagraph, " ");
                    strcat(tempParagraph, count);
                    strcat(tempParagraph, " ");
                    strcat(tempParagraph, username);
                    strcat(tempParagraph, "\n");
                    strcat(tempStr, tempParagraph);
                }
            }
            return tempStr;
        }
     }
    return "-5";
}

/* Função para inserir novo registro de cliente */
int addRegister(char username[], char filename[])
{
    FILE *writeFile;
    char tempFilename[100];
    
    /* Verificar se existe algum usuário com o nome informado */
    for (int i=0; i<MAX_CLIENTE; ++i)
    {
        if ((strcmp(username, allocatedUsers[i].nome)) == 0)
        {
            /* Verificar se existe algum arquivo com o nome passado */
            for (int j=0; j<MAX_CLIENTE; j++)
            {
                if ((strcmp(filename, allocatedFiles[j].filename)) == 0)
                    return 2;
            }

            /* Criar arquivo caso não exista com o nome passado */
            memset(tempFilename, 0, sizeof(tempFilename));
            strcat(tempFilename, filename);
            strcat(tempFilename, ".txt");

            writeFile = fopen(tempFilename, "w");

            /* Verificar qual posição do vetor de arquivos está livre */
            for (int j=0; j<MAX_CLIENTE; j++)
            {
                if (allocatedFiles[j].filename[0] == '\0')
                {
                    strcpy(allocatedFiles[j].filename, filename);
                    strcpy(allocatedFiles[j].owner, username);

                    fclose(writeFile);
                    return 1;
                }
            }
        }
    }

    /* Nenhum usuário foi encontrado, precisa criar um novo usuário */
    for (int i=0; i<MAX_CLIENTE; i++)
    {
        if (allocatedUsers[i].nome[0] == '\0')
        {
            /* Verificar se existe algum arquivo com o nome passado */
            for (int j=0; j<MAX_CLIENTE; j++)
            {
                if ((strcmp(filename, allocatedFiles[j].filename)) == 0)
                {
                    strcpy(allocatedUsers[i].nome, username);
                    return 2;
                }  
            }

            /* Criar arquivo caso não exista com o nome passado */
            memset(tempFilename, 0, sizeof(tempFilename));
            strcat(tempFilename, filename);
            strcat(tempFilename, ".txt");

            writeFile = fopen(tempFilename, "w");

            /* Verificar qual posição do vetor de arquivos está livre */
            for (int j=0; j<MAX_CLIENTE; j++)
            {
                if (allocatedFiles[j].filename[0] == '\0')
                {
                    strcpy(allocatedUsers[i].nome, username);
                    strcpy(allocatedFiles[j].filename, filename);
                    strcpy(allocatedFiles[j].owner, username);

                    fclose(writeFile);
                    return 0;
                }
            }  
        }
    }
    return -5;
}

/* Função para receber messagem do tipo "S" do cliente */
int messageReceive(char username[], char filename[], char paragraphContent[], char paragraphPosition[], int paragraphNumber)
{
    FILE *writeFile;
    char tempFilename[100], tempParagraph[BUFFER_SIZE], tempString[50][256];
    int paragraphCount = 0;

    /* Primeiro passo: verificar se o usuário existe no vetor de usuários */
    for (int i=0; i<MAX_CLIENTE; ++i)
    {
        /* Caso o usuário não exista no vetor, retorna -1 */
        if ((strcmp(allocatedUsers[i].nome, username)) != 0)
            if ((i+1) == MAX_CLIENTE)
                return -1;
        
        /* Caso o usuário exista no vetor */
        if ((strcmp(allocatedUsers[i].nome, username)) == 0)
        {
            /* Verificação para tentar encontrar o nome do arquivo passado como parâmetro no vetor de arquivos */
            for (int j=0; j<MAX_CLIENTE; j++)
            {
                /* Caso o arquivo não exista no vetor de arquivos, retorna -2 */
                if ((strcmp(allocatedFiles[j].filename, filename)) != 0)
                    if ((j+1) == MAX_CLIENTE)
                        return -2;
                
                /* Caso o arquivo exista no vetor */
                if ((strcmp(allocatedFiles[j].filename, filename)) == 0)
                {
                    /* Colocar na variável temporária de arquivo, o nome do mesmo conforme foi gravado */
                    memset(tempFilename, 0, sizeof(tempFilename));
                    strcpy(tempFilename, allocatedFiles[j].filename);
                    strcat(tempFilename, ".txt");

                    writeFile = fopen(tempFilename, "r");

                    /* Contagem dos parágrafos do arquivo */
                    while((fgets(tempParagraph, BUFFER_SIZE, writeFile)) != NULL)
                    {
                        strcpy(tempString[paragraphCount], tempParagraph);
                        ++paragraphCount;
                    }

                    fclose(writeFile);

                    /* Verificar qual posição inserir no arquivo */
                    /* Caso 1: inserir no inicio */
                    if ((strcmp(paragraphPosition, "[inicio]")) == 0)
                    {
                        writeFile = fopen(tempFilename, "w");
                        fprintf(writeFile, "[%s] %s\n", allocatedUsers[i].nome, paragraphContent);

                        for (int a=0; a<paragraphCount; ++a)
                            fprintf(writeFile, "%s", tempString[a]);

                        fclose(writeFile);
                    }
                    else if ((strcmp(paragraphPosition, "[fim]")) == 0)
                    {
                        /* Caso 2: inserir no fim */
                        writeFile = fopen(tempFilename, "w");

                        for (int a=0; a<paragraphCount; ++a)
                            fprintf(writeFile, "%s", tempString[a]);

                        fprintf(writeFile, "[%s] %s\n", allocatedUsers[i].nome, paragraphContent);

                        fclose(writeFile);
                    }
                    else if (paragraphPosition[0] == 'P')
                    {
                        /* Caso 3: inserir no parágrafo desejado */
                        if (paragraphNumber > paragraphCount || paragraphNumber <= 0)
                            return -3;
                        
                        writeFile = fopen(tempFilename, "w");

                        for (int a=0; a<paragraphCount; ++a)
                        {
                            if ((a+1) == paragraphNumber)
                                fprintf(writeFile, "[%s] %s\n", allocatedUsers[i].nome, paragraphContent);

                            fprintf(writeFile, "%s", tempString[a]);
                        }

                        fclose(writeFile);
                    }

                    /* Retorno 0 caso tenha sucesso na inserção */
                    return 0;
                }
            }
        }
    }
    return -5;
}

int main(int argc, char **argv)
{
    /* Iniciando semáforo para sincronização das threads */
    sem_init(&sinaleiro, 0, 1);

    /* Criando pthread e mutex iniciado */
    pthread_t clients;

    /* Estruturas para o servidor e cliente */
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;

    /* Descritores para o servidor e cliente */
    int serverSocket, clientSocket, *newSocket;

    char buffer[BUFFER_SIZE];

    /* Verifica a chamada do código */
    if(argc < 2)
    {
        fprintf(stderr, "\nCÓDIGO: ES01\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n");
        exit(-1);
    }
    
    /* Recebe a porta para conexão */
    short port = atoi(argv[1]);

    if (port < 5000)
	{
	    fprintf(stderr, "ERRO ES02\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n");
	    exit(-1);
	}

    /* Cria socket IPV4 */
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(!serverSocket)
    {
        fprintf(stderr, "\nCÓDIGO: ES03\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n");
        close(serverSocket);
        exit(-1);
    }
    /* Permitindo que o servidor use portas já utilizadas anteriormente */
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    
    /* Mapeando valores da estrutura do servidor */
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    memset(&serverAddress.sin_zero, 0, sizeof(serverAddress.sin_zero));

    if(bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        fprintf(stderr, "Erro ES04\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n");
        close(serverSocket);
        exit(-1);
    }

    /* Responsável pela lista dos clientes */
    if(listen(serverSocket, MAX_CLIENTE) == 1)
    {
        fprintf(stderr, "Erro ES05\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n");;
        close(serverSocket);
        exit(-1);
    }

    while (1)
    {
        /* Obtendo o tamanho do endereço do cliente */
        socklen_t clientAddrLength = sizeof(clientAddress);

        /* Aceita a conexão do cliente */
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddrLength);
        /* Verifica se está na capacidade máxima */
        if(++clientCount == MAX_CLIENTE)
        {
            /* Mensagem de erro para o servidor */
            fprintf(stderr, "ERRO ES06\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n");
            /* Mensagem de erro para o cliente */
            sprintf(buffer, "ERRO EC06\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n");
            /* Envia a mensagem e encerra a conexão */
            send(clientSocket, buffer, strlen(buffer), 0);
            shutdown(clientSocket, SHUT_RDWR);
            close(clientSocket);
            continue;
        }
       
        /* Cria thread para o cliente */
        newSocket = malloc(1);
        *newSocket = clientSocket;
        clientCount++;
        if(pthread_create(&clients, NULL, clientThread, (void *)newSocket) < 0)
        {
            fprintf(stderr, "ES07\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n");
            exit(-1);
        }       
      
        memset(buffer, 0, BUFFER_SIZE);
    }  

    return 0;
}

void *clientThread(void *newSocket)
{
    /* Descritor do cliente */
    int clientSocket = *(int*)newSocket;

    int length;

    char receiveBuffer[BUFFER_SIZE];
    char sendBuffer[BUFFER_SIZE];

    length = recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);
    if (length < 0)
    {
        fprintf(stderr, "ES08\nVerifique o arquivo DocumentaçãoWebDocs.pdf\n\n");
        close(clientSocket);
    }

    receiveBuffer[length] = '\0';
    
    int indice = 0, paragraphNumber = 0;
    char *pt, messageType = -1, username[50], filename[50], paragraphContent[1024], paragraphPosition[10];
    
    /* Inicializando string com os parâmetros da mensagem */
    memset(filename, 0, sizeof(filename));
    memset(paragraphContent, 0, sizeof(paragraphContent));
    memset(paragraphPosition, 0, sizeof(paragraphPosition));
    pt = strtok(receiveBuffer, " ");
    while(pt)
    {
        switch(indice)
        {
            case 0:
                messageType = *pt;
                break;
            case 1:
                strcpy(username,  pt);
                break;
            case 2:
                if(messageType != 'L')
                    strcpy(filename, pt);
                break;
            case 3:
                if(messageType == 'S')
                    strcpy(paragraphContent, pt);
                break;
            case 4:
                strcpy(paragraphPosition, pt);
                break;
            case 5:
                paragraphNumber = atoi(pt);
                break;
        
        }
        indice++;
        if (indice <= 2 || indice > 3)
            pt = strtok(NULL, " ");
        else
            pt = strtok(NULL, "[]");
    }

    sem_wait(&sinaleiro);

    int operationResult;
    char *tempResult = malloc(sizeof(char)*BUFFER_SIZE);
    switch (messageType)
    {
    case 'N':
        operationResult = addRegister(username, filename);
        snprintf(sendBuffer, BUFFER_SIZE, "%c %d\n", messageType, operationResult);
        send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
        break;
    
    case 'L':
        tempResult = verifyRegister(username);
        snprintf(sendBuffer, BUFFER_SIZE, "%s\n", tempResult);
        send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
        break;

    case 'S':
        operationResult = messageReceive(username, filename, paragraphContent, paragraphPosition, paragraphNumber);
        snprintf(sendBuffer, BUFFER_SIZE, "%c %d", messageType, operationResult);
        send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
        break;
    default:
        break;
    }
    clientCount--;
    close(clientSocket);
    sem_post(&sinaleiro);

    pthread_exit(0);
    //pthread_detach(pthread_self());
}
