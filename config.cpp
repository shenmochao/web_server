#include "config.h"

Config::Config(){
    //端口号,默认9006
    PORT = 9006;

    //日志写入方式，默认异步(1)
    LOGWrite = 1;

    //触发组合模式(1),默认listenfd LT + connfd ET
    TRIGMode = 1;

    //listenfd触发模式，默认LT(0)
    LISTENTrigmode = 0;

    //connfd触发模式，默认ET(1)
    CONNTrigmode = 1;

    //优雅关闭链接，默认不使用
    OPT_LINGER = 0;

    //数据库连接池数量,默认8
    sql_num = 8;

    //线程池内的线程数量,默认8
    thread_num = 16;

    //关闭日志,默认关闭(1) 打开(0)
    close_log = 1;

    //并发模型,默认是proactor
    actor_model = 0;
}

void Config::parse_arg(int argc, char*argv[]){
    int opt;
    const char *str = "p:l:m:o:s:t:c:a:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            PORT = atoi(optarg);
            break;
        }
        case 'l':
        {
            LOGWrite = atoi(optarg);
            break;
        }
        case 'm':
        {
            TRIGMode = atoi(optarg);
            break;
        }
        case 'o':
        {
            OPT_LINGER = atoi(optarg);
            break;
        }
        case 's':
        {
            sql_num = atoi(optarg);
            break;
        }
        case 't':
        {
            thread_num = atoi(optarg);
            break;
        }
        case 'c':
        {
            close_log = atoi(optarg);
            break;
        }
        case 'a':
        {
            actor_model = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
}