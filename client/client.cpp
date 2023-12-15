#include<winsock2.h>
#include<iostream>
#include<windows.h>
#include<cstring>
#include<pthread.h>
using namespace std;
#pragma comment(lib,"Ws2_32.lib")
#define STSIZE 512
#define MAXTHREAD 4
SOCKET client;
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
bool closemain=false;
void *txx(void* args){
    char close[6]="close";
    cout<<'\n';
    bool swift=true;
    char sendbuf[STSIZE],recvbuf[STSIZE]{};
    memset(recvbuf,0,STSIZE);
    bool cat=true;
    do{
        memset(sendbuf,0,STSIZE);
        cin.getline(sendbuf,STSIZE);
        if(strcmp(sendbuf,"/close")==0||swift==false){
            send(client,sendbuf,STSIZE,0);
            cout<<"断开连接"<<'\n';
            closemain=true;
            // Sleep(0.5);
            swift=false;
        }else{
            send(client,sendbuf,STSIZE,0);
        }
    }while(swift);
    cout<<"DONE";
    pthread_exit(NULL);
}
bool stopcheck=false,checked=false,firstcheck=false;
char conbuff[STSIZE]{};
void *checkconnect(void* args){
    while(1){
        int ret=recv(client,conbuff,STSIZE,0);
        if(ret>0){
            checked=true;
            break;
        }
        if(stopcheck==true){
            break;
        }
    }
    pthread_exit(NULL);
}
char name[STSIZE]{};
void recin(){
    while(1){
        memset(name,0,sizeof(name));
        cout<<"请输入用户名:";
        cin>>name;
        if(name[0]=='.'||name[0]=='/'){
            cout<<"用户名首位不能为小数点或斜杠，请重新输入\n";
            continue;
        }else{
            break;
        }
    }
}
int main(){
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);
    pthread_t thr[MAXTHREAD];
    system("chcp 65001");
    system("cls");
    system("title Misaka Network");
    cout<<"Misaka Network TCP 1 to 1 b1.0.0 Client"<<'\n';
    client=socket(AF_INET,SOCK_STREAM,0);
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
    reconnect:
    firstcheck=false;
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
        }
    }else{
        cout<<"连接成功"<<'\n';
    }
    checked=false;
    memset(conbuff,0,sizeof(conbuff));
    pthread_create(&thr[2],NULL,checkconnect,NULL);
    recinname:
    memset(name,0,sizeof(name));
    cout<<"请输入用户名:";
    cin>>name;
    if(name[0]=='.'||name[0]=='/'){
        cout<<"用户名首位不能为小数点或斜杠，请重新输入\n";
        goto recinname;
    }
    stopcheck=true;
    if(checked==false){
        cout<<"连接超时，中断连接\n";
        closesocket(client);
        WSACleanup();
        cout<<"EXIT";
        system("pause");
        return 0;
    }else{
        if(strcmp(conbuff,".disconnect")==0){
            cout<<"服务器人数已满！\n";
            closesocket(client);
            WSACleanup();
            cout<<"EXIT";
            system("pause");
            return 0;
        }
    }
    checked=false;
    stopcheck=false;
    // if(data!=SOCKET_ERROR&&data!=0){
    //     cout<<"欢迎加入御坂网络！输入/close断开链接";
    // }
    // if(pthread_create(&thr[1],NULL,txx,NULL)!=0){
    //     cout<<"线程创建出错！，连接中止"<<endl;
    //     closesocket(client);
    //     WSACleanup();
    //     cout<<"EXIT";
    //     system("pause");
    //     return 0;
    // }
    char buff[STSIZE];
    bool ans=false,inital=false;
    rewhile:
    send(client,name,STSIZE,0);
    while(1){
        memset(buff,0,sizeof(buff));
        int rect=recv(client,buff,STSIZE,0);
        if(rect!=0&&rect!=SOCKET_ERROR&&!inital){
            if(strcmp(buff,".samename")==0){
                cout<<"用户名重复使用！请重新输入用户名以注册。\n";
                recin();
                goto rewhile;
            }else{
                cout<<"欢迎加入御坂网络！输入/close断开链接\n";
                if(pthread_create(&thr[1],NULL,txx,NULL)!=0){
                    cout<<"线程创建出错！，连接中止"<<endl;
                    closesocket(client);
                    WSACleanup();
                    cout<<"EXIT";
                    system("pause");
                    return 0;
                }
                inital=true;
                continue;
            }
        }
        if(closemain==true){
            pthread_join(thr[1],NULL);
            closesocket(client);
            WSACleanup();
            cout<<"EXIT";
            system("pause");
            return 0;
        }
        if(rect!=0&&rect!=SOCKET_ERROR){
            if(ans==false){
                cout<<'\n';
                ans=true;
            }
            cout<<buff<<endl;
        }
    }
}