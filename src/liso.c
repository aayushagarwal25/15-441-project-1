/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/


// FDSETSIZE?
// select should use readfds and writefds?

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/parse.h"
 #include <fcntl.h> /* Added for the nonblocking socket */

#define ECHO_PORT 9999
#define BUF_SIZE 4096

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{

    char* arg1=argv[1];
    char* arg2=argv[2];
    //printf("argv[1]=%s\n",argv[2]);
    char* arg3=argv[3];
    char* arg4=argv[4];
    char* arg5=argv[5];
    char* arg6=argv[6];
    char* arg7=argv[7];
    char* arg8=argv[8];

    int http_port=atoi(arg1);
    /*
    TODO: fill the rest of the arguments

    */
   printf("http port= %d\n",http_port);
    
    int sock, client_sock;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];

    fprintf(stdout, "----- Echo Server -----\n");
    
    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(http_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }


    if (listen(sock, 10))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // add the listener to the master set
    FD_SET(sock, &master);

    // keep track of the biggest file descriptor
    fdmax = sock; // so far, it's this one

    int i,j;
    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        read_fds = master; // copy it
        //printf("Before select... waiting for new connections\n");
        if(select(fdmax+1,&read_fds,NULL,NULL,NULL)==-1)
        {
            perror("select");
            //exit(4);
        }
        for(i=0;i<=fdmax;i++)
        {
            if(FD_ISSET(i,&read_fds))
            {
                //we got one
                if(i==sock)
                {
                    cli_size = sizeof(cli_addr);
                    if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr,&cli_size)) == -1)
                    {
                        close(sock);
                        fprintf(stderr, "Error accepting connection.\n");
                        return EXIT_FAILURE;
                    }
                    else
                    {
                        FD_SET(client_sock,&master); // add to master set
                        if(client_sock > fdmax)
                        {   // keep track of the max
                            fdmax = client_sock;
                        }
                    }
                    
                }
                else
                { //handle data from client
                    readret=0;
                    int flag=0;
                    fcntl(i, F_SETFL, fcntl(i, F_GETFL,0) | O_NONBLOCK);
                
                    if((readret = recv(i, buf, BUF_SIZE, 0)) > 0)
                    {
                        printf("At start of while\n");
                        //for(j=0;j<=fdmax;j++)
                        //{
                        printf("received buffer= %s \nfrom client%d",buf,i);
                        Request *request = parse(buf,readret,i);
                        if(request==NULL)
                        {
                            printf("************Bad request************\nExiting...");
                            //request->http_method
                            char *response="HTTP/1.1 400 Bad Request\r\n\r\n";

                            if(send(i, response, 29, 0)==-1)
                            {
                                //close_socket(i);
                                //close_socket(sock);
                                fprintf(stderr, "Error sending to client.\n");
                                //return EXIT_FAILURE;
                            }
                             //close_socket(i);
                                //close_socket(sock);
                            //memset(buf, 0, BUF_SIZE);
                            //close_socket(i);
                            //FD_CLR(i,&master);
                            //free(request);
                            flag=1;
                            printf("*****BAD request send complete\n");
                            //readret=0;
                            //break;

                            //exit(0);
                        }
                        // else{
                        //     printf("Not null\n");
                        //     exit(0);
                        // }
                        else 
                        {
                            printf("**************Good request*******\n");
                            printf("dsadsadsadsad header_count= %d",request->header_count);
                            //free(request);
                            if (send(i, buf, readret, 0) != readret)
                            {
                                //close_socket(i);
                                //close_socket(sock);
                                fprintf(stderr, "Error sending to client.\n");
                                //return EXIT_FAILURE;
                            }
                            //memset(buf, 0, BUF_SIZE);
                            fcntl(i, F_SETFL, fcntl(i, F_GETFL,0) &  ~O_NONBLOCK);
                            //readret=0;
                            //close_socket(i);
                            //FD_CLR(i,&master);
                            printf("*****good request send complete\n");
                        }
                    } 
                    if(readret<=0)
                    {
                        if(readret==0)
                        {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                            close_socket(i);
                            FD_CLR(i,&master);
                        }
                        else
                        {       
                            perror("recv");
                            close_socket(i);
                            FD_CLR(i,&master);
                            //close_socket(sock);
                            //fprintf(stderr, "Error reading from client socket.\n");
                            //return EXIT_FAILURE;
                        }
                    }  
                    

                    /*if (close_socket(i))
                    {
                        FD_CLR(i,&master);
                        close_socket(sock);
                        fprintf(stderr, "Error closing client socket.\n");
                        return EXIT_FAILURE;
                    }*/
                }
            }
        }
    }
    close_socket(sock);

    return EXIT_SUCCESS;
}
