/****************************************YUV422P_To_RGB24.c**************************/  
//模块功能：将YUV422_PLANAR图像数据转换成RGB24格式   
typedef unsigned char BYTE; // [0..255]   
/* 
 * 接口说明： 
 * 功能：构建查询表，转换模块运行前必须进行的初始化操作 
 */  
void YUV422P_To_RGB24_init();  
/* 
 *接口说明： 
 *功能：将YUV422P图像数据转换成RGB24格式 
 *参数： 
 *            pY: YUV422P图像数据Y的起始指针 
 *            pU: YUV422P图像数据U的起始指针 
 *            pV: YUV422P图像数据V的起始指针 
 *        DstPic: 转换成的RGB24图像数据的起始指针 
 *         width: 图像宽度 
 *        height: 图像高度 
 *返回值：成功返回0，失败返回-1 
 *注意：DstPic所指向的缓冲区必须事先分配好，其大小应该为 width*height*3 
 */  
int YUV422P_To_RGB24(BYTE* pY, BYTE* pU, BYTE* pV, BYTE* DstPic, int width, int height);  

    //使用整数运算(定点数运算)来代替浮点运算   
    const int csY_coeff_16 = 1.164383 * (1 << 16);  
    const int csU_blue_16 = 2.017232 * (1 << 16);  
    const int csU_green_16 = (-0.391762) * (1 << 16);  
    const int csV_green_16 = (-0.812968) * (1 << 16);  
    const int csV_red_16 = 1.596027 * (1 << 16);  
    //颜色查表   
    static BYTE _color_table[256 * 3];  
    static const BYTE* color_table = &_color_table[256];  
    //查表   
    static int Ym_tableEx[256];  
    static int Um_blue_tableEx[256];  
    static int Um_green_tableEx[256];  
    static int Vm_green_tableEx[256];  
    static int Vm_red_tableEx[256];  
    //颜色饱和函数   
    inline long border_color(long color) {  
        if (color > 255)  
            return 255;  
        else if (color < 0)  
            return 0;  
        else  
            return color;  
    }  
    //采用查找表进行计算时，必须运行的初始化函数   
    void YUV422P_To_RGB24_init() {  
        int i;  
        for (i = 0; i < 256 * 3; ++i)  
            _color_table[i] = border_color(i - 256);  
        for (i = 0; i < 256; ++i) {  
            Ym_tableEx[i] = (csY_coeff_16 * (i - 16)) >> 16;  
            Um_blue_tableEx[i] = (csU_blue_16 * (i - 128)) >> 16;  
            Um_green_tableEx[i] = (csU_green_16 * (i - 128)) >> 16;  
            Vm_green_tableEx[i] = (csV_green_16 * (i - 128)) >> 16;  
            Vm_red_tableEx[i] = (csV_red_16 * (i - 128)) >> 16;  
        }  
    }  
    inline void YUVToRGB24_Table(BYTE *p, const BYTE Y0, const BYTE Y1,  
            const BYTE U, const BYTE V) {  
        int Ye0 = Ym_tableEx[Y0];  
        int Ye1 = Ym_tableEx[Y1];  
        int Ue_blue = Um_blue_tableEx[U];  
        int Ue_green = Um_green_tableEx[U];  
        int Ve_green = Vm_green_tableEx[V];  
        int Ve_red = Vm_red_tableEx[V];  
        int UeVe_green = Ue_green + Ve_green;  
        *p = color_table[(Ye0 + Ve_red)];  
        *(p + 1) = color_table[(Ye0 + UeVe_green)];  
        *(p + 2) = color_table[(Ye0 + Ue_blue)];  
        *(p + 3) = color_table[(Ye1 + Ve_red)];  
        *(p + 4) = color_table[(Ye1 + UeVe_green)];  
        *(p + 5) = color_table[(Ye1 + Ue_blue)];  
    }  
    int YUV420P_To_RGB24(BYTE* pY, BYTE* pU, BYTE* pV, BYTE* DstPic, int width,  
            int height) {  
        int y, x, x_uv;  
        BYTE* pDstLine = DstPic;  
        if ((width % 2) != 0 || (height % 2) != 0)  
            return (-1);  
        for (y = 0; y < height; ++y) {  
            //DECODE_PlanarYUV211_Common_line(pDstLine, pY, pU, pV,width);   
            for (x = 0; x < width; x += 2) {  
                x_uv = x >> 1;  
                YUVToRGB24_Table(&pDstLine[x * 3], pY[x], pY[x + 1], pU[x_uv],  
                        pV[x_uv]);  
            }  
            pDstLine += width * 3; //RGB888   
            pY += width; //YUV422   
            if (y % 2 == 1) {  
                pU += width / 2;  
                pV += width / 2;  
            }  
        }  
        return 0;  
    }  