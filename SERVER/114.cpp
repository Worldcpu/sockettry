#include<bits/stdc++.h>
#include<string.h>
#include<cstring>
#include<winsock2.h>
#include<windows.h>
#include <pthread.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib")
#define STSIZE 512
#define MAXTHREAD 4
string nn;
bool newname=false;
void error(){
    cout<<"错误代码："<<WSAGetLastError;
}
int loopread(SOCKET socket,char *buff,int length){//阻滞recv
    int nleft,nread;
    nleft=length;
    while(nleft>0){
        nread=recv(socket,buff,nleft,0);
        if(nread==0){
            return -1;
        }
        if(nread<0){
            if(errno==EINTR||errno==EAGAIN||errno==EWOULDBLOCK){
                continue;
            }
            return -1;
        }
        nleft-=nread;
        buff+=nread;
    }
    return(length-nleft);
}
SOCKET client;//方便线程自行读取，线程会自己存一份自己用的
short i=0;
void *tx(void* args){
    SOCKET c=client;
    string names,mess,mesage;
    char name[STSIZE]{},mes[STSIZE]{};
    int rec=loopread(c,name,STSIZE);//收名字
    if(rec!=-1&&rec!=0){
        names=string(name);
        nn="[御坂服务器]欢迎"+string(name)+"加入御坂网络!";
        send(c,nn.data(),STSIZE,0);//发欢迎信息
    }else{
        pthread_exit(NULL);//推出现场
    }
    cout<<"欢迎"<<names<<"加入御坂网络！"<<endl;
    do{
        rec=loopread(c,mes,STSIZE);
        if(strcmp(mes,"close")==0){//退出连接指令读取
            cout<<names<<"断开了连接"<<endl;
            break;
        }
        mess=string(mes);//暂改
        cout<<"["<<names<<"]:"<<mess<<endl;
        //todo——实现客户端收消息
    }while(rec!=SOCKET_ERROR&&rec!=0);
    pthread_exit(NULL);
}
int main(){
    system("chcp 65001");
    system("cls");
    system("title Misaka Network");
    pthread_t thr[MAXTHREAD];
    cout<<"Misaka Network TCP 1 to 4 b1.0.0 Server"<<endl;
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);
    SOCKET s=socket(AF_INET,SOCK_STREAM,0);
    if(s==INVALID_SOCKET){
        cout<<"服务端套接口创建失败！"<<endl;
        error();
        return 0;
    }else{
        cout<<"服务端WSA初始化成功！"<<endl;
    }
    sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    saddr.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
    saddr.sin_port=htons(7777);
    int sadlen=sizeof(sockaddr_in);
    if(bind(s,(sockaddr*)&saddr,sadlen)!=0){
        cout<<"服务端套接口绑定ip与端口信息失败！"<<endl;
        error();
        return 0;
    }else{
        cout<<"服务端接口绑定ip与端口成功"<<endl;
    }
    if(listen(s,4)!=0){
        cout<<"监听失败！"<<endl;
        error();
    }else{
        cout<<"开始监听"<<endl;
    }
    sockaddr_in caddr;
    while(1){
        client=accept(s,(sockaddr*)NULL,NULL);
        if(client==INVALID_SOCKET){
            cout<<"连接失败"<<endl;
        }else{
            cout<<"连接成功"<<endl;
            i+=1;
            if(pthread_create(&thr[i],NULL,tx,NULL)!=0){//创建线程，用i构建假for循环
                cout<<"PTHREAD FAIL!";
            }
        }
    }
    closesocket(client);
    closesocket(s);
    WSACleanup();
    cout<<"Exiting...";
    pthread_exit(NULL);
    system("pause");
    return 0;
}
