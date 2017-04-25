#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){

	//creation d'une socket
	int clientSocket;
 	if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		perror ("erreur socket");
		exit(1); }
	
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port=htons(1234);
	connect(clientSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	char *buffer = argv[1];
	write(clientSocket, buffer, 1024);

	printf("Message envoye au serveur: %s\n",argv[1]);
	
	close(clientSocket);
	return 0;
}
