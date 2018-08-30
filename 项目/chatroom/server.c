#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<mysql/mysql.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<time.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<stdio.h>
#include<pthread.h>
#include<errno.h>

int sock_fd;
int userfd;
int groupfd;

#define MAX_USER   100
#define MAX_GROUP  50
#define MAX_EPOLL  1000

#define SERV_PORT     4507  //服务端的端口
#define LISTENO       500    //连接请求队列的最大长度
#define INVALID_USERINFO   'n' //用户信息无效
#define VALLD_USERINFO     'y' //用户信息有效
#define ONLINE              1
#define DOWNLINE            0

#define CHARSIZE       1024




pthread_mutex_t mutex;
int lian=0;
struct userinfo              //保存用户信息结构体
{
    char username[20];
    char password[20];
    int status;             
    int conn_fd;
    char friends[50][20];  //好友信息
    int numfriend;          //好友数量
    char group[20][20];    //群信息
    int numgroup;           //群数量
    struct userinfo *next;
};
struct userinfo *userhead=NULL;
int usernum=0;

struct groupinfo
{
    char groupname[20];      //群名
    char groupowner[20];    //群主
    char member[50][20]; //成员
    int membernum;          //成员数量
    struct groupinfo *next;
};
struct groupinfo *grouphead=NULL;
int groupnum=0;


typedef struct record
{
    char from[20];
    char to [20];
    char time[30];
    char message[30];
}chatrecord;

typedef struct data
{
    int sendfd;
    int recvfd;
    int size;          //保存文件大小
    int nowsize;
    char from[20];     //发送方
    char to[20];       //接收方
    char name[50][20];
    char message[CHARSIZE];
}datas;

typedef struct pack
{
    char type;
    datas data;
}packet;

packet leavepack[200];
int le=0;

//时间函数
char *mytime()
{
    time_t now;
    time(&now);
    return (ctime(&now));
}

//自定义的错误处理函数
void my_err(const char *err_string,int line)
{
    fprintf(stderr,"line:%d ",line);
    perror(err_string);
    exit(1);
}

//发送数据包
void sendpack(packet *send_pack)
{
    int ret = 0;
    if((ret = send(send_pack->data.recvfd,send_pack,sizeof(packet),0))<0)
    {
        my_err("send",__LINE__);
    }
}


//将聊天记录保存到文件
void saverecord(char filename[20],chatrecord mes)
{
    int mesfd;
    if((mesfd=open(filename,O_RDWR|O_CREAT|O_APPEND,0777))==-1)
    {
        my_err("open",__LINE__);
        exit(1);
    }

    if(write(mesfd,&mes,sizeof(mes))<0)
    {
        my_err("write",__LINE__);
        exit(1);
    }
    close(mesfd);
}

//从文件中读取用户信息和群信息
int readfile()
{
    struct userinfo *p,*np;
    p=userhead;
    struct groupinfo *q,*nq;
    q=grouphead;
    struct userinfo temp;
    struct groupinfo tempp;
    int t,k=0;
    if((userfd=open("user.txt",O_RDONLY|O_CREAT,S_IRUSR|S_IWUSR))<0)
    {
        my_err("open",__LINE__);
        exit(1);
    }
    while((t=read(userfd,&temp,sizeof(struct userinfo)))>0)
    {
        temp.status=DOWNLINE;
        p=(struct userinfo *)malloc(sizeof(struct userinfo));
        *p=temp;
        if(k==0)
        {
            k=1;
            p->next=userhead;
            userhead=p;
            np=p;
        }
        else
        {
            p->next=NULL;
            np->next=p;
            np=p;
        }
    }
    close(userfd);

    k=0;
    if((groupfd=open("group.txt",O_RDONLY|O_CREAT,S_IRUSR|S_IWUSR))<0)
    {
        my_err("open",__LINE__);
        exit(1);
    }
    while((t=read(groupfd,&tempp,sizeof(struct groupinfo)))>0)
    {
        q=(struct groupinfo *)malloc(sizeof(struct groupinfo));
        *q=tempp;
        if(k==0)
        {
            k=1;
            q->next=grouphead;
            grouphead=q;
            nq=q;
        }
        else
        {
            q->next=NULL;
            nq->next=q;
            nq=q;
        }
    }
    close(groupfd);
}


