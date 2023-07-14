$(CC) = gcc

build:
	$(CC) -o servidor servidor.c -Wall -Wextra -pedantic -lpthread
	$(CC) -o cliente cliente.c -Wall -Wextra -pedantic -lpthread

server: 
	$(CC) -o servidor servidor.c -Wall -Wextra -pedantic -lpthread

cliente:
	$(CC) -o cliente cliente.c -Wall -Wextra -pedantic -lpthread

clean:
	rm -rf *.txt servidor cliente
	
