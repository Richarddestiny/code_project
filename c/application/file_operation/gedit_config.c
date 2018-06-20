/*
 *从配置文件中读取相应的值
 *输入参数：1，配置文件路径 2，匹配标记 3，输出存储空间
 *并且排除了空行，“=”前后无内容，无“=”的情况
 */
ReadConfig(char *conf_path,char *conf_name,char *config_buff)
{
    char config_linebuf[256];
    char line_name[40];
    char exchange_buf[256];
    char *config_sign = "=";
    char *leave_line;
    FILE *f;
    f = fopen(conf_path,"r");
    if(f == NULL)
    {
        printf("OPEN CONFIG FALID\n");
        return 0;
    }
    fseek(f,0,SEEK_SET); 
    while(fgets(config_linebuf,256,f) != NULL)
    {   
        if(strlen(config_linebuf) < 3) //判断是否是空行
        {
            continue;
        }
        if (config_linebuf[strlen(config_linebuf)-1] == 10) //去除最后一位是\n的情况
        {
            
            memset(exchange_buf,0,sizeof(exchange_buf));
            strncpy(exchange_buf,config_linebuf,strlen(config_linebuf)-1);
            memset(config_linebuf,0,sizeof(config_linebuf));
            strcpy(config_linebuf,exchange_buf);
        }
        memset(line_name,0,sizeof(line_name));
        leave_line = strstr(config_linebuf,config_sign);
        if(leave_line == NULL)                            //去除无"="的情况
        {
            continue;
        }
        int leave_num = leave_line - config_linebuf;
        strncpy(line_name,config_linebuf,leave_num);
        if(strcmp(line_name,conf_name) ==0)
        {
            strncpy(config_buff,config_linebuf+(leave_num+1),strlen(config_linebuf)-leave_num-1);
            break;
        }
        if(fgetc(f)==EOF)
        {
            break;  
        }
        fseek(f,-1,SEEK_CUR);
        memset(config_linebuf,0,sizeof(config_linebuf));
    }
    fclose(f);
}
/*
 *添加修改文件（当配置文件中存在标记字段，则进行修改，若不存在则进行添加）
 *
 *输入参数：1，配置文件路径 2，匹配标记 3，替换或添加的内容
 *
 */
AddOrAltConfig(char *conf_path,char *conf_name,char *config_buff)
{
    
    char config_linebuf[256];
    char line_name[40];
    char *config_sign = "=";
    char *leave_line;
    int  alter_sign = 0;
    
    FILE *f;
    f = fopen(conf_path,"r+");
    if(f == NULL)
    {
        printf("OPEN CONFIG FALID\n");
        return 0;
    }
    fseek(f,0,SEEK_END);
    long congig_lenth = ftell(f);
    int configbuf_lenth = strlen(config_buff);
    configbuf_lenth = configbuf_lenth + 5;
    char sum_buf[congig_lenth+configbuf_lenth];
    memset(sum_buf,0,sizeof(sum_buf));
    fseek(f,0,SEEK_SET); 
    while(fgets(config_linebuf,256,f) != NULL)
    {   
        if(strlen(config_linebuf) < 3) //判断是否是空行
        {
            strcat(sum_buf,config_linebuf);
            continue;
        }
        leave_line = NULL;
        leave_line = strstr(config_linebuf,config_sign);
        if(leave_line == NULL)                            //去除无"="的情况
        {
            strcat(sum_buf,config_linebuf);
            continue;
        }
        int leave_num = leave_line - config_linebuf;
        memset(line_name,0,sizeof(line_name));
        strncpy(line_name,config_linebuf,leave_num);
        if(strcmp(line_name,conf_name) ==0)
        {
            strcat(sum_buf,config_buff);
            strcat(sum_buf,"\n");
            alter_sign = 1;
        }
        else
        {
            strcat(sum_buf,config_linebuf);
        }
        if(fgetc(f)==EOF)
        {
            break;  
        }
        fseek(f,-1,SEEK_CUR);
        memset(config_linebuf,0,sizeof(config_linebuf));
    }
    if(alter_sign == 0)
    {
        strcat(sum_buf,config_buff);
        strcat(sum_buf,"\n");
    }
    printf("---sum_buf---->%s<----------\n",sum_buf);
    remove(conf_path);
    fclose(f);
    FILE *fp;
    fp = fopen(conf_path,"w+");
    if(fp == NULL)
    {
        printf("OPEN CONFIG FALID\n");
        return 2;
    }
    fseek(fp,0,SEEK_SET);
    fputs(sum_buf,fp);
    fclose(fp);
}
/*
 *删除配置文件内容（
 *
 *输入参数：1，配置文件路径 2，匹配标记 
 *
 */
DeleteConfig(char *conf_path,char *conf_name)
{
    
    char config_linebuf[256];
    char line_name[40];
    char *config_sign = "=";
    char *leave_line;
    
    FILE *f;
    f = fopen(conf_path,"r+");
    if(f == NULL)
    {
        printf("OPEN CONFIG FALID\n");
        return 0;
    }
    fseek(f,0,SEEK_END);
    long congig_lenth = ftell(f);
    char sum_buf[congig_lenth+2];
    memset(sum_buf,0,sizeof(sum_buf));
    fseek(f,0,SEEK_SET); 
    while(fgets(config_linebuf,256,f) != NULL)
    {   
        if(strlen(config_linebuf) < 3) //判断是否是空行
        {
            strcat(sum_buf,config_linebuf);
            continue;
        }
        leave_line = NULL;
        leave_line = strstr(config_linebuf,config_sign);
        if(leave_line == NULL)                            //去除无"="的情况
        {
            strcat(sum_buf,config_linebuf);
            continue;
        }
        int leave_num = leave_line - config_linebuf;
        memset(line_name,0,sizeof(line_name));
        strncpy(line_name,config_linebuf,leave_num);
        if(strcmp(line_name,conf_name) ==0)
        {
            
        }
        else
        {
            strcat(sum_buf,config_linebuf);
        }
        
        if(fgetc(f)==EOF)
        {
            break;  
        }
        fseek(f,-1,SEEK_CUR);
        memset(config_linebuf,0,sizeof(config_linebuf));
    }
    printf("---sum_buf---->%s<----------\n",sum_buf);
    remove(conf_path);
    fclose(f);
    FILE *fp;
    fp = fopen(conf_path,"w+");
    if(fp == NULL)
    {
        printf("OPEN CONFIG FALID\n");
        return 2;
    }
    fseek(fp,0,SEEK_SET);
    fputs(sum_buf,fp);
    fclose(fp);
}