#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/epoll.h>

#define BUF_SIZE 30
#define REQUEST_SIZE 3
#define RESPONSE_SIZE 3
#define EPOLL_SIZE 10
void error_handling(char *message);
int get_udp_sock(int port);
int dispose(char *msg, int len, int *vol_val);

int main(int argc, char *argv[]){

    char message[BUF_SIZE];
    int str_len;
    char buf[BUF_SIZE];
    int buf_len;

    int serv_sock, i, flag;
    struct sockaddr_in clnt_adr;
    socklen_t clnt_adr_sz;

    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    int voltage_val = 0;
    int vol_fd, len;

    int LED43_fd, LED44_fd;
    int LED43_val, LED44_val;

    if(argc!=2){
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
   
    serv_sock = get_udp_sock(atoi(argv[1]));

    epfd=epoll_create(EPOLL_SIZE);//其实参数没用
    ep_events=malloc(sizeof(struct epoll_event)*EPOLL_SIZE);//用于存储发生改变的文件描述符

    event.events=EPOLLIN;
    event.data.fd=serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);
    
    while(1){
        vol_fd = open("/sys/devices/platform/c0000000.soc/c0053000.adc/iio:device0/in_voltage7_raw", 0);
        if(vol_fd<0){
            error_handling("volfile open error");
            exit(1);
        }
        len = read(vol_fd, buf,sizeof buf - 1);
        if(len > 0){
            voltage_val = atoi(buf);
        }
        close(vol_fd);
        //printf("%d\n", voltage_val);

        event_cnt=epoll_wait(epfd, ep_events, EPOLL_SIZE, 0);
        if(event_cnt == -1){
            puts("epoll_wait() error");
			break;
        }

        for(i=0; i<event_cnt; ++i){//便于以后扩展
            clnt_adr_sz = sizeof(clnt_adr);
            str_len=recvfrom(serv_sock, message, REQUEST_SIZE, 0, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
            flag = dispose(message, str_len, &voltage_val);
            sendto(serv_sock, message, RESPONSE_SIZE, 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);
        }

        if(voltage_val <= 1000 || voltage_val >= 4000 ){
            system("play 10986.mp3");
        }
        /* 
         * 此处用于编写蜂鸣器报警功能。
         * 
         */
    }
    
    close(serv_sock);
    close(epfd);
    return 0;
}
/* dispose(char *msg, int len)
 * 用于处理请求消息并回复
 * 
 * 请求数据报长度：3个字节
 * 第一个字节表示请求类型：
 * 1表示请求电压值，2表示改变灯泡亮灭
 * 当第一个字节为1时，
 * 第二,三个字节默认填充1
 * 当第一个字节为2时，
 * 第二个字节用于指定灯泡：
 * 43表示LED43, 44表示LED44.
 * 第三个字节用于指定灯泡亮灭:
 * 1表示让灯亮,2表示让灯灭.
 * 当第一个字节为3时，
 * 第二,三个字节默认填充1
 * 用于模拟电压异常的情况
 * 
 * 响应数据报长度：3
 * 第一个字节表示响应类型：
 * 1表示响应电压值，2表示回复灯泡亮灭
 * 当第一个字节为1时，
 * 后两个字节一起表示电压值，第二个字节为高位
 * 当第一个字节为2时，
 * 第二个字节用于指定灯泡：
 * 43表示LED43, 44表示LED44.
 * 第三个字节用于表示是否成功:
 * 1表示成功, 0表示失败.
 * 当第一个字节为3时，
 * 后两个字节一起表示电压值，第二个字节为高位
 * 用于返回异常的电压值。
 */
int dispose(char *msg, int len, int *volval){
    if(len != 3){
        puts("illegal requet");
        return 1;
    }

    int request_type = msg[0];
    if(request_type == 1){
        msg[1] = (*volval)/100;
        msg[2] = (*volval)%100;
        msg[3] = 0;
        printf("Send to client volval: %d\n", (*volval));
    }else if(request_type == 2){
        int led_fd;
        char path[50] = "/sys/class/gpio/gpio";
        char buf[10] = {0};

        path[20] = msg[1]/10 + '0';
        path[21] = msg[1]%10 + '0';
        strcat(path, "/value");
        buf[0] = msg[2] + '0' - 1;
        buf[1] = 0;
        
        printf("%s\n", buf);
        led_fd = open(path, O_WRONLY);
        write(led_fd, buf, strlen(buf));
        close(led_fd);
        
        printf("gpio%d: %s\n", msg[1], (msg[2] == 2) ? "off":"on");
        msg[2] = 1;
        msg[3] = 0;
    }else if(request_type == 3){
        (*volval) = 4500;
        msg[1] = (*volval)/100;
        msg[2] = (*volval)%100;
        msg[3] = 0;
        printf("Warning! volval: %d\n", (*volval));
    }else{
        puts("illegal requet");
        return 1;
    }
    return 0;
}
/*
 *error_handling(char *message)
 *用于打印错误。
 */
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
/*
 *get_udp_sock(int port)
 *得到用于监听特定端口的套接字。
 */
int get_udp_sock(int port){

    int tmp_serv_sock;
    struct sockaddr_in serv_adr;

    tmp_serv_sock=socket(PF_INET, SOCK_DGRAM, 0);
                    //协议族，套接字类型，
    if(tmp_serv_sock == -1)
        error_handling("UDP socket creation error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(port);

    if(bind(tmp_serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))== -1)
        error_handling("bind() error");
    return tmp_serv_sock;
}