//把信息写入文件 
int writefile()
{
    struct userinfo *p;
    p=userhead;
    if((userfd=open("user.txt",O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR))<0)
    {
        my_err("open",__LINE__);
        return -1;
    }
    while(p)
    {
        if(write(userfd,p,sizeof(struct userinfo))<0)
        {
            my_err("write",__LINE__);
            return -1;
        }
        p=p->next;
    }
    close(userfd);
    printf("user.txt have write\n");
    
    struct groupinfo *q;
    q=grouphead;
    if((groupfd=open("group.txt",O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR))<0)
    {
        my_err("open",__LINE__);
        return -1;
    }
    while(q)
    {
        if(write(groupfd,q,sizeof(struct groupinfo))<0)
        {
            my_err("write",__LINE__);
            return -1;
        }
        q=q->next;
    }
    close(groupfd);
    printf("group.txt have write\n");
}



//判断有没有此群,0表示没有,1表示有.
int judge_group(char *buf)
{
    int flag=0;
    struct groupinfo *p;
    p=grouphead;
    while(p)
    {
        if(strcmp(p->groupname,buf)==0)
        {
            flag=1;
            return 1;
        }
        p=p->next;
    }
    if(flag==0)
    {
        return 0;
    }
}

//判断某个群里有没有某个人,0表示没有,1表示有
int judge_groupren(char *buf,char *dst)
{
    int j;
    struct groupinfo *p;
    p=grouphead;
    while(p)
    {
        if(strcmp(p->groupname,buf)==0)
        {
            for(j=0;j<p->membernum;j++)
            {
                if(strcmp(p->member[j],dst)==0)
                {
                    return 1;
                }
            }
        }
        p=p->next;
    }
    return 0;
}

//返回某个人在某个群中的位置
int locategroup(char *buf,char *dest)
{
    struct groupinfo *p;
    p=grouphead;
    int j;
    while(p)
    {
        if(strcmp(p->groupname,buf)==0)
        {
            for(j=0;j<p->membernum;j++)
            {
                if(strcmp(p->member[j],dest)==0)
                {
                    return j;
                }
            }
        }
        p=p->next;
    }
}


//返回某个群在某个用户群组中的位置
int locateuser(char *dest,char *buf)
{
    struct userinfo *p;
    p=userhead;
    int j;
    while(p)
    {
        if(strcmp(p->username,dest)==0)
        {
            for(j=0;j<p->numgroup;j++)
            {
                if(strcmp(p->group[j],dest)==0)
                {
                    return j;
                }
            }

        }
        p=p->next;
    }
}

//好友添加请求的处理
void deal_adfriend(packet *recv)
{
    int zhuangtai;
    struct userinfo *p,*q;
    p=userhead;
    if(recv->data.message[0]=='y')
    {
        while(p)
        {
            if(strcmp(p->username,recv->data.from)==0)
            {
                p->numfriend++;
                strcpy(p->friends[p->numfriend-1],recv->data.to);
            }
            if(strcmp(p->username,recv->data.to)==0)
            {
                p->numfriend++;
                zhuangtai=p->status;
                strcpy(p->friends[p->numfriend-1],recv->data.from);
                recv->data.recvfd=p->conn_fd;
            }
            p=p->next;
        }
        printf("用户%s和%s成为好友!\n",recv->data.from,recv->data.to);
    }
    if(recv->data.message[0]=='n')
    {
        while(p)
        {
            if(strcmp(p->username,recv->data.to)==0)
            {
                zhuangtai=p->status;
                recv->data.recvfd=p->conn_fd;
            }
            p=p->next;
        }
    }
    if(zhuangtai==ONLINE)
    {
        sendpack(recv);
    }
    if(zhuangtai==DOWNLINE)
    {
        leavepack[le++]=*recv;
    }
}

//添加好友
void addfriend(packet *recv)
{
    int zhuangtai;
    int flag=0;
    struct userinfo *p,*q;
    p=userhead;
    while(p)
    {
        if(strcmp(p->username,recv->data.from)==0)
        {
            for(int j=0;j<p->numfriend;j++)
            {
                if(strcmp(recv->data.to,p->friends[j])==0)
                {
                    recv->type='6';
                    break;
                }
            }
        }
        if(strcmp(p->username,recv->data.to)==0)
        {
            flag=1;
            zhuangtai=p->status;
            recv->data.recvfd=p->conn_fd;
            break;
        }
        q=p;
        p=p->next;
    }
    if(flag==0)
    {
        recv->data.recvfd=recv->data.sendfd;
        strcpy(recv->data.from,"server");
        recv->type='1';
    }
    if(zhuangtai==ONLINE||recv->type=='1')
    {
        sendpack(recv);
    }
    if(zhuangtai==DOWNLINE)
    {
        leavepack[le++]=*recv;
    }
}

