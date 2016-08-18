/*************************************************************************
> File Name: network.c
> Author: richard
> Mail: freedom_wings@foxmail.com 
> Created Time: 2013年12月12日 星期四 11时56分16秒
************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <unistd.h>

#define  DEBUG
int get_netlink_status(const char *if_name);

int main(int argc,char *argv[])
{
    char *phy[] ={"eth0","wlan0","ppp0"};
    int change_flag = 0;
    int run_flag = 0;
    int i;
    struct ifreq  ifr;
    if(getuid() != 0)
    {
        fprintf(stderr, "Netlink Status Check Need Root Power.\n");
        return 1;
    }
    int skfd,ret;

    if (( skfd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0)
    {
        printf("network check : socket err %d!\n",skfd);
        return -1;
    }
    printf("%d\n",argc);

    for(i=0;i<3;i++)
    {
        strncpy(ifr.ifr_name, phy[i] ,sizeof(ifr.ifr_name));
        if(ret = ioctl( skfd,SIOCGIFFLAGS, &ifr))
            change_flag &= ~(1<<i);
        else
            change_flag =  (ifr.ifr_flags & IFF_RUNNING ? 1:0) << i;
        if( argc >1 && 0 == i &&change_flag & 0x01 )
        {
            //system("ip addr flush dev eth0;udhcpc -b -q -i eth0");
			   system("/usr/bin/dhclient -r  eth0;/usr/bin/dhclient eth0");
		     #if defined(DEBUG)
                    printf("dhclient eth0 ok!\n");
            #endif
        }
    #if defined(DEBUG)
    printf("%s:flags =%x\t",phy[i],ifr.ifr_flags);
    #endif
    }
    #if defined(DEBUG)
    printf("\n");
    #endif
    while(1)
    {
        for(i=0;i<3;i++)
        {
        strncpy(ifr.ifr_name, phy[i] ,sizeof(ifr.ifr_name));
        if(ret = ioctl( skfd,SIOCGIFFLAGS, &ifr))
            run_flag &= ~(1<<i);
        else
            run_flag =  (ifr.ifr_flags & IFF_RUNNING ? 1:0) << i;
            if((run_flag ^ change_flag) & (1 << i))
            {
                change_flag ^= (1<<i);
                switch(i+1)
                {
                    case 1 :
                    if( run_flag & (1<<i))
                    {
                        //system("/bin/ip addr flush dev eth0;/sbin/udhcpc -b -q -i eth0");
 						system("/usr/bin/dhclient eth0  &");
                        #if defined(DEBUG)
                         printf("dhclient eth0 ok!\n");
                        #endif
                    }
                    else
                    {
                       // system("/bin/ip addr flush dev eth0");
						system("/usr/bin/dhclient -r eth0 &");
                        #if defined(DEBUG)
                        printf("route del eth0 ok!\n");
                        #endif
                    }
                    break;
                    case 2 :

                    if( run_flag & (1<<i))
                    {
                        //system("/bin/ip addr flush dev wlan0;/sbin/udhcpc -b -q -i wlan0");
						      system("/usr/bin/dhclient wlan0 &");
                        #if defined(DEBUG)
                        printf("route add wlan0 ok!\n");
                        #endif
                    }
                    else
                    {
                       // system("/bin/ip addr flush dev wlan0");
						  system("/usr/bin/dhclient -r wlan0 &");
                        #if defined(DEBUG)
                        printf("clean wlan0 ok!\n");
                        #endif
                    }
                    break;
                    case 3 :
                    if( run_flag & (1<<i))
                    {
                        system("/sbin/route del default dev ppp0;/sbin/route add default dev ppp0 metric 2");
                        #if defined(DEBUG)
                        printf("route add ppp0 ok!\n");
                        #endif
                    }
                    else
                    {
                        system("/sbin/route del default dev ppp0");
                        #if defined(DEBUG)
                        printf("route del ppp0 ok!\n");
                        #endif
                    }
                    break;
                    default:break;

                 }

         }   

    }
    sleep(1);
    }
    close(skfd);
    return 0;
}



