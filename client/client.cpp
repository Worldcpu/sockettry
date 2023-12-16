#include<bits/stdc++.h>
#include<winsock2.h>
#include<windows.h>
#include<pthread.h>
using namespace std;
#pragma comment(lib,"Ws2_32.lib")
#define STSIZE 512
#define MAXTHREAD 4
#define MAXCOMMANDLIST 15
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
char sendbuf[STSIZE]{},beatmes[14]=".checkconnect";
string commandlist[MAXCOMMANDLIST]={"msg"};
bool closemain=false,sacceptcommmand=0;
short commandflag=0;
void commandexcute(char pd[6],short charpos){
    if(strstr(pd,"msg")!=NULL){
        int ipos,ans=0;
        char namebuff[STSIZE],messagebuff[STSIZE],msgmode[5]=".msg";
        memset(namebuff,0,STSIZE);
        memset(messagebuff,0,STSIZE);
        for(int i=charpos;i<=STSIZE;i++){
            if(sendbuf[i]==32){
                break;
            }else if(sendbuf[i]==0){
                cout<<"指令格式错误！/msg 私聊发送者姓名 私聊信息"<<endl;
                return;
            }
            // cout<<"COMMAND_SENDBUF:"<<sendbuf[i]<<endl;
            namebuff[ans]=sendbuf[i];
            ans++;
            ipos=i;
        }
        ans=0;
        // cout<<"name:"<<namebuff<<endl;
        if(sendbuf[ipos+2]==0||sendbuf[ipos+2]==32){
            cout<<"指令格式错误！/msg 私聊发送者姓名 私聊信息"<<endl;
            return;
        }else ipos+=2;
        for(int i=ipos;i<=STSIZE;i++){
            if(sendbuf[i]==NULL){
                break;
            }
            messagebuff[ans]=sendbuf[i];
            ans++;
        }
        // cout<<"message:"<<messagebuff<<endl;
        send(client,msgmode,STSIZE,0);
        while(sacceptcommmand==0);
        // cout<<"发送私信名称\n";
        send(client,namebuff,STSIZE,0);
        while(commandflag==0);
        if(commandflag!=1){
            // cout<<"无用户名\n";
            return;
        }
        // cout<<"发送信息\n";
        send(client,messagebuff,STSIZE,0);
        commandflag=0;
        sacceptcommmand=0;
    }
}
bool heartbeating=false,txisexcute=false,recvieeserver=false,lostconnect=false;
void *heartbeat(void* args){
    time_t starttime,stoptime;
    while(1){
        Sleep(5000);
        if(txisexcute==true){
            // cout<<"txisnotok";
            while(txisexcute);
        }
        else{
            // cout<<"txisok";
        } 
        // cout<<"发送beat"<<endl;
        send(client,beatmes,STSIZE,0);
        starttime=time(NULL);
        while(recvieeserver==false){
            stoptime=time(NULL);
            if(stoptime-starttime==5){
                // cout<<"时间到\n";
                break;
            }
        }
        if(recvieeserver==false){
            // cout<<"连接超时,";
            lostconnect=true;
            heartbeating=false;
            closemain=true;
            break;
        }else{
            // cout<<"检测成功";
            heartbeating=false;
            recvieeserver=false;
            lostconnect=false;
        }
    }
    // cout<<"结束\n";
    pthread_exit(NULL);
}
void *txx(void* args){
    char close[6]="close";
    cout<<'\n';
    bool swift=true;
    char recvbuf[STSIZE]{},panduan[6]{};
    memset(recvbuf,0,STSIZE);
    bool cat=true;
    continuewhile:
    do{
        txisexcute=false;
        memset(sendbuf,0,STSIZE);
        memset(panduan,0,STSIZE);
        cin.getline(sendbuf,STSIZE);
        while(heartbeating){
            if(lostconnect==true);
            swift=false;
        }
        txisexcute=true;
        if(strcmp(sendbuf,"/close")==0||swift==false){
            send(client,sendbuf,STSIZE,0);
            cout<<"断开连接"<<'\n';
            closemain=true;
            // Sleep(0.5);
            break;
        }else if(sendbuf[0]=='/'){
            short charpos,commandpos=-1;
            for(int i=1;i<=STSIZE;i++){
                // cout<<"i++";
                if(sendbuf[i]==0||sendbuf[i]==32){
                    break;
                }
                panduan[i-1]=sendbuf[i];
                // cout<<"i="<<i<<"\npanduan[i]="<<panduan[i]<<"\nsendbuf[i]="<<sendbuf[i];
                charpos=i;
                // cout<<"\n";
            }
            // cout<<panduan<<"\n";
            if(sendbuf[charpos+1]==0){ 
                // cout<<"not passed\n";
                goto continuewhile;
            }
            for(int i=0;i<=MAXCOMMANDLIST;i++){
                if(strcmp(panduan,commandlist[i].data())==0){
                    commandpos=i;
                    break;
                }
            }
            if(commandpos!=-1) commandexcute(panduan,charpos+2);
        }
        else{
            send(client,sendbuf,STSIZE,0);
        }
    }while(swift);
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
    c.sin_port=htons(1145);
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
        cout<<"EXIT\n";
        system("pause");
        return 0;
    }else{
        if(strcmp(conbuff,".disconnect")==0){
            cout<<"服务器人数已满！\n";
            closesocket(client);
            WSACleanup();
            cout<<"EXIT\n";
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
                pthread_create(&thr[3],NULL,heartbeat,NULL);
                inital=true;
                continue;
            }
        }
        if(closemain==true){
            closesocket(client);
            WSACleanup();
            cout<<"EXIT\n";
            system("pause");
            return 0;
        }
        if(rect!=0&&rect!=SOCKET_ERROR&&strcmp(buff,".heartbeatok")==0){
            cout<<"收到beat\n";
            recvieeserver=true;
            continue;
        }
        if(rect!=0&&rect!=SOCKET_ERROR&&strcmp(buff,".acceptcommand")==0){
            sacceptcommmand=1;
            continue;
        }
        if(rect!=0&&rect!=SOCKET_ERROR&&strcmp(buff,".accept")==0){
            commandflag=1;
            continue;
        }else if(rect!=0&&rect!=SOCKET_ERROR&&strcmp(buff,".nousername")==0){
            cout<<"用户名不存在"<<endl;
            commandflag=2;
            continue;
        }
        if(rect!=0&&rect!=SOCKET_ERROR&&strstr(buff,".msgm")!=NULL){
            char msgbuff[STSIZE]{};
            for(int i=5;i<=STSIZE;i++){
                if(buff[i]==NULL) break;
                msgbuff[i-5]=buff[i];
            }
            cout<<msgbuff<<endl;
            continue;
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