//删除好友
void delfriend(packet *recv)
{
    int flag=0;
    int locate;
    int j;
    struct userinfo *p,*q;
    p=userhead;
    while(p)
    {
        if(strcmp(p->username,recv->data.from)==0)
        {
            for(j=0;j<p->numfriend;j++)
            {
                if(strcmp(p->friends[j],recv->data.to)==0)
                {
                    locate=j;
                    break;
                }
            }
            for(j=locate;j<p->numfriend;j++)
            {
                memcpy(p->friends[j],(p->friends)[j+1],20);
            }
            p->numfriend-=1;
        }
        if(strcmp(p->username,recv->data.to)==0)
        {
            for(j=0;j<p->numfriend;j++)
            {
                if(strcmp(p->friends[j],recv->data.from)==0)
                {
                    locate=j;
                    break;
                }
            }
            for(j=locate;j<p->numfriend;j++)
            {
                memcpy(p->friends[j],p->friends[j+1],40);
            }
            p->numfriend-=1;
        }
        p=p->next;
    }
    recv->type='2';
    recv->data.recvfd=recv->data.sendfd;
    strcpy(recv->data.message,recv->data.to);
    strcpy(recv->data.to,recv->data.from);
    printf("用户%s和用户%s不再是好友!\n",recv->data.from,recv->data.message);
    sendpack(recv);
}

//查看好友
void seefriend(packet *recv)
{
    int i;
    struct userinfo *p;
    int num;
    p=userhead;
    while(p)
    {
        if((strcmp(p->username,recv->data.from))==0)
        {
            recv->data.recvfd=p->conn_fd;
            if(p->numfriend==0)
            {
                recv->type='4';
            }
            else
            {
                for(i=0;i<p->numfriend;i++)
                {
                    //printf("%s\n",p->friends[i]);
                    strcpy((recv->data).name[i],p->friends[i]);
                }
                num=p->numfriend;
            }
            break;
        }
        p=p->next;
    }
    recv->data.sendfd=num; //*****************
    strcpy(recv->data.to,recv->data.from);
    strcpy(recv->data.from,"server");
    sendpack(recv);
}

//私聊
void chat_one(packet *recv)
{
    int flag=0;
    int zhuangtai;
    struct userinfo *p,*q;
    chatrecord temp; 
    p=userhead;
    while(p)
    {
        if((strcmp(p->username,recv->data.to))==0)
        {
            flag=1;
            zhuangtai=p->status;
            recv->data.recvfd=p->conn_fd;
            printf("%d-------------------------------\n",recv->data.recvfd);
            break;
        }
        p=p->next;
    }
    if(flag==0)
    {
        recv->type='3';
        recv->data.recvfd=recv->data.sendfd;
    }
    if(flag==1)
    {
        strcpy(temp.from,recv->data.from);
        strcpy(temp.to,recv->data.to);
        strcpy(temp.message,recv->data.message);
        strcpy(temp.time,mytime());
        saverecord("siliao.txt",temp);
    }
    if(zhuangtai==ONLINE||recv->type=='3')
    {
        sendpack(recv);
    }
    if(zhuangtai==DOWNLINE)
    {
        leavepack[le++]=*recv;
    }
}

//查看私聊聊天记录
void see_onerecord(packet *recv)
{
    int onefd;
    int t;
    chatrecord temp;
    if((onefd=open("siliao.txt",O_RDONLY|O_CREAT,S_IRUSR|S_IWUSR))<0)
    {
        my_err("open",__LINE__);
        exit(1);
    }
    while((t=read(onefd,&temp,sizeof(chatrecord)))>0)
    {
        if((!(strcmp(temp.to,recv->data.to))&&!(strcmp(temp.from,recv->data.from)))||(!(strcmp(temp.to,recv->data.from))&&!(strcmp(temp.from,recv->data.to))))
        {
            strcpy(recv->data.from,temp.from);
            strcpy(recv->data.to,temp.to);
            strcpy(recv->data.message,temp.message);
            strcpy(recv->data.name[0],temp.time);
            recv->data.name[0][strlen(recv->data.name[0])-1]='\0';
            recv->data.recvfd=recv->data.sendfd;
            sendpack(recv);
        }
    }
    close(onefd);
}



//发送离线消息
void leavemes(packet *recv)
{
    struct userinfo *p,*q;
    int t;
    for(int j=0;j<le;j++)
    {
        if(strcmp(leavepack[j].data.to,recv->data.from)==0)
        {
            p=userhead;
            while(p)
            {
                if((strcmp(recv->data.from,p->username))==0)
                {
                    leavepack[j].data.recvfd=p->conn_fd;
                    //printf("%d---------=====================\n",leavepack[j].data.recvfd);
                    break;
                }
                p=p->next;
            }
            sendpack(&leavepack[j]);
        }
    }
    for(int j=0;j<le;j++)
    {
        if(strcmp(leavepack[j].data.to,recv->data.from)==0)
        {
            for(t=j;t<le;t++)
            {
                leavepack[t]=leavepack[t+1];
            }
            le--;
        }
    }
}

