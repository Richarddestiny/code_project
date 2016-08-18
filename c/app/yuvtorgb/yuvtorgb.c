/*************************************************************************
	> File Name: yuvtorgb.c
	> Author: richard 
	> Mail: freedom_wings@foxmail.com
	> Created Time: Tue 20 Oct 2015 03:17:17 AM PDT
 ************************************************************************/

static   long   int   crv_tab[256]; static   long   int   cbu_tab[256]; static   long   int   cgu_tab[256]; static   long   int   cgv_tab[256]; static   long   int   tab_76309[256]; static   unsigned   char   clp[1024]; //for   clip   in   CCIR601 #include   <string> #include   <string.h> using   namespace   std; #define   FRAME   0 #define   FILED   1 #define   YUV422   0 #define   YUV420   1 #define   RGB   2 #define   TRUE   0 #define   FALSE   1 static   string   str; void   InitConvtTbl(); void   YUV2RGB420(unsigned   char   *src,   unsigned   char   *dst_ori,   int   width,int   height); void   YUV2RGB(unsigned   char   *src,   unsigned   char   *dst_ori,   int   width,int   height); void   RGB2YUV(unsigned   char   *,   unsigned   char   *,   int,int); int   main(int   argc,char   *argv[]) { int   i,j,k,m,n; int   fileback; int   frameWidth=160;   int   frameHeight=120;  }
