#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 1024

/*
struct sockaddr_in{
	uint8_t sa_len;
	sa_family_t sa_family;
	char sun_path[104];
}*/

int main(){

	char line[10000], method[10000], url[10000], protocol[10000], host[10000], path[10000];
	unsigned short port;
	int iport;
	int sockfd;
	int ssl;
	FILE* sockrfp;
	FILE* sockwfp;
	
	/*
	* verifier la requete est bonne
	*/

	/* Read the first line of the request. */
	if ( fgets( line, sizeof(line), stdin ) == (char*) 0 )
	//send_error( 400, "Bad Request", (char*) 0, "No request found." );

	/* Parse it. */
	trim( line );
	if ( sscanf( line, "%[^ ] %[^ ] %[^ ]", method, url, protocol ) != 3 )
		//send_error( 400, "Bad Request", (char*) 0, "Can't parse request." );

	if ( url[0] == '\0' )
		//send_error( 400, "Bad Request", (char*) 0, "Null URL." );

	openlog( "micro_proxy", 0, LOG_DAEMON );
	syslog( LOG_INFO, "proxying %s", url );

	if ( strncasecmp( url, "http://", 7 ) == 0 ){
	(void) strncpy( url, "http", 4 );	/* make sure it's lower case */
		if ( sscanf( url, "http://%[^:/]:%d%s", host, &iport, path ) == 3 )
			port = (unsigned short) iport;
		else if ( sscanf( url, "http://%[^/]%s", host, path ) == 2 )
			port = 80;
		else if ( sscanf( url, "http://%[^:/]:%d", host, &iport ) == 2 ){
			port = (unsigned short) iport;
			*path = '\0';
		}
		else if ( sscanf( url, "http://%[^/]", host ) == 1 ){
			port = 80;
			*path = '\0';
		}
		//else send_error( 400, "Bad Request", (char*) 0, "Can't parse URL." );
		ssl = 0;
	}
	else if ( strcmp( method, "CONNECT" ) == 0 ){
		if ( sscanf( url, "%[^:]:%d", host, &iport ) == 2 )
			port = (unsigned short) iport;
		else if ( sscanf( url, "%s", host ) == 1 )
			port = 443;
		else
		//send_error( 400, "Bad Request", (char*) 0, "Can't parse URL." );
		ssl = 1;
	}
	else
		//send_error( 400, "Bad Request", (char*) 0, "Unknown URL type." );

	
	/**************************************************
	*		creation d'une socket
	***************************************************/
	
	int serverSocket;
 	if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		perror ("erreur socket");
		exit(1); }
	
	//ouverture d'un service
	int dialogSocket;
	int clilen;
	
	struct sockaddr_in serv_addr, cli_addr;
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port=htons(1234);
	bind(serverSocket , (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	listen(serverSocket, 23);
	
	clilen = sizeof(cli_addr);
	char buffer[BUF_SIZE]={0};

	while(1){		
		dialogSocket= accept(serverSocket,(struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
		
		if(dialogSocket< 0) {
			 perror("servecho : erreur accept\n");
			 exit (1);
		}else{
			int reqlen = read(dialogSocket, buffer, BUF_SIZE);
			if(reqlen > 0){
				write(dialogSocket, buffer, reqlen);
				close(dialogSocket);
				printf("%s\n",buffer);
				memset(buffer,'\0',BUF_SIZE-1);
			}
		}
	}	
	
	close(serverSocket);

	return 0;
	
}