//创建群
void build_group(packet *recv)
{
    int flag=0;
    struct userinfo *tp;
    tp=userhead;
    struct groupinfo *p,*q,*np;
    p=grouphead;
    q=grouphead;
    while(p)
    {
        if((strcmp(p->groupname,recv->data.message))==0)
        {
            flag=1;
            memcpy(recv->data.message,"n\0",2);
        }
        q=p;
        p=p->next;
    }
    if(flag==0)
    {
        np=(struct groupinfo *)malloc(sizeof(struct groupinfo));
        memset(np,0,sizeof(struct groupinfo));
        memcpy(np->groupname,recv->data.message,20);
        strcpy(np->groupowner,recv->data.from);
        strcpy(np->member[0],recv->data.from);
        np->membernum++;
        if(grouphead==NULL)
        {
            np->next=NULL;
            grouphead=np;
        }
        else
        {
            np->next=NULL;
            q->next=np;
        }
        while(tp)
        {
            if(strcmp(tp->username,recv->data.from)==0)
            {
                tp->numgroup++;
                strcpy(tp->group[tp->numgroup-1],recv->data.message);
                break;
            }
            tp=tp->next;
        }
        memcpy(recv->data.message,"y\0",2);
    }
    strcpy(recv->data.to,recv->data.from);
    recv->data.recvfd=recv->data.sendfd;
    sendpack(recv);
}


//邀请好友进群
void invite_group(packet *recv)
{
    int zhuangtai=DOWNLINE;
    struct userinfo *p;
    p=userhead;
    if(judge_group(recv->data.message)==1)
    {
        if(judge_groupren(recv->data.message,recv->data.from)==1)
        {
            if(judge_groupren(recv->data.message,recv->data.to)==0)
            {
                p=userhead;
                while(p)
                {
                    if(strcmp(p->username,recv->data.to)==0)
                    {
                        recv->type='9';
                        zhuangtai=p->status;
                        recv->data.recvfd=p->conn_fd;
                    }
                    p=p->next;
                }
            }
            else
            {
                memcpy(recv->data.name[0],"a\0",2);
                strcpy(recv->data.to,recv->data.from);
                recv->data.recvfd=recv->data.sendfd;
            }
        }
        else
        {
            memcpy(recv->data.name[0],"b\0",2);
            strcpy(recv->data.to,recv->data.from);
            recv->data.recvfd=recv->data.sendfd;
        }
    }
    else
    {
        memcpy(recv->data.name[0],"c\0",2);
        strcpy(recv->data.to,recv->data.from);
        recv->data.recvfd=recv->data.sendfd;
    }
    if(zhuangtai==ONLINE||recv->data.name[0][0]=='a'||recv->data.name[0][0]=='b'||recv->data.name[0][0]=='c')
    {
        sendpack(recv);
    }
    else
    {
        leavepack[le++]=*recv;
    }
}


//处理群邀请
void deal_invitegroup(packet *recv)
{
    int zhuangtai;
    struct userinfo *p;
    struct groupinfo *q;
    p=userhead;
    q=grouphead;
    if(recv->data.name[0][0]=='y')
    {
        while(p)
        {
            if(strcmp(p->username,recv->data.from)==0)
            {
                p->numgroup++;
                strcpy(p->group[p->numgroup-1],recv->data.message);
            }
            if(strcmp(p->username,recv->data.to)==0)
            {
                zhuangtai=p->status;
                recv->data.recvfd=p->conn_fd;
            }
            p=p->next;
        }
        while(q)
        {
            if(strcmp(q->groupname,recv->data.message)==0)
            {
                q->membernum++;
                strcpy(q->member[q->membernum-1],recv->data.from);
                break;
            }
            q=q->next;
        }
        printf("用户%s被%s邀入群%s!\n",recv->data.from,recv->data.to,recv->data.message);
    }
    if(recv->data.name[0][0]=='n')
    {
        while(p)
        {
            if(strcmp(p->username,recv->data.to)==0)
            {
                zhuangtai=p->status;
                recv->data.recvfd=p->conn_fd;
            }
            p=p->next;
        }
    }
    if(zhuangtai==ONLINE)
    {
        sendpack(recv);
    }
    if(zhuangtai==DOWNLINE)
    {
        leavepack[le++]=*recv;
    }
}

