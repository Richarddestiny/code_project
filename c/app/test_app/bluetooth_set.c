#include <stdio.h> /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h> /* String function definitions */
#include <unistd.h> /* UNIX standard function definitions */
#include <fcntl.h> /* File control definitions */
#include <errno.h> /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <pthread.h>

static int set_fl(int fd, int flags)
{
    int val;

    val = fcntl(fd, F_GETFL, 0);
    if(val < 0){
        perror("fcntl get error");
        exit(1);
    }

    val |= flags;
    if(fcntl(fd, F_SETFL, val) < 0){
        perror("fcntl set error");
        exit(2);
    }

    return 0;
}

/*
 * @brief Open serial port with the given device name
 *
 * @return The file descriptor on success or -1 on error.
 */
int open_port(char *port_device)
{
    int fd; /* File descriptor for the port */

    fd = open(port_device, O_RDWR );//O_NOCTTY );
    if (fd == -1)
    {
        perror("open_port: Unable to open port");
        exit(-1);
    }
    

    return (fd);
}




void get_result( void *ptr )
{
    char buf[1024];
    int fd=*((int*)ptr);
    char* pos;
    int n;
    
    strcpy(buf,"\n> ");
    pos=&buf[strlen(buf)];
    while(1){
        if((n=read(fd, pos, 1))==1 ){
         if(*pos=='\n'){
         *pos=0;
         if(pos!=buf)printf("%s\n",buf);
         fflush(stdout);
                strcpy(buf,"\n> ");
                pos=&buf[strlen(buf)];
     continue;
         }
         pos++;
        }
    }

}

int main(int argc, char* argv[])
{
    struct termios options;
    pthread_t get_result_thread;
    void * thread_ret;

    if(argc<3){
     printf("usage:\n\t%s \n",argv[0]);
     return -1;
    }
    printf("Selected baud rate is %ld\n",atol(argv[2]));
    
    int fd=open_port(argv[1]);
    if(fd==-1){
        return -1;
    }

    tcgetattr(fd, &options);


    //Set the baud rate
    switch(atol(argv[2])){
    case 9600:
        cfsetispeed(&options, B9600);
        cfsetospeed(&options, B9600);
        break;
    case 19200:
        cfsetispeed(&options, B19200);
        cfsetospeed(&options, B19200);
        break;
    case 38400:
        cfsetispeed(&options, B38400);
        cfsetospeed(&options, B38400);
        break;
    case 57600:
        cfsetispeed(&options, B57600);
        cfsetospeed(&options, B57600);
        break;
    case 115200:
        cfsetispeed(&options, B115200);
        cfsetospeed(&options, B115200);
        break;
    default:
        printf("Selected baud rate %ld is not support now!\n", atol(argv[2]));
        close(fd);
        return -1;
    }
   

    //Enable the receiver and set local mode...
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE; /* Mask the character size bits */
    options.c_cflag |= CS8; /* Select 8 data bits */

    //No parity
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;


    //Set the new options for the port...
    tcsetattr(fd, TCSANOW, &options);
    pthread_create( &get_result_thread, NULL, (void*)&get_result, (void*)&fd);
    
    char buf[1024];


    while(1){
        memset(buf,0,sizeof buf);
        printf("CMD: ");
        fgets(buf, sizeof buf, stdin);
        int len=strlen(buf);
        if(len>2){
         buf[len-1]='\r';
         buf[len]='\n';
         buf[len+1]=0;
            size_t n=write(fd, buf, len+1);
        
            if(n!=len+1){
                printf("\n\tError!\n");
            }

        }
        usleep(100000);
    }
    
    
    pthread_join(get_result_thread,&thread_ret);

    close(fd);
}
