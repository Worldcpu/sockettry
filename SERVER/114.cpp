#include<bits/stdc++.h>
#include<map>
#include<string.h>
#include<cstring>
#include<winsock2.h>
#include<windows.h>
#include<pthread.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib")
#define STSIZE 512
#define MAXTHREAD 10
bool thrr[MAXTHREAD];
map<short,SOCKET> thrsocket;
bool thrbool[MAXTHREAD];
//！！！！！！！！！TODO 将thrr与他们所对应的线程队列对应起来(用map优化)
string gbmessage;//用于缓存要广播的数据
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
bool guangbo=false,shutoffgb=false;//左侧广播信号，由广播线程监听。右侧关闭广播线程
SOCKET clientcopy[MAXTHREAD];//方便广播
short pos=0,ans=0;
void *gb(void* args){
    while(shutoffgb!=true){
        while(guangbo==false);
        for(short i=1;i<=ans;i++){
            if(ans==0){//防止手欠
                break;
            }
            if(thrbool[i]==true){//检测是否有线程
                // cout<<i<<"号线程有占用!\n";
                if(send(thrsocket[i],gbmessage.data(),STSIZE,0)!=0){
                    // cout<<"广播发送失败！\n";
                }
            }
        }
    guangbo=false;//信号归零
    }
    pthread_exit(NULL);
}
void *tx(void* args){
    bool forfirsttime=true;
    short positon=pos;
    // SOCKET c=clientcopy[positon];//线程缓存一份不用线程同步
    string mesage;
    char name[STSIZE]{},mes[STSIZE]{};
    int rec=loopread(thrsocket[positon],name,STSIZE);//收名字
    if(rec!=-1&&rec!=0){
        if(guangbo==true){
            while(guangbo=false);//等待广播完毕
        }
        gbmessage="[御坂服务器]欢迎"+string(name)+"加入御坂网络!";
        // cout<<"线程"<<positon<<"激活广播\n";
        guangbo=true;
    }else{
        pthread_exit(NULL);//推出现场
    }
    cout<<"欢迎"<<name<<"加入御坂网络！"<<endl;
    do{
        rec=loopread(clientcopy[positon],mes,STSIZE);
        if(strcmp(mes,"/close")==0){//退出连接指令读取与识别
            cout<<name<<"断开了连接"<<endl;
            rec=-1;
            if(guangbo==true) while(guangbo=false);
            gbmessage=string(name)+"断开了连接";
            guangbo=true;
            break;
        }
        if(forfirsttime&&rec!=SOCKET_ERROR&&rec!=0){//优化输出格式
            forfirsttime=false;
            continue;
        }
        cout<<"["<<name<<"]:"<<mes<<endl;
        if(guangbo==true){
            while(guangbo=false);
        }
        gbmessage="["+string(name)+"]:"+string(mes);//广播其他人的信息
        guangbo=true;
    }while(rec!=SOCKET_ERROR&&rec!=0);
    thrr[positon]=false;
    // cout<<"线程"<<positon<<"退出\n";
    pthread_exit(NULL);
}
int main(){
    system("chcp 65001");
    system("cls");
    system("title Misaka Network");
    for(int i=1;i<=MAXTHREAD;i++){
        thrbool[i]=false;
        // cout<<thrr[i];
    }
    pthread_t thr[MAXTHREAD];
    string disc=".disconnect",acce=".accept";
    cout<<"Misaka Network TCP 1 to 4 b1.0.0 Server"<<endl;
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);
    SOCKET s=socket(AF_INET,SOCK_STREAM,0);
    if(s==INVALID_SOCKET){
        cout<<"服务端套接口创建失败！"<<endl;
        cout<<"错误代码："<<WSAGetLastError;
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
        cout<<"错误代码："<<WSAGetLastError;
        return 0;
    }else{
        cout<<"服务端接口绑定ip与端口成功"<<endl;
    }
    if(listen(s,5)!=0){
        cout<<"监听失败！"<<endl;
        cout<<"错误代码："<<WSAGetLastError<<endl;
    }else{
        cout<<"开始监听"<<endl;
    }
    SOCKET client;
    if(pthread_create(&thr[MAXTHREAD],NULL,gb,NULL)!=0) cout<<"PTHREAD FAIL!";
    while(1){
        client=accept(s,(sockaddr*)NULL,NULL);
        if(client==INVALID_SOCKET){
            cout<<"连接失败"<<endl;
        }else{
            for(short i=1;i<MAXTHREAD;i++){//设置线程被占用
                if(thrbool[i]==false){
                    // thrr[i]=true;
                    thrbool[i]=true;
                    thrsocket[i]=client;
                    pos=i;
                    ans=max(ans,pos);
                    send(client,acce.data(),STSIZE,0);
                    if(pthread_create(&thr[pos],NULL,tx,NULL)!=0){//创建线程，用i构建假for循环
                        cout<<"PTHREAD FAIL!";
                    }
                    break;
                }
                if(i+1==MAXTHREAD){
                    send(client,disc.data(),STSIZE,0);
                    break;
                }
            }//防守欠
            // cout<<"客户端连接成功,占用第"<<pos<<"线程 历史最高线程使用数"<<ans<<'\n'<<endl;
            // clientcopy[pos]=client;//pos这里是线程的顺序
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