//加群
void addgroup(packet *recv)
{
    int zhuangtai;
    int flag=0;
    struct userinfo *q;
    struct groupinfo *p;
    p=grouphead;
    q=userhead;
    while(p)
    {
        if(strcmp(p->groupname,recv->data.message)==0)
        {
            flag=1;
            if(judge_groupren(p->groupname,recv->data.from)==1)
            {
                memcpy(recv->data.name[0],"p\0",2);
                recv->data.recvfd=recv->data.sendfd;
                break;
            }
            else
            {
                strcpy(recv->data.to,p->groupowner);
                recv->type='7';
                while(q)
                {
                    if(strcmp(q->username,recv->data.to)==0)
                    {
                        zhuangtai=q->status;
                        recv->data.recvfd=q->conn_fd;
                        break;
                    }
                    q=q->next;
                }
            }
            break;
        }
        p=p->next;
    }
    if(flag==0)
    {
        memcpy(recv->data.name[0],"m\0",2);
        recv->data.recvfd=recv->data.sendfd;
    }
    if(zhuangtai==ONLINE||recv->data.name[0][0]=='p'||recv->data.name[0][0]=='m')
    {     
        sendpack(recv);
    }
    if(zhuangtai==DOWNLINE)
    {
        leavepack[le++]=*recv;
    }
}


//用户加群请求的处理
void deal_adgroup(packet *recv)
{
    int zhuangtai;
    struct userinfo *p;
    struct groupinfo *q;
    p=userhead;
    q=grouphead;
    if(recv->data.name[0][0]=='y')
    {
        while(p)
        {
            if(strcmp(p->username,recv->data.to)==0)
            {
                p->numgroup++;
                zhuangtai=p->status;
                strcpy(p->group[p->numgroup-1],recv->data.message);
                //printf("%s-------------+++++==\n",p->group[p->numgroup-1]);
                recv->data.recvfd=p->conn_fd;
            }
            p=p->next;
        }
        while(q)
        {
            if(strcmp(q->groupname,recv->data.message)==0)
            {
                q->membernum++;
                strcpy(q->member[q->membernum-1],recv->data.to);
            }
            q=q->next;
        }
        printf("用户%s已加入群%s!\n",recv->data.from,recv->data.message);
    }
    if(recv->data.name[0][0]=='n')
    {
        while(p)
        {
            if(strcmp(p->username,recv->data.to)==0)
            {
                zhuangtai=p->status;
                recv->data.recvfd=p->conn_fd;
            }
            p=p->next;
        }
    }
    if(zhuangtai==ONLINE)
    {
        sendpack(recv);
    }
    if(zhuangtai==DOWNLINE)
    {
        leavepack[le++]=*recv;
    }
}


//退群
void quitgroup(packet *recv)
{
    int flag=0;
    struct userinfo *p;
    p=userhead;
    int j,t,h;
    struct groupinfo *q;
    q=grouphead;
    while(p)
    {
        if(strcmp(p->username,recv->data.from)==0)
        {
            t=locateuser(p->username,recv->data.message);
            for(j=t;j<p->numgroup;j++)
            {
                strcpy(p->group[j],p->group[j+1]);
            }
            p->numgroup--;
            break;
        }
        p=p->next;
    }
    while(q)
    {
        if(strcmp(q->groupname,recv->data.message)==0)
        {
            h=locategroup(recv->data.message,recv->data.from);
            for(j=h;j<q->membernum;j++)
            {
                strcpy(q->member[j],q->member[j+1]);
            }
            q->membernum--;
            break;
        }
        q=q->next;
    }
    recv->data.recvfd=recv->data.sendfd;
    strcpy(recv->data.to,recv->data.from);
    printf("用户%s退出群%s\n",recv->data.from,recv->data.message);
    sendpack(recv);
}

//群聊
void chat_group(packet *recv)
{
    int j,t,flag=0;
    chatrecord temp;
    struct groupinfo *p;
    struct userinfo *q;
    p=grouphead;
    int zhuangtai;
    while(p)
    {
        if(strcmp(p->groupname,recv->data.name[0])==0)
        {
            /*for(j=0;j<p->membernum;j++)
            {
                printf("%s------------++\n",p->member[j]);
            }*/
            flag=1;
            for(j=0;j<p->membernum;j++)
            {
                if(strcmp(p->member[j],recv->data.from)!=0)
                {
                    q=userhead;
                    while(q)
                    {
                        if((strcmp(q->username,p->member[j])==0))
                        {
                            zhuangtai=q->status;
                            recv->data.recvfd=q->conn_fd;
                            strcpy(recv->data.to,p->member[j]);
                            break;
                        }
                        q=q->next;
                    }
                    if(zhuangtai==ONLINE)
                    {
                        sendpack(recv);
                    }
                    else
                    {
                        leavepack[le++]=*recv;
                    }
                }
            }
        }
        p=p->next;
    }
    if(flag==1)
    {
        strcpy(temp.from,recv->data.from);
        strcpy(temp.to,recv->data.name[0]);
        strcpy(temp.message,recv->data.message);
        strcpy(temp.time,mytime());
        saverecord("qunliao.txt",temp);
    }
    if(flag==0)
    {
        recv->type='m';
        recv->data.recvfd=recv->data.sendfd;
        strcpy(recv->data.to,recv->data.from);
        sendpack(recv);
    }
}

