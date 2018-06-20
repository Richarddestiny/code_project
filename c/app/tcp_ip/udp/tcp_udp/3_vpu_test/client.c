/*************************************************************************
> File Name: client.c
> Author: richard 
> Mail: freedom_wings@foxmail.com
> Created Time: Mon 12 Oct 2015 01:28:32 AM PDT
************************************************************************/

#include<stdio.h>

#include <stdlib.h>

#include <errno.h>

#include <string.h>

#include <netdb.h>

#include <sys/types.h>

#include <netinet/in.h>

#include <sys/socket.h>

#define BUFSIZE 1000
#define DOCK_BUF 1024
struct dock {
        char magic_key[5];
        int cmd_type;
        int data_size;
        char buf[DOCK_BUF];

};
int

main (int argc, char *argv[])

{

    int s;

    int len;

    struct sockaddr_in remote_addr;

    struct dock buf;

    memset (&remote_addr, 0, sizeof (remote_addr));

    remote_addr.sin_family = AF_INET;

    remote_addr.sin_addr.s_addr = inet_addr ("192.168.0.148");

    remote_addr.sin_port = htons (6666);

    if ((s = socket (PF_INET, SOCK_STREAM, 0)) < 0)
    {

        perror ("socket");
        return 1;

    }
    if (connect (s, (struct sockaddr *) &remote_addr, sizeof (struct sockaddr)) <
        0)
    {

        perror ("connect");
        return 1;
    }
    while (1)

    {
        memset(&buf,0,sizeof(buf));
    memcpy(&buf.magic_key,"dock",5);
        send(s,&buf,sizeof(buf),0);
        len = recv (s, &buf, sizeof(buf), 0);
        printf ("received:%s\n", buf.buf);
        sleep(1);

    }
      close (s);

        return 0;


}
