#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>

int aBloquer(char *url, char* namefile)
{
   FILE *file = fopen (namefile, "r");
   int res = 0;
   if ( file != NULL )
   {
      char line [ 1000 ];
      while ( fgets ( line, sizeof line, file ) != NULL )
      {
          if (strstr(url, line) != NULL) {
             res = 1;
          }
      }
      fclose ( file );
   }
   else
   {
      perror ( "impossible de lire le fichier");
   }
   return res;
}

int main(int argc,char* argv[])
{

  //création d'une socket
  pid_t pid;
  struct sockaddr_in cli_addr,serv_addr;
  struct hostent* host;
  int sockfd, dialogSocket, clilen;

  if(argc<2)
    perror("Veuillez choisir un numero de port en argument");

//bzero((char*)&serv_addr,sizeof(serv_addr));
//bzero((char*)&cli_addr, sizeof(cli_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(argv[1]));
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		perror ("La socket ne s'est pas creee correctement");
		exit(1);
  }

if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
  perror("La liaison ne s'est pas effectuee correctement");


  listen(sockfd,23);
  clilen = sizeof(cli_addr);



accepting:

dialogSocket = accept(sockfd,(struct sockaddr*)&cli_addr,(socklen_t *)&clilen);

if(dialogSocket< 0) {
 perror("erreur de communication\n");
 exit (1);
}

//multi-thread pour plusieurs clients
pid=fork();
if(pid==0)
{
  //adresse du site
  struct sockaddr_in host_addr;
  struct addrinfo hints;
	struct addrinfo *res = NULL;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  int test_port=0, connexionSocket, retour, port = 0, i , socketWeb;

  char buffer[1000], req[100], url[500], version[10], p[10];

  char* tmp = NULL;

  //reception
  recv(dialogSocket,buffer,500,0);

  //division du contenu HTTP
  sscanf(buffer,"%s %s %s", req, url, version);


  if(((strncmp(req,"GET",3)==0))&&
  ((strncmp(version,"HTTP/1.1",8)==0)||(strncmp(version,"HTTP/1.0",8)==0))&&
  (strncmp(url,"http://",7)==0))
  {
    //copie url dans req
    strcpy(req, url);

    test_port = 0;

    //verifie si un port est en fin de requete
    //met flag à 1 si :port dans l'url
    for(i=7;i<strlen(url);i++)
    {
      if(url[i]==':')
      {
        test_port=1;
        break;
      }
    }

    //temp = http:
    tmp=strtok(url,"//");
    if(test_port==0)
    {
      port = 80;
      //http:
      //temp=strtok(NULL,"/");
    }
    else
    {
      //http
      tmp=strtok(NULL,":");
    }


    sprintf(url,"%s",tmp);
    printf("host = %s",url);

    if(!aBloquer(url, "blacklist.txt"))
    {
      host=gethostbyname(url);

      //recuperation du port
      if(test_port==1)
      {
        tmp=strtok(NULL,"/");
        port=atoi(tmp);
      }

      sprintf(p, "%d", port);
      getaddrinfo(url,p,&hints,&res);

      strcat(req,"^]");
      tmp=strtok(req,"//");
      tmp=strtok(NULL,"/");
      if(tmp!=NULL)
        tmp=strtok(NULL,"^]");
      printf("\npath = %s\nPort = %d\n",tmp,port);


      host_addr.sin_port=htons(port);
      host_addr.sin_family=AF_INET;
      bcopy((char*)host->h_addr,(char*)&host_addr.sin_addr.s_addr,host->h_length);

      socketWeb = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
      connexionSocket = connect(socketWeb,(struct sockaddr*)&host_addr,sizeof(struct sockaddr));
      sprintf(buffer,"\nConnecté à %s  IP - %s\n",url,inet_ntoa(host_addr.sin_addr));
      if(connexionSocket<0)
        perror("connexion impossible au serveur web");

      printf("\n%s\n",buffer);
      //send(newsockfd,buffer,strlen(buffer),0);
      //bzero((char*)buffer,sizeof(buffer));
      if(tmp!=NULL)
        sprintf(buffer,"GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n",tmp,version,url);
      else
        sprintf(buffer,"GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n",version,url);

      retour = send(socketWeb,buffer,strlen(buffer),0);
      printf("\n%s\n",buffer);
      if(retour<0)
        perror("aucun envoi au client");
      else{
      do
        {
          bzero((char*)buffer,500);
          retour=recv(socketWeb,buffer,500,0);
          if(!(retour<=0))
          send(connexionSocket,buffer,retour,0);
        }while(retour>0);
      }
    }
    else
    {
      send(dialogSocket,"400 : MAUVAISE REQUETE\nTRAITEMENT DES REQUETE HTTP UNIQUEMENT",18,0);
    }
    close(socketWeb);
    close(dialogSocket);
    close(sockfd);
    exit(0);
  }
  }else{
    close(dialogSocket);
    goto accepting;
  }
  return 0;
}