//查看群
void seegroup(packet *recv)
{
    int flag=0;
    struct userinfo *p;
    int j;
    p=userhead;
    while(p)
    {
        if(strcmp(p->username,recv->data.from)==0)
        {
            recv->data.size=p->numgroup;
            for(j=0;j<p->numgroup;j++)
            {
                //printf("----------------%d\n",p->numgroup);
                //printf("%s--------------------\n",p->group[j]);
                flag=1;
                strcpy(recv->data.name[j],p->group[j]);
                //printf("#$#$#$#$%s\n",recv->data.name[j]);
            }
            break;
        }
        p=p->next;
    }
    if(flag==0)
    {
        recv->type='5';
    }
    recv->data.recvfd=recv->data.sendfd;
    strcpy(recv->data.to,recv->data.from);
    sendpack(recv);
}

//请求发送文件消息的转发
void req_sendfile(packet *recv)
{
    struct userinfo *p;
    p=userhead;
    int zhuangtai;
    while(p)
    {
        if(strcmp(p->username,recv->data.to)==0)
        {
            zhuangtai=p->status;
            recv->data.recvfd=p->conn_fd;
        }
        p=p->next;
    }
    if(zhuangtai==ONLINE)
    {
        sendpack(recv);
    }
    else
    {
        leavepack[le++]=*recv;
    }
}


//处理是否同意发送文件
void deal_reqsendfile(packet *recv)
{
    struct userinfo *p;
    p=userhead;
    int zhuangtai;
    while(p)
    {
        if(strcmp(p->username,recv->data.to)==0)
        {
            recv->data.recvfd=p->conn_fd;
            zhuangtai=p->status;
        }
        p=p->next;
    }
    if(zhuangtai==ONLINE)
    {
        sendpack(recv);
    }
    else
    {
        leavepack[le++]=*recv;
    }
}


//发送文件
void send_file(packet *recv)
{
    int zhuangtai;
    int flag=0;
    struct userinfo *p;
    p=userhead;
    while(p)
    {
        if(strcmp(p->username,recv->data.to)==0)
        {
            flag=1;
            zhuangtai=p->status;
            recv->data.recvfd=p->conn_fd;
            break;
        }
        p=p->next;
    }
    if(flag==0)
    {
        recv->type='s';
        recv->data.recvfd=recv->data.sendfd;
    }
    if(zhuangtai==ONLINE||recv->type=='s')
    {
        sendpack(recv);
    }
    else
    {
        leavepack[le++]=*recv;
    }
}

//查看群聊记录
void see_grouprecord(packet *recv)
{
    int manyfd;
    int t;
    chatrecord temp;
    if((manyfd=open("qunliao.txt",O_RDONLY|O_CREAT,S_IRUSR|S_IWUSR))<0)
    {
        my_err("open",__LINE__);
        exit(1);
    }
    while((t=read(manyfd,&temp,sizeof(chatrecord)))>0)
    {
        if(!(strcmp(temp.to,recv->data.to)))
        {
            strcpy(recv->data.from,temp.from);
            strcpy(recv->data.to,temp.to);
            strcpy(recv->data.message,temp.message);
            strcpy(recv->data.name[0],temp.time);
            recv->data.name[0][strlen(recv->data.name[0])-1]='\0';
            recv->data.recvfd=recv->data.sendfd;
            sendpack(recv);
        }
    }
    close(manyfd);
}


//解散群
void release_group(packet *recv)
{
    int flag=0;
    int j,k,t,biao=0;;
    struct userinfo *p,*np;
    struct groupinfo *q,*nq;
    p=userhead;
    q=grouphead;
    nq=grouphead;
    while(q)
    {
        if(strcmp(q->groupname,recv->data.message)==0)
        {
            if(strcmp(q->groupowner,recv->data.from)==0)
            {
                for(j=0;j<q->membernum;j++)
                {
                    p=userhead;
                    while(p)
                    {
                        if(strcmp(q->member[j],p->username)==0)
                        {
                            t=locateuser(recv->data.message,q->member[j]);
                            for(k=t;k<p->numgroup;k++)
                            {
                                strcpy(p->group[k],p->group[k+1]);
                            }
                            p->numgroup--;
                            break;
                        }
                        p=p->next;
                    }
                }
                biao=1;
                if(flag==0)
                {
                    grouphead=q->next;
                    free(q);
                }
                else
                {
                    nq->next=q->next;
                    free(q);
                }
            }
        }
        nq=q;
        q=q->next;
        flag++;
    }
    if(biao==0)
    {
        recv->type=='u';
    }
    recv->data.recvfd=recv->data.sendfd;
    strcpy(recv->data.to,recv->data.from);
    sendpack(recv);
    printf("\n群主%s解散了群%s\n",recv->data.to,recv->data.message);
}


