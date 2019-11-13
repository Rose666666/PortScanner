#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#define PORT 8089
#define IP "127.0.0.1"
#define BACKLOG 1
#define MAXPORT 65535
#define MINPORT 0
struct sc_index
    {
        char* ip;
        int* connect_sock;
        int min;
        int max;
    };
//用于端口转换为字符串发送给客户端的字符串转换函数，用于传输字符串给客户端
char* NumtoString(int num,char *str)//10进制 
{
	int i = 0;//指示填充str 
	if(num<0)//如果num为负数，将num变正 
	{
		num = -num;
		str[i++] = '-';
	} 
	//转换 
	do
	{
		str[i++] = num%10+48;//取num最低位 字符0~9的ASCII码是48~57；简单来说数字0+48=48，ASCII码对应字符'0' 
		num /= 10;//去掉最低位	
	}while(num);//num不为0继续循环
	
	str[i] = '\0';
	
	//确定开始调整的位置 
	int j = 0;
	if(str[0]=='-')//如果有负号，负号不用调整 
	{
		j = 1;//从第二位开始调整 
		++i;//由于有负号，所以交换的对称轴也要后移1位 
	}
	//对称交换 
	for(;j<i/2;j++)
	{
		//对称交换两端的值 其实就是省下中间变量交换a+b的值：a=a+b;b=a-b;a=a-b; 
		str[j] = str[j] + str[i-1-j];
		str[i-1-j] = str[j] - str[i-1-j];
		str[j] = str[j] - str[i-1-j];
	} 
	
	return str;//返回转换后的值 
}
//扫描程序
void scanner(char* ip,int* connect_socket,int min,int max){
    struct sockaddr_in server_addr;
    int fake_client_socket;
    int server_addr_size;
    char portchar_in[5];
    char *portchar_out;
    //判断端口是否合理
    if(min>max||min<MINPORT||max>MAXPORT){
        printf("ERROR PORT IS %d,%d",min,max);
        perror("input port error! (out of range)\n");
        exit(-1);
    }
    //生成TCP套接字
    printf("mix:%d,max:%d\n",min,max);
    for(int port=min;port<=max;port++){
        if(port == 65536){
            pthread_exit(NULL);
            break;
        }
    if((fake_client_socket=socket(AF_INET,SOCK_STREAM,0))==-1){
        printf("server socket error!\n");
        perror("server socket error!\n");
        exit(-1);
    }
    // for(int port=9990;port<=10000;port++){
    server_addr.sin_family = AF_INET;  //套接字地址协议族
    server_addr.sin_port = htons(port);//套接字端口
    server_addr.sin_addr.s_addr =inet_addr(ip);//*((struct in_addr *) he->h_addr);//inet_addr(ip); //套接字IP地址
    server_addr_size = sizeof(struct sockaddr); //套接字地址长度
    if(connect(fake_client_socket,(struct sockaddr*)&server_addr,server_addr_size)==-1){
        //printf("port:%d port closed!\n",port);
        // perror("port closed!");
        // printf("\n");
    }else{
        portchar_out = NumtoString(port,portchar_in); //将端口号转换为字符串
         strcat(portchar_out," is opened !\n");
         printf("%s",portchar_out);
         send(*connect_socket,portchar_out,strlen(portchar_out),0);
         printf("length of transportation is %d\n",(int)strlen(portchar_out)); //输出传输字节数
         if(port == 65535){
             send(*connect_socket,"END TO SCAN!\n",strlen("END TO SCAN!\n"),0); //告诉客户端扫描结束
         }
    }
     close(fake_client_socket);
    }
    

}
//scanner函数封装 为了实现多线程
void*exec_scanner1(struct sc_index* s_i){
    printf("IP is %s\n",s_i->ip);
    printf("min:%d,max:%d\n",s_i->min,s_i->max);
    scanner(s_i->ip,s_i->connect_sock,s_i->min,s_i->max);
    printf("进程1结束\n");
    pthread_exit(NULL);
}
void*exec_scanner2(struct sc_index* s_i){
    printf("IP is %s\n",s_i->ip);
    printf("min:%d,max:%d\n",s_i->min,s_i->max);
    scanner(s_i->ip,s_i->connect_sock,s_i->min,s_i->max);
    printf("进程2结束\n");
    pthread_exit(NULL);
}
void*exec_scanner3(struct sc_index* s_i){
    printf("IP is %s\n",s_i->ip);
    printf("min:%d,max:%d\n",s_i->min,s_i->max);
    scanner(s_i->ip,s_i->connect_sock,s_i->min,s_i->max);
    printf("进程3结束\n");
    pthread_exit(NULL);
}
int main() {
    struct sockaddr_in server_addr,client_addr; //服务端套接字地址结构
    int server_socket;  //服务端监听套接字描述符
    int connect_socket; //服务端连接套接字描述符
    int server_address_size;
    int client_address_size;
    //FILE* file;
    //生成TCP套接字
    pthread_t thread1, thread2,thread3;
    int ret_thrd1, ret_thrd2,ret_thrd3;
    struct sc_index scan_index1,scan_index2,scan_index3;
   // struct sc_index* index1,index2,index3;
    if((server_socket=socket(AF_INET,SOCK_STREAM,0))==-1){
        perror("server socket error!\n");
        exit(-1);
    }
    int opt = SO_REUSEADDR;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //设置套接字地址标识
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;  //套接字地址协议族
    server_addr.sin_port = htons(PORT);//套接字端口
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //套接字IP地址
    //server_address_size = sizeof(server_addr); //套接字地址长度
    //绑定套接字到对应地址结构上
    if(bind(server_socket,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))==-1){
         perror("bind error!\n");
         exit(-1);
    }
    //使监听套接字处于监听状态
    if(listen(server_socket,BACKLOG)==-1){
        perror("listen error!\n");
    }else
    {
        printf("监听成功\n");
    }
    
    //监听到连接后从连接请求队列中取出队头给进程
    client_address_size = sizeof(struct sockaddr_in);
   while(1){
         if((connect_socket=accept(server_socket,(struct sockaddr*)&client_addr,(socklen_t*)&client_address_size))==-1){
        perror("accept error!\n");
        exit(-1);
    }else
    {
        printf("成功连接客户端！\n");
        send(connect_socket,"You have connected to Server!\n",strlen("You have connected to Server!\n"),0);
        //close(connect_socket);
       // break;
       // file = fopen("/Users/air/Documents/网络安全与技术/portscan.rtf","w");
    }
    while (1){
        //接收开始扫描标志
        char start_buffer[6];
        char* end_buffer[4];
        int recv_count1;
        int recv_count2;
        char ip_buffer[16];
        int recv_count3;
        if((recv_count1 = (int)recv(connect_socket,start_buffer,5,0)) == -1){
        perror("recv error!\n");
            break;//continue;
        }else{
            start_buffer[recv_count1] = '\0';   //recv函数接收到的字符串是没有空字符的需要手动加上
            printf("length is %d\n",recv_count1);
            if(strcmp(start_buffer,"start")==0){
             /*start to scan*/ //扫描结果打印到文件中
             printf("start successfully!\n");
                if((recv_count3 = (int)recv(connect_socket,ip_buffer,15,0)) == -1){
                    continue;
                }else{
                ip_buffer[recv_count3] = '\0';
                printf("the destination ip is %s\n",ip_buffer);
                scan_index1.ip = ip_buffer;
                scan_index1.connect_sock = &connect_socket;
                scan_index1.min = 0;
                scan_index1.max = 20000;

                scan_index2.ip = ip_buffer;
                scan_index2.connect_sock = &connect_socket;
                scan_index2.min = 20001;
                scan_index2.max = 40000;

                scan_index3.ip = ip_buffer;
                scan_index3.connect_sock = &connect_socket;
                scan_index3.min = 40001;
                scan_index3.max = 65535;

                ret_thrd1 = pthread_create(&thread1, NULL, (void *)&exec_scanner1, (void*)&scan_index1);
                ret_thrd2 = pthread_create(&thread2, NULL, (void *)&exec_scanner2, (void*)&scan_index2);
                ret_thrd3 = pthread_create(&thread3, NULL, (void *)&exec_scanner3, (void*)&scan_index3);

                // 线程创建成功，返回0,失败返回失败号
                if (ret_thrd1 != 0) {
                    printf("线程1创建失败\n");
                } else {
                    printf("线程1创建成功\n");
                }

                if (ret_thrd2 != 0) {
                    printf("线程2创建失败\n");
                } else {
                    printf("线程2创建成功\n");
                }
                if (ret_thrd3 != 0) {
                    printf("线程3创建失败\n");
                } else {
                    printf("线程3创建成功\n");
                }
                  pthread_join(thread1, NULL);
                  pthread_join(thread2, NULL);
                  pthread_join(thread3, NULL);

                 //scanner(ip_buffer,&connect_socket,0,65535);
                 break;//continue; //如果开始扫描则跳转下一次循环，为下一次扫描提供开始条件
                } //接收IP地址最大长度的字节           
        }else{
            perror("Fail to start!\n");
            //continue;
            break;
        }

        }
        //接收结束扫描标志
        //if((recv_count2 = (int)recv(connect_socket,start_buffer,3,0)) != -1){
        //end_buffer[recv_count2-1] = '\0';
        //printf("%s",end_buffer);
        //if(strcmp(end_buffer,"end")){
        //    break;
        //}
        //}
        

    }
    close(connect_socket);
    break;
    }
    close(server_socket);
    return 0;
}
