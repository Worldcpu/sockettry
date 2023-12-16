#include<bits/stdc++.h>
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
char namelist[MAXTHREAD][STSIZE]{},heartbeatok[15]=".heartbeatok",acceptcomad[15]=".acceptcommand",samename[10]=".samename",nousername[12]=".nousername",disc[12]=".disconnect",acce[8]=".accept";
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
    bool forfirsttime=true,namecheck=true;
    short positon=pos;
    // SOCKET c=clientcopy[positon];//线程缓存一份不用线程同步
    char mes[STSIZE]{};
    rewait:
    // for(int i=0;i<STSIZE;i++){
    //     namelist[positon][i]=0;
    // }
    // cout<<"name1="<<namelist[positon]<<endl;
    // cout<<"进入read\n";
    int rec;//收名字
    rec=loopread(thrsocket[positon],namelist[positon],STSIZE);
    // cout<<"name2="<<namelist[positon]<<endl;
    if(rec!=-1&&rec!=0){
        for(int i=1;i<MAXTHREAD;i++){
            if(strcmp(namelist[i],namelist[positon])==0&&i!=positon){
                send(thrsocket[positon],samename,STSIZE,0);
                thrr[positon]=false;
                memset(namelist[positon],0,sizeof(namelist[positon]));
                goto rewait;
                namecheck=false;
            }
        }
        if(guangbo==true){
            while(guangbo=false);//等待广播完毕
        }
        gbmessage="[御坂服务器]欢迎"+string(namelist[positon])+"加入御坂网络!";
        // cout<<"线程"<<positon<<"激活广播\n";
        // cout<<"名称广播\n";
        guangbo=true;
     }else{
        pthread_exit(NULL);//推出现场
        }
    cout<<"欢迎"<<namelist[positon]<<"加入御坂网络！"<<endl;
    time_t start,stop;
    start=time(NULL);
    cout<<start<<endl;
    do{
        bool koo=false;
        memset(mes,0,STSIZE);
        // cout<<"进入循环\n";
        rec=0;
        rec=recv(thrsocket[positon],mes,STSIZE,0);
        stop=time(NULL);
        // cout<<"STOP:"<<stop<<" START:"<<start<<endl;
        // cout<<"收到mes："<<mes<<"\n";
        if(strcmp(mes,"/close")==0){//退出连接指令读取与识别
            cout<<namelist[positon]<<"断开了连接"<<endl;
            rec=-1;
            if(guangbo==true) while(guangbo=false);
            gbmessage=string(namelist[positon])+"断开了连接";
            guangbo=true;
            break;
        }
        if(stop-start>7){
            cout<<namelist[positon]<<"断开了连接(连接超时)"<<endl;
            rec=-1;
            if(guangbo==true) while(guangbo=false);
            gbmessage=string(namelist[positon])+"断开了连接";
            guangbo=true;
            break;
        }
        if(strcmp(mes,".msg")==0){
            memset(mes,0,sizeof(mes));
            send(thrsocket[positon],acceptcomad,STSIZE,0);
            short anss=1;
            while(1){
                anss=0;
                anss=loopread(thrsocket[positon],mes,STSIZE);
                if(anss>0){
                    break;
                }
            }
            // cout<<"mesname:"<<mes<<endl;
            char msgmesage[STSIZE]{};
            short namepos;
            bool wehave=false;
            for(int i=1;i<=MAXTHREAD;i++){
                if(strcmp(mes,namelist[i])==0){
                    namepos=i;
                    send(thrsocket[positon],acce,STSIZE,0);
                    wehave=true;
                }
            }
            if(wehave){
                while(1){
                    anss=0;
                    anss=loopread(thrsocket[positon],msgmesage,STSIZE);
                    if(anss>0){
                        break;
                    }
                }
                string buffsend=".msgm[来自于"+string(namelist[positon])+"的私信]:"+msgmesage,servercout="["+string(namelist[positon])+"对"+string(namelist[namepos])+"的私聊]:"+msgmesage;
                send(thrsocket[namepos],buffsend.data(),STSIZE,0);
                cout<<servercout<<endl;
            }else{
                send(thrsocket[positon],nousername,STSIZE,0);
            }
            continue;
        }
        if(strcmp(mes,".checkconnect")==0){
            send(thrsocket[positon],heartbeatok,STSIZE,0);
            start=time(NULL);
            continue;
        }
        if(forfirsttime&&rec!=SOCKET_ERROR&&rec!=0){//优化输出格式
            forfirsttime=false;
            continue;
        }
        if(rec!=0&&rec!=SOCKET_ERROR){
            cout<<"["<<namelist[positon]<<"]:"<<mes<<endl;
            if(guangbo==true){
                while(guangbo=false);
            }
            gbmessage="["+string(namelist[positon])+"]:"+string(mes);//广播其他人的信息
            // cout<<"收发广播\n";
            guangbo=true;
        }
    }while(1);
    thrr[positon]=false;
    memset(namelist[positon],0,sizeof(namelist[positon]));
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
    saddr.sin_port=htons(1145);
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
                    send(client,acce,STSIZE,0);
                    if(pthread_create(&thr[pos],NULL,tx,NULL)!=0){//创建线程，用i构建假for循环
                        cout<<"PTHREAD FAIL!";
                    }
                    break;
                }
                if(i+1==MAXTHREAD){
                    send(client,disc,STSIZE,0);
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