/*//连接MYSQL。
void connect_mysql()
{
    MYSQL   mysql;
    MYSQL_RES  *RES=NULL;
    MYSQL_ROW  row;
    char *query_str=NULL;
    int rc,i,fields;
    int rows;
    if(NULL==mysql_init(&mysql))
    {
        printf("mysql_init(): %s\n",mysql_error(&mysql));
        exit(1);
    }
    if(NULL==mysql_real_connect(&mysql,
                "localhost",
                "ltt",
                "787604",
                "chatroom",
                0,
                NULL,
                0)){
        printf("mysql_real_connect() :%s\n",mysql_error(&mysql));
        exit(1);
    }
    printf("connected MYSQL successful!\n");
}
*/

//注册
void sign_up(packet *recv)
{
    int flag=0;
    struct userinfo *p,*q,*np;
    q=userhead;
    p=userhead;
    while(p)
    {
        if((strcmp(p->username,recv->data.from))==0)
        {
            flag=1;
            memcpy(recv->data.message,"n\0",2);
        }
        q=p;
        p=p->next;
    }
    if(flag==0)
    {
        np=(struct userinfo *)malloc(sizeof(struct userinfo));
        memset(np,0,sizeof(struct userinfo));
        strcpy(np->username,recv->data.from);
        strcpy(np->password,recv->data.message);
        np->status=DOWNLINE;
        if(userhead==NULL)
        {
            np->next=NULL;
            userhead=np;
        }
        else
        {
            np->next=NULL;
            q->next=np;
        }
        memcpy(recv->data.message,"y\0",2);
    }
    strcpy(recv->data.to,recv->data.from);
    strcpy(recv->data.from,"server");
    recv->data.recvfd=recv->data.sendfd;
    recv->data.sendfd=sock_fd;
    sendpack(recv);
}


//登录
void sign_in(packet *recv)
{
    int flag=0;
    int num=0;
    struct userinfo *p,*q;
    p=userhead;
    while(p!=NULL)
    {
        if(strcmp(p->username,recv->data.from)==0)
        {
            if(strcmp(p->password,recv->data.message)==0)
            {
                if(p->status==ONLINE)
                {
                    flag=1;
                    recv->type='5';
                    break;
                }
                else
                {
                    flag=1;
                    p->status=ONLINE;
                    p->conn_fd=recv->data.sendfd;
                    for(int j=0;j<le;j++)
                    {
                        if((strcmp(leavepack[j].data.to,recv->data.from))==0)
                        {
                            num++;
                        }
                    }
                    memcpy(recv->data.message,"y\0",2);
                    printf("user:%s login\n",recv->data.from);
                    break;
                }
            }
            else
            {
                memcpy(recv->data.message,"n\0",2);
            }
        }
        p=p->next;
    }
    if(flag==0)
    {
        memcpy(recv->data.message,"n\0",2);
    }
    strcpy(recv->data.to,recv->data.from);
    strcpy(recv->data.from,"server");
    recv->data.recvfd=recv->data.sendfd;
    recv->data.sendfd=num;   //把离线消息数量放进去，无奈的办法！
    sendpack(recv);
}




//服务器关闭
void stop(int i)
{
    writefile();
    printf("server close!\n");
    exit(1);
}

