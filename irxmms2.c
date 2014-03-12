/*
 *Program:irmovie - play movie use IR
 *Author:Chier Xuefei(Clumart.G);myregs6@gmail.com
 *Date: 2014-3-4
 *Update:None
 */
#define VERSION "0.0.1"
#define DEBUG

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <lirc/lirc_client.h>
#include <xmmsclient/xmmsclient.h>

#define TRUE 1
#define FALSE 0
#define RETURN_SUCCESS 1
#define RETURN_FAILE 2

static int connect_status = FALSE;  //save the status of xmms2's connection

void xmms2_disconnect_callback(void *userdata)
{
    syslog(LOG_ERR,"Error: Code:209.\n");
    syslog(LOG_INFO,"XMMS2 close the connection!\n");
    connect_status = FALSE;
    exit(EXIT_FAILURE);
}

static int xmms2_conntion(char *program_name)   //Connect To Xmms2 
{
    xmmsc_connection_t *xmms2conn = NULL;
    char *xmms2_socket_path = NULL;
    int  result = RETURN_FAILE;

    //get socket of xmms2
    xmms2_socket_path = getenv("XMMS_PATH");

    //Init the xmms connection
    xmms2conn = xmmsc_init(program_name);

    if( xmms2conn == NULL )
    {   //init failed and quit
        syslog(LOG_ERR,"Error: Code:102.\n");
        syslog(LOG_INFO,"XMMS2 API init failed! Maybe your memory not enough!\n");
        result = RETURN_FAILE;
        exit(EXIT_FAILURE);
    }
    else
    {   //init success ,then connect to xmms2's socket
        if( !xmmsc_connect(xmms2conn,xmms2_socket_path) )
        {   //xmms2 connect failed.
            char* error_message;
            error_message = xmmsc_get_last_error( xmms2conn );
            syslog(LOG_ERR,"Error: Code:103.\n");
            syslog(LOG_INFO,"XMMS2 connection is failed!\n");
            syslog(LOG_DEBUG,"%s\n",error_message);
            syslog(LOG_DEBUG,"GET XMMS2 SOCKET PATH: %s \n",xmms2_socket_path);
            result = RETURN_FAILE;
            exit(EXIT_FAILURE);
        }
        else
        {   //xmms2 connect success,then update the connect_status and set callback.
            syslog(LOG_INFO,"Connect xmms2d success!\n");
            xmmsc_disconnect_callback_set(xmms2conn, xmms2_disconnect_callback, NULL);
            connect_status = TRUE;
            result = RETURN_SUCCESS;
        }
    }
    return result;                                                                  
}

int main(int argc, char *argv[])    //argc为命令和命令行参数的总数,argv[0]为程序名，argv[1]为第一个参数
{
    char *program_name;
    struct lirc_config *config;

    program_name="irxmms2";   //得到程序本身的名字,fix me ,here can get name by argv[0]and strformat

    //显示程序基础信息
    printf("Welcome To Use IRxmms2 :) \n");
    printf("Version is : %s \n", VERSION);
    printf("Logs in /var/log/user.log \n");
    
    openlog(program_name,LOG_PID|LOG_ODELAY|LOG_CONS,LOG_USER);
    if(argc>2)          //如果有一个以上的参数，说明有问题，程序退出
    {
        fprintf(stderr,"Usage: %s <config file>\n",program_name);
        syslog(LOG_ERR,"Usage: %s <config file>\n",program_name);
        exit(EXIT_FAILURE);
    }
    
    xmms2_conntion(program_name);
    //初始化连接lircd，如果连接成功，什么都不打印，如果连接失败，则在错误终端返回第二个参数的值(非0)
    //该函数正常返回lircd的socket的文件描述符，错误情况下返回-1
    if(lirc_init(program_name,10)==-1)
    {
        syslog(LOG_ERR,"Error: Code:101.\n");
        syslog(LOG_INFO,"Lirc API init is failed! Maybe your memory not enough!\n");    
        exit(EXIT_FAILURE); 
    }

    //lirc_readconfig(char *file,struct lirc_config **config,int (check)(char *s));
    //该函数加载配置,可以多次执行以加载多个配置;该函数执行成功返回0，失败返回-1;
    //file指要加载的配置的绝对路径；可以使用NULL代表默认配置.lircrc;
    //第二个参数是将配置文件数据的指针地址传递给本程序后面调用，例如lirc_code2char() 函数就需要这个参数;
    //第三个check参数是检查配置文件的语法正确性;如果不检查则使用NULL；如果配置文件有语法错误返回-1,配置正确返回0;
    if(lirc_readconfig(argc==2 ? argv[1]:NULL,&config,NULL)==0)
    {
        char *code;
        char *c;
        int ret;
        //int lirc_nextcode(char **code); 该函数直到lircd上有可用信息时才会阻塞，因此常用在主程序的循环中;
        //code是指在数据流中下一个可用的字符串，该函数正常返回0，错误(如lircd接口关闭)返回-1;
        //你的程序必须通过free(3)函数释放字符串，如果没有完整的可用字符串，则设置为NULL;
        //
        while(lirc_nextcode(&code)==0)
        {
            if(code==NULL) continue;
            //int lirc_code2char(struct lirc_config *config,char *code,char **string);
            //该函数用于当收到红外信号后，获取配置文件里面对应信号的config字段;
            //config是配置文件数据结构的指针,即lirc_readconfig函数返回的内容;
            //code是lircd传递给本程序的code值；
            //string就是配置文件里面config字段的部分
            //因为用户也许会一个键按多次，因此必须要等到NULL字段的返回(即用户活动停止)或者产生了错误再继续
            //该函数正常返回0，错误返回-1
            while((ret=lirc_code2char(config,code,&c))==0 && c!=NULL)
            {
#ifdef DEBUG
                if (strcmp(c,"play") == 0)
                    printf("Hello \"%s\"\n",c);
                else if (strcmp(c,"KEY_1") == 0)
                    printf("Hello \"%s\"\n",c);
                else
                    printf("Hello \"%s\"\n",c);

                sleep(1);
                c = NULL;
#endif
            }
            free(code);
            if(ret==-1) break;
        }
        //void lirc_freeconfig(struct lirc_config *config);该函数释放config变量
        lirc_freeconfig(config);
    }
    //int lirc_deinit();该函数断开和lircd的连接并进行内部的清理
    lirc_deinit();
    closelog();
    exit(EXIT_SUCCESS);
}
