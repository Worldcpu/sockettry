#include<winsock2.h>
#include<iostream>
#include<windows.h>
#include<cstring>
#include<thread>
using namespace std;
#pragma comment(lib,"Ws2_32.lib")
#define STSIZE 512
int loopread(SOCKET socket,char *buff,int length){
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
bool cect=true;
void recvfroms(SOCKET sockk){//todo——实现客户端与服务端收发信息，改pthread多线程
    cout<<"YESSSS!";
    char recie[STSIZE];
    string message;
    int ret;
    bool swift=true;
    while(1){
        memset(recie,0,STSIZE);
        loopread(sockk,recie,STSIZE);
        if(strcmp(recie,"closeall")){
            cect=false;
            break;
        }else if(cect==false){
            break;
        }
        message=string(recie);
        cout<<message<<'\n';
    }
}
int main(){
    system("chcp 65001");
    system("cls");
    system("title Misaka Network");
    cout<<"Misaka Network TCP 1 to 1 b1.0.0 Client"<<'\n';
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);
    SOCKET client=socket(AF_INET,SOCK_STREAM,0);
    if(client==INVALID_SOCKET){
        cout<<"套接口创建client错误！";
        WSACleanup();
        return 1;
    }else{
        cout<<"套接口创建成功"<<'\n';
    }
    sockaddr_in c;
    c.sin_family=AF_INET;
    c.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
    c.sin_port=htons(7777);
    int len=sizeof(c);
    short swit;
    SOCKET server;
    reconnect:
    if(connect(client,(SOCKADDR*)&c,len)!=0){
        system("title Misaka Network");
        cout<<WSAGetLastError<<'\n';
        cout<<"连接失败！"<<'\n'<<"是否进行重连(1重连，0退出):";
        rechose:
        cin>>swit;
        if(swit==0){
            closesocket(client);
            WSACleanup();
            return 0;
        }else if(swit==1){
            goto reconnect;
        }else{
            cout<<"小子别闹,我再给你一次机会:";
            system("title 老弟别闹");
            goto rechose;
        }
    }else{
        cout<<"连接成功"<<'\n';
    }
    string close="close",n;
    char name[STSIZE]{};
    memset(name,0,STSIZE);
    cout<<"请输入用户名:";
    cin>>name;
    n=string(name);
    int data;
    data=send(client,name,STSIZE,0);
    if(data!=SOCKET_ERROR&&data!=0){
        cout<<"欢迎加入御坂网络！输入/close断开链接";
    }
    thread mythread(recvfroms,client);
    cout<<'\n';
    bool swift=true;
    char sendbuf[STSIZE],recvbuf[STSIZE];
    memset(recvbuf,0,STSIZE);
    do{
        memset(sendbuf,0,STSIZE);
        cout<<"Enter a word:";
        cin.getline(sendbuf,STSIZE);
        if(strcmp(sendbuf,"/close")==0||cect==false){
            send(client,close.data(),STSIZE,0);
            cect=false;
            cout<<"断开连接"<<'\n';
            break;
        }else{
            send(client,sendbuf,STSIZE,0);
            cout<<'\n';
        }
    }while(1);
    closesocket(client);
    WSACleanup();
    mythread.join();
    cout<<"EXIT";
    system("pause");
    return 0;
}