//处理接收到的包,每出现一个包，来个线程。
void *thread(void *arg)
{
    int i;
    packet *recv_pack=(packet *)arg;
    
    printf("recvfd:%d\n",recv_pack->data.recvfd);
    printf("sendfd:%d\n",recv_pack->data.sendfd);
    printf("recvname:%s\n",recv_pack->data.to);
    printf("sendname:%s\n",recv_pack->data.from);
    printf("message:%s\n",recv_pack->data.message);
    printf("type:%c\n", recv_pack->type);

    switch(recv_pack->type)
    {
        case 'l':
            {
                sign_in(recv_pack);
                break;
            }
        case 'r':
            {
                sign_up(recv_pack);
                break;
            }
        case 'a':
            {
                addfriend(recv_pack);
                break;
            }
        case 'b':
            {
                delfriend(recv_pack);
                break;
            }
        case 'c':
            {
                deal_adfriend(recv_pack);
                break;
            }
        case 'd':
            {
                chat_one(recv_pack);
                break;
            }
        case 'o':
            {
                see_onerecord(recv_pack);
                break;
            }
        case 'e':
            {
                seefriend(recv_pack);
                break;
            }
        case 'g':
            {
                leavemes(recv_pack);
                break;
            }
        case 'h':
            {
                build_group(recv_pack);
                break;
            }
        case 'i':
            {
                invite_group(recv_pack);
                break;
            }
        case 'j':
            {
                addgroup(recv_pack);
                break;
            }
        case '8':
            {
                deal_adgroup(recv_pack);
                break;
            }
        case '0':
            {
                deal_invitegroup(recv_pack);
                break;
            }
        case 'k':
            {
                quitgroup(recv_pack);
                break;
            }
        case't':
            {
                see_grouprecord(recv_pack);
                break;
            }
        case 'n':
            {
                chat_group(recv_pack);
                break;
            }
        case 'z':
            {
                seegroup(recv_pack);
                break;
            }
        case 'p':
            {
                release_group(recv_pack);
                break;
            }
        case 'v':
            {
                send_file(recv_pack);
                break;
            }
        case 'x':
            {
                req_sendfile(recv_pack);
                break;
            }
        case 'w':
            {
                deal_reqsendfile(recv_pack);
                break;
            }
        case 'q':
            {
                break;
            }
    }
}

int main()
{
    int i,t;
    int conn_fd;
    int epfd;
    int fdnum;
    int ret;
    int optval;
    packet recv_buf;
    packet *recv_pack;
    int name_num;
    pthread_t tid,stid;
    socklen_t cli_len;
    struct sockaddr_in serv_addr,cli_addr;

    readfile();
    //声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
    struct epoll_event ev,events[LISTENO];

    pthread_mutex_init(&mutex,NULL);

    //生成用于处理accept的epoll专用的文件描述符
    epfd=epoll_create(MAX_EPOLL);
    signal(SIGTSTP,stop);
    //创建tcp套接字
    sock_fd=socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd<0){
        my_err("socket",__LINE__);
        exit(1);
    }
    optval=1;
    if(setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,(void *)&optval,sizeof(int))<0){
        my_err("setsockopt",__LINE__);
    }
    ev.data.fd=sock_fd;
    ev.events=EPOLLIN;//设置要处理的事件类型为文件描述符可以读
    epoll_ctl(epfd,EPOLL_CTL_ADD,sock_fd,&ev);//注册epoll事件

    memset(&serv_addr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(SERV_PORT);
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

    if(bind(sock_fd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in))<0)
    {
        my_err("bind fail!",__LINE__);
    }
    printf("bind success!\n");
    
    if(listen(sock_fd,LISTENO)<0)
    {
        my_err("listen fail",__LINE__);
    }
    printf("listen success!\n");

    cli_len=sizeof(struct sockaddr_in);

    while(1)
    {
        fdnum=epoll_wait(epfd,events,MAX_EPOLL,500);
        for(int i=0;i<fdnum;i++)
        {
            //处理所发生的所有事件
            if(events[i].data.fd==sock_fd)//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            {
                conn_fd=accept(sock_fd,(struct sockaddr *)&cli_addr,&cli_len);
               // printf("####%d\n",conn_fd);
                if(conn_fd<0)
                {
                    my_err("accept",__LINE__);
                }
                printf("a new connect: %s\n",inet_ntoa(cli_addr.sin_addr));
                
                 //设置用于读操作的文件描述符
                ev.data.fd=conn_fd;
                //设置用于注测的读操作事件
                ev.events=EPOLLIN;
                //注册ev
                epoll_ctl(epfd,EPOLL_CTL_ADD,conn_fd,&ev);
            }
            else if(events[i].events&EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入。
            {
                t=recv(events[i].data.fd,&recv_buf,sizeof(packet),0);
                recv_buf.data.sendfd=events[i].data.fd;
                //printf("4$$%d\n",recv_buf.data.sendfd);
                if(t<0)
                {
                    close(events[i].data.fd);
                    my_err("recv",__LINE__);
                    continue;
                }
                else if(t==0)
                {
                    struct userinfo *p;
                    p=userhead;
                    while(p)
                    {
                        if(events[i].data.fd==p->conn_fd)
                        {
                            printf("user:%s downline\n",p->username);
                            p->status=DOWNLINE;
                            break;
                        }
                        p=p->next;
                    }
                    ev.data.fd=events[i].data.fd;
                    epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&ev);
                    close(events[i].data.fd);
                    continue;
                }
                pthread_create(&tid,NULL,(void *)thread,(void *)&recv_buf);
            }
        }
    }
    return 0;
}
