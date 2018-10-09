#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<mysql/mysql.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<time.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<pthread.h>
#include<stdio.h>
#include<termios.h>
#include<assert.h>
#include<stdlib.h>
#include<errno.h>



#define MAX_USER   100
#define MAX_GROUP  50
#define max_epoll  1000

#define SERV_PORT     4507  //服务端的端口
#define LISTENO       500    //连接请求队列的最大长度
#define INVALID_USERINFO   'n' //用户信息无效
#define VALLD_USERINFO     'y' //用户信息有效

#define CHARSIZE       1024

#define ONLINE         1
#define DOWNLINE       0

pthread_mutex_t mutex;
int lian=0;
int conn_fd;
char myname[20];
struct userinfo              //保存用户信息结构体
{
    char username[20];
    char password[20];
    int status;
    int conn_fd;
    char friends[50][20];  //好友信息i
    int numfriend;          //好友数量
    char group[20][20];    //群信息
    int numgroup;           //群数量
    struct userinfo *next;
};

struct groupinfo
{
    char groupname[20];      //群名
    char groupowner[20];    //群主
    char member[50][20];  //群成员信息
    int membernum;          //成员数量
    struct groupinfo *next;
};


typedef struct data
{
    int sendfd;
    int recvfd;
    int size;          //发文件时保存文件大小,禁言时用于保存禁言时间
    int nowsize;
    char from[20];     //发送方
    char to[20];       //接收方
    char name[50][20]; //当发送文件时,name[0]保存文件名.
    char message[CHARSIZE];
}datas;

typedef struct pack
{
    char type;
    datas data;
}packet;

packet pack[50];  //接收到的要处理的消息的保存处
int ke=0;
packet box[300];
int qun=0;
int kai=0;
int jie=0;
int shou=0;

//自定义的错误处理函数
void my_err(const char *err_string,int line)
{
    fprintf(stderr,"line:%d ",line);
    perror(err_string);
    exit(1);
}

//获取用户输入存入到buf,buf的长度为len,用户输入数据以'\n'为结束标志
int get_input(char *buf,int len)
{
    int i;
    char c;
    if(buf == NULL)
    {
        return -1;
    }
    i = 0;
    while(((c = getchar()) != '\n') && (c != EOF) && (i < len-2))
    {
        buf[i++] = c;
    }
    buf[i++] = '\0';
    return 0;
}


int getch()
{
	int c=0;
	struct termios org_opts, new_opts;
	int res=0;
	//-----  store old settings -----------
	res=tcgetattr(STDIN_FILENO, &org_opts);
	assert(res==0);
	//---- set new terminal parms --------
	memcpy(&new_opts, &org_opts, sizeof(new_opts));
	new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
	c=getchar();
	//------  restore old settings ---------
	res=tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
	assert(res==0);
	return c;
}
//输入密码
void input_mima(char *pd)
{
    int i;
    //char *pd = (char *)malloc(sizeof(char) * 128);
    for(i = 0; ; i++)
    {
        pd[i] = getch();
        if(pd[i] == '\n')
	    {
            pd[i] = '\0';
            break;
	    }
        if(pd[i] == 127) //删除
	    {
            printf("\b \b");
            i = i - 2;
        }
	    else
	    {
            printf(".");
	    }
        if(i<0)
	    {
            pd[0]='\0';
        }
    }
}


//添加好友
void addfriend()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='a';
    printf("^~~~~~~~~~~~~~~~~添加好友~~~~~~~~~~~~~~~~~\n");
    while(1)
    {
        printf("请输入您要添加的用户的名称:");
        get_input(sendpack.data.to,40);
        if(strcmp(sendpack.data.to,myname)==0)
        {
            printf("对不起，您不能添加自己!\n");
        }
        else
        {
            break;
        }
    }
    strcpy(sendpack.data.from,myname);
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }
}


//处理添加好友的请求
void deal_adfriend()
{
    int j,l;
    int i,t;
    char b,d;
    int n;
    int a[50];
    packet temppack;
    packet sendpack;
    while(1)
    {
        l=1;
        j=0;
        memset(&sendpack,0,sizeof(packet));
        sendpack.type='c';
        system("clear");
        printf("\n^~~~~~~~~~~~~~~~~好友添加请求~~~~~~~~~~~~~~~~~^\n\n");
        printf("q:返回上级菜单~~~~~~~~~~~~~~~~~~~~~~~~~c:继续处理\n\n");
        for(i=0;i<50;i++)
        {
            if(pack[i].type=='a')
            {
                printf("\n(%d):来自用户%s!\n",l++,pack[i].data.from);
                a[j++]=i;
            }
        }
        printf("\n请输入你要处理的序号:");
        while(1)
        {
            scanf("%d",&n);
            if(n<0||n>50)
            {
                printf("error!\n");
            }
            else
            {
                break;
            }
        }
        temppack=pack[a[n-1]];
        t=n-1;
        printf("\n用户%s请求添加您为好友,您是否同意.\n",temppack.data.from);
        printf("y:同意                    n:不同意\n");
        printf("请输入你的选择:");
        getchar();
        scanf("%c",&b);
        if(b=='y')
        {
            memcpy(sendpack.data.message,"y\0",2);
        }
        else
        {
            memcpy(sendpack.data.message,"n\0",2);
        }
        strcpy(sendpack.data.to,temppack.data.from);
        strcpy(sendpack.data.from,myname);
        //把已处理的消息覆盖
        for(i=t;i<50;i++)
        {
            pack[i]=pack[i+1];
        }
        ke--;
        if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
        {
            my_err("send",__LINE__);
            exit(1);
        }
        printf("返回or继续处理:");
        getchar();
        scanf("%c",&d);
        if(d=='q')
        {
            break;
        }
        if(d=='c')
        {
            continue;
        }
    }
}
   
//删除好友
void delfriend()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='b';
    strcpy(sendpack.data.from,myname);
    printf("请输入你要删除的好友的昵称:");
    scanf("%s",sendpack.data.to);
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }
}

//查看好友
void seefriend()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(sendpack));
    sendpack.type='e';
    strcpy(sendpack.data.from,myname);
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }
}

//建群
void build_group()
{
    int t;
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='h';
    strcpy(sendpack.data.from,myname);
    while(1)
    {
        printf("请输入您要建立的群名[<20]:");
        scanf("%s",sendpack.data.message);
        t=strlen(sendpack.data.message);
        if(t<0||t>20)
        {
            printf("\n群名输入不规范!请重新输入.\n");
        }
        else
        {
            break;
        }
    }
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }
}


//私聊
void chat_one()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(sendpack));
    sendpack.type='d';
    strcpy(sendpack.data.from,myname);
    while(1)
    {
        printf("请输入你想要私聊的好友:");
        scanf("%s",sendpack.data.to);
        getchar();
        if(strcmp(sendpack.data.to,sendpack.data.from)==0)
        {
            printf("\n对不起,您不能和自己进行私聊,请重新输入\n");
        }
        else
            break;
    }
    system("clear");
    printf("^私~~~~~~~~~~~~~~~~%s~~~~~~~~~~~~~~~~~~~聊^\n",sendpack.data.to);
    printf("exit:退出私聊!\n");
    while(1)
    {
        get_input(sendpack.data.message,100);
        if(strcmp(sendpack.data.message,"exit")==0)
        {
            break;
        }
        if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
        {
            my_err("send",__LINE__);
            exit(1);
        }
    }
}


//请求查看离线消息
void leavemes()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='g';
    strcpy(sendpack.data.from,myname);
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }
}


//邀请好友进群
void invite_group()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='i';
    strcpy(sendpack.data.from,myname);
    printf("请输入您准备邀请好友进的群:");
    scanf("%s",sendpack.data.message);
    printf("\n请输入您要邀进群的好友:");
    scanf("%s",sendpack.data.to);
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }
}

//处理群邀请
void deal_invitegroup()
{
    int j,l;
    int i,t;
    char b,d;
    int n;
    int a[50];
    packet temppack;
    packet sendpack;
    while(1)
    {
        l=1;
        j=0;
        memset(&sendpack,0,sizeof(packet));
        sendpack.type='0';
        system("clear");
        printf("\n^~~~~~~~~~~~~~~~~群邀请消息~~~~~~~~~~~~~~~~~^\n\n");
        printf("q:返回上级菜单~~~~~~~~~~~~~~~~~~~~~~~~~c:继续处理\n\n");
        for(i=0;i<50;i++)
        {
            if(pack[i].type=='9')
            {
                printf("\n(%d):来自用户%s!\n",l++,pack[i].data.from);
                a[j++]=i;
            }
        }
        printf("\n请输入你要处理的序号:");
        scanf("%d",&n);
        temppack=pack[a[n-1]];
        t=n-1;
        printf("\n用户%s邀请您加入群%s,您是否同意.\n",temppack.data.from,temppack.data.message);
        printf("y:同意                    n:不同意\n");
        printf("\n请选择:");
        getchar();
        scanf("%c",&b);
        if(b=='y')
        {
            memcpy(sendpack.data.name[0],"y\0",2);
        }
        else
        {
            memcpy(sendpack.data.name[0],"n\0",2);
        }
        strcpy(sendpack.data.message,temppack.data.message);
        strcpy(sendpack.data.to,temppack.data.from);
        strcpy(sendpack.data.from,myname);
        //把已处理的消息覆盖
        for(i=t;i<50;i++)
        {
            pack[i]=pack[i+1];
        }
        ke--;
        if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
        {
            my_err("send",__LINE__);
            exit(1);
        }
        printf("\n返回or继续处理:");
        getchar();
        scanf("%c",&d);
        if(d=='q')
        {
            break;
        }
        if(d=='c')
        {
            continue;
        }
    }
}


//加群
void addgroup()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='j';
    strcpy(sendpack.data.from,myname);
    printf("请输入你想要加入的群:");
    scanf("%s",sendpack.data.message);
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }

}


//处理加群申请
void deal_adgroup()
{
    int j,l;
    int i,t;
    char b,d;
    int n;
    int a[50];
    packet temppack;
    packet sendpack;
    while(1)
    {
        l=1;
        j=0;
        memset(&sendpack,0,sizeof(packet));
        sendpack.type='8';
        system("clear");
        printf("\n^~~~~~~~~~~~~~~~~请求加群消息~~~~~~~~~~~~~~~~~^\n\n");
        printf("q:返回上级菜单~~~~~~~~~~~~~~~~~~~~~~~~~c:继续处理\n\n");
        for(i=0;i<50;i++)
        {
            if(pack[i].type=='7')
            {
                printf("\n(%d):来自用户%s!\n",l++,pack[i].data.from);
                a[j++]=i;
            }
        }
        printf("\n请输入你要处理的序号:");
        scanf("%d",&n);
        temppack=pack[a[n-1]];
        t=n-1;
        printf("\n用户%s请求加入您的群%s,您是否同意.\n",temppack.data.from,temppack.data.message);
        printf("y:同意                    n:不同意\n");
        printf("\n请选择:");
        getchar();
        scanf("%c",&b);
        if(b=='y')
        {
            memcpy(sendpack.data.name[0],"y\0",2);
        }
        else
        {
            memcpy(sendpack.data.name[0],"n\0",2);
        }
        strcpy(sendpack.data.message,temppack.data.message);
        strcpy(sendpack.data.to,temppack.data.from);
        strcpy(sendpack.data.from,myname);
        //把已处理的消息覆盖
        for(i=t;i<50;i++)
        {
            pack[i]=pack[i+1];
        }
        ke--;
        if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
        {
            my_err("send",__LINE__);
            exit(1);
        }
        printf("\n返回or继续处理:");
        getchar();
        scanf("%c",&d);
        if(d=='q')
        {
            break;
        }
        if(d=='c')
        {
            continue;
        }
    }
}

//退群
void quitgroup()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='k';
    strcpy(sendpack.data.from,myname);
    printf("\n请输入你想要退掉的群的群名:");
    scanf("%s",sendpack.data.message);
    if(send(conn_fd,&sendpack,sizeof(sendpack),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }
}

//群聊
void chat_group()
{
    kai=1;
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='n';
    strcpy(sendpack.data.from,myname);
    printf("\n请输入你要群聊的的群:");
    scanf("%s",sendpack.data.name[0]);

    system("clear");
    printf("^群~~~~~~~~~~~~~~~~%s~~~~~~~~~~~~~~~~~~~聊^\n",sendpack.data.name[0]);
    printf("exit:退出群聊!\n");
    while(1)
    {
        scanf("%s",sendpack.data.message);
        if(strcmp(sendpack.data.message,"exit")==0)
        {
            kai=0;
            break;
        }
        if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
        {
            my_err("send",__LINE__);
            exit(1);
        }
    }
}


//从群中移除某人
void remove_one()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='f';
    strcpy(sendpack.data.from,myname);
    printf("\n请输入你要处理的群:\n");
    scanf("%s",sendpack.data.message);
    printf("\n请输入你要移除的群员的名称:");
    scanf("%s",sendpack.data.to);
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }
}


//查看群消息
void see_groupmes()
{
    int j;
    system("clear");
    printf("\n~~~~~~~~~~~~~~~~~~~未看群消息~~~~~~~~~~~~~~~~~~\n");
    for(j=0;j<qun;j++)
    {
        printf("%s->say:%s\n",box[j].data.from,box[j].data.message);
    }
    kai=0;
    qun=0;
}

//请求发送文件
void req_sendfile()
{
    packet sendpack;
    int size;
    shou=0;
    struct stat buf;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='x';
    strcpy(sendpack.data.from,myname);
    printf("\n请输入你要发给的对象:");
    scanf("%s",sendpack.data.to);
    printf("\n请输入你要发送给对方的文件名:");
    scanf("%s",sendpack.data.message);
    if(lstat(sendpack.data.message,&buf)==-1)
    {
        //my_err("lstat",__LINE__);
        printf("\n输入的文件不存在\n");
        return;
    }
    sendpack.data.size=buf.st_size;
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }
}

//发送文件
void send_file()
{
    packet sendpack;
    int filefd;
    int ret,size;
    struct stat buf;
    shou=0;
    memset(&sendpack,0,sizeof(packet));
    char write_buf[100];
    char read_buf[100];
    sendpack.type='v';
    strcpy(sendpack.data.from,myname);
    printf("\n请输入你要发给的对象:");
    scanf("%s",sendpack.data.to);
    printf("\n请输入你要发送的文件:");
    scanf("%s",sendpack.data.name[0]);
    if(lstat(sendpack.data.name[0],&buf)==-1)
    {
        printf("\n输入的文件不存在\n");
        return;
    }

    size=buf.st_size;
    if((filefd=open(sendpack.data.name[0],O_RDONLY|O_CREAT,S_IRWXU))==-1)
    {
        my_err("open",__LINE__);
        exit(1);
    }
    do
    {
        memset(sendpack.data.message,0,sizeof(sendpack.data.message));
        if(size>=500)
        {
            if(( ret=read(filefd,sendpack.data.message,500))<0)
            {
                printf("此文件不存在!\n");
            }
            //sendpack.data.nowsize=size-500;

        }
        else
        {
            if((ret=read(filefd,sendpack.data.message,size))<0)
            { 
                printf("此文件不存在!\n");;
            }
            //sendpack.data.nowsize=size;
        }
        sendpack.data.nowsize=size;
        if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
        {
            my_err("send",__LINE__);
            exit(1);
        }
        size-=500;
        usleep(200000);
    }while(size>0&&shou==0);
}

//处理发送文件请求
void deal_reqsendfile()
{
    int j,l;
    int i,t;
    char b,d;
    int n;
    int a[50];
    packet temppack;
    packet sendpack;
    while(1)
    {
        l=1;
        j=0;
        memset(&sendpack,0,sizeof(packet));
        sendpack.type='w';
        system("clear");
        printf("\n^~~~~~~~~~~~~~~~~请求发送文件消息~~~~~~~~~~~~~~~~~^\n\n");
        printf("q:返回上级菜单~~~~~~~~~~~~~~~~~~~~~~~~~c:继续处理\n\n");
        for(i=0;i<50;i++)
        {
            if(pack[i].type=='x')
            {
                printf("(%d):来自用户%s!",l++,pack[i].data.from);
                a[j++]=i;
            }
        }
        printf("\n请输入你要处理的序号:");
        scanf("%d",&n);
        temppack=pack[a[n-1]];
        t=n-1;
        printf("\n用户%s向您发送文件%s,您是否同意.\n",temppack.data.from,temppack.data.message);
        printf("y:同意                    n:不同意\n");
        printf("\n请选择:");
        getchar();
        scanf("%c",&b);
        if(b=='y')
        {
            memcpy(sendpack.data.name[0],"y\0",2);
        }
        else
        {
            memcpy(sendpack.data.name[0],"n\0",2);
        }
        strcpy(sendpack.data.message,temppack.data.message);
        strcpy(sendpack.data.to,temppack.data.from);
        strcpy(sendpack.data.from,myname);
        //把已处理的消息覆盖
        for(i=t;i<50;i++)
        {
            pack[i]=pack[i+1];
        }
        ke--;
        if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
        {
            my_err("send",__LINE__);
            exit(1);
        }
        printf("\n返回or继续处理:");
        getchar();
        scanf("%c",&d);
        if(d=='q')
        {
            break;
        }
        if(d=='c')
        {
            continue;
        }
    }
}


//接收文件
void acceptfile(packet file)
{
    int fd;
    struct stat buf;
    if((fd=open(file.data.name[0],O_RDWR|O_CREAT|O_APPEND,S_IRWXU))==-1)
    {
        my_err("open",__LINE__);
        exit(1);
    }
    if(file.data.nowsize>500)
    {
        if(write(fd,file.data.message,500)!=500)
        {
            my_err("write",__LINE__);
            exit(1);
        }
    }
    else
    {
        if(write(fd,file.data.message,file.data.nowsize)!=file.data.nowsize)
        {
            my_err("write",__LINE__);
            exit(1);
        }
        file.data.nowsize=0;
    }
    if(file.data.nowsize<=0)
    {
        printf("\n文件接收完毕!\n");
    }
}

//查看群
void seegroup()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='z';
    strcpy(sendpack.data.from,myname);
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }
}

//解散群
void release_group()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='p';
    strcpy(sendpack.data.from,myname);
    printf("\n请输入你要解散的自己的群:");
    scanf("%s",sendpack.data.message);
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }

}

//查看私聊聊天记录
void see_onerecord()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='o';
    strcpy(sendpack.data.from,myname);
    printf("\n请输入你要查看查看私聊记录的好友名字:");
    scanf("%s",sendpack.data.to);
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }
}

//查看群聊聊天记录
void see_grouprecord()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='t';
    strcpy(sendpack.data.from,myname);
    printf("\n请输入你想要查看群聊记录的群:");
    scanf("%s",sendpack.data.to);
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);

    }
}

//禁言某人
void jinyan()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    sendpack.type='m';
    printf("\n请输入你要实施禁言的群\n");
    scanf("%s",sendpack.data.message);
    strcpy(sendpack.data.from,myname);
    printf("\n请输入你要禁言的群员名称:");
    scanf("%s",sendpack.data.to);
    printf("\n您打算禁言他多长时间:");
    scanf("%d",&sendpack.data.size);
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
    }
}

//登录
int sign_in()
{
    int re=0;
    packet sendpack;
    packet recvpack;
    memset(myname,0,sizeof(myname));
    memset(&sendpack,0,sizeof(packet));
    printf("^~~~~~~~~~~~~~~~~登录~~~~~~~~~~~~~~^\n");
    while(1)
    {
        sendpack.type='l';
        printf("用户名:");
        scanf("%s",sendpack.data.from);
        printf("密码:");
        getchar();
        input_mima(sendpack.data.message);
        if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
        {
            my_err("send",__LINE__);
            exit(1);
        }
        if(recv(conn_fd,&recvpack,sizeof(packet),0)<0)
        {
            my_err("recv",__LINE__);
            exit(1);
        }
        if(recvpack.type=='5')
        {
            printf("\n您已登录，请不要重复登录!\n");
        }
        if(recvpack.data.message[0]=='n')
        {
            printf("\n您的用户名或密码输入有误，请仔细核查后重新输入!\n");
        }
        if(recvpack.data.message[0]=='y')
        {
            re=1;
            strcpy(myname,sendpack.data.from);
            printf("\n登陆成功 ^~^\n");
            if(recvpack.data.sendfd==0)
            {
                printf("\n您没有离线消息!");
            }
            else
            {
                printf("\n您有离线消息%d条,请前往离线消息查看!",recvpack.data.sendfd);
            }
            break;
        }
    }
    return re;
}

//注册
void sign_up()
{
    packet recvpack;
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    printf("^~~~~~~~~~~~~~~注册~~~~~~~~~~~~~~~^\n");
    while(1)
    {
        sendpack.type='r';
        printf("\n用户名:");
        scanf("%s",sendpack.data.from);
        if((sizeof(sendpack.data.from))==0)
        {
            printf("用户名不能为空，请重新输入:");
            get_input(sendpack.data.message,20);
        }
        printf("\n密码:");
        getchar();
        input_mima(sendpack.data.message);
        printf("\n");
        strcpy(sendpack.data.to,"server");
        if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
        {
            my_err("send",__LINE__);
            exit(1);
        }
        if(recv(conn_fd,&recvpack,sizeof(packet),0)<0)
        {
            my_err("recv",__LINE__);
            exit(1);
        }
        if(recvpack.data.message[0]=='n')
        {
            printf("用户名已被使用!");
        }
        if(recvpack.data.message[0]=='y')
        {
            printf("\n您已注册成功!\n");
            break;
        }
    }
}

//通知消息管理
void noticemes_manage()
{
    int number;
    do
    {
        system("clear");
        printf("\n");
        printf("\t\t1:好友请求添加消息\n");
        printf("\t\t2:请求加群消息\n");
        printf("\t\t3:群邀请消息\n");
        printf("\t\t4:请求发送文件\n");
        printf("\t\t5:查看未收群消息\n");
        printf("\t\t6:返回\n");
        setbuf(stdin,NULL);
        printf("请选择你想要查看的消息序号:");
        scanf("%d",&number);
        switch(number)
        {
            case 1:
               {
                   deal_adfriend();
                   break;
               } 
            case 2:
               {
                   deal_adgroup();
                   break;
               }
            case 3:
               {
                   deal_invitegroup();
                   break;
               }
            case 4:
               {
                   deal_reqsendfile();
                   break;
               }
            case 5:
               {
                   see_groupmes();
                   break;
               }
        }
    }while(number!=6);
}




//主菜单
void main_menu()
{
    char choice;
    do
    {
        //system("clear");
        printf("\n\n\n^****************^~^~%s~^~^***********************^\n",myname);
        printf("\t\ta:添加好友\n");
        printf("\t\tb:删除好友\n");
        printf("\t\td:找人私聊\n");
        printf("\t\to:查看私聊聊天记录\n");
        printf("\t\te:查看好友\n");
        printf("\t\th:我要建群\n");
        printf("\t\ti:邀请好友进群\n");
        printf("\t\tr:我要踢人\n");
        printf("\t\tm:我要禁言\n");
        printf("\t\tj:我要加群\n");
        printf("\t\tk:我要退群\n");
        printf("\t\tz:查看我加的群\n");
        printf("\t\tl:我要群聊\n");
        printf("\t\tt:查看群聊聊天记录\n");
        printf("\t\tp:我要解散群\n");
        printf("\t\tx:请求发送文件\n");
        printf("\t\tv:发送文件\n");
        printf("\t\tf:消息管理\n");
        printf("\t\tg:离线消息\n");
        printf("\t\tq:退出\n");
        setbuf(stdin,NULL);
        printf("请输入你的选择:");
        scanf("%c",&choice);
        getchar();
        switch(choice)
        {
            case 'a':
                {
                    addfriend();
                    break;
                }
            case 'b':
                {
                    delfriend();
                    break;
                }
            case 'e':
                {
                    seefriend();
                    break;
                }
            case 'f':
                {
                    noticemes_manage();
                    break;
                }
            case 'd':
                {
                    chat_one();
                    break;
                }
            case 'o':
                {
                    see_onerecord();
                    break;
                }
            case 'm':
                {
                    jinyan();
                    break;
                }
            case 'g':
                {
                    leavemes();
                    break;
                }
            case 'h':
                {
                    build_group();
                    break;
                }
            case 'i':
                {
                    invite_group();
                    break;
                }
            case 'r':
                {
                    remove_one();
                    break;
                }
            case 'j':
                {
                    addgroup();
                    break;
                }
            case 'k':
                {
                    quitgroup();
                    break;
                }
            case 'l':
                {
                    chat_group();
                    break;
                }
            case 't':
                {
                    see_grouprecord();
                    break;
                }
            case 'z':
                {
                    seegroup();
                    break;
                }
            case 'p':
                {
                    release_group();
                    break;
                }
            case 'x':
                {
                    req_sendfile();
                    break;
                }
            case 'v':
                {
                    send_file();
                    break;
                }
        }
    }while(choice!='q');
}

//退出
void quit()
{
    packet sendpack;
    memset(&sendpack,0,sizeof(packet));
    if(send(conn_fd,&sendpack,sizeof(packet),0)<0)
    {
        my_err("send",__LINE__);
        exit(1);
    }
}

//登录菜单
int login_menu()
{
    int re=0;
    char flag=0;
    char a;
    do
    {
        system("clear");
        printf("\t\033[35m                              \033[0m\n");
        printf("\033[35m\t      welcome to COCO Chat    \033[0m\n");
        printf("\t\033[35m                       V6.6.6 \033[0m\n");
        printf("\n");     
        printf("\t\033[35m                              \033[0m\n");
        printf("\033[35m\t          COCO Chat           \033[0m\n");
        printf("\t\033[35m                              \033[0m\n");
        printf("\033[35m\t          l:登录              \033[0m\n");
        printf("\t\033[35m                              \033[0m\n");
        printf("\033[35m\t          r:新用户注          \033[0m\n");
        printf("\t\033[35m                              \033[0m\n");
        printf("\033[35m\t          q:退出              \033[0m\n");
        printf("\t\033[35m                              \033[0m\n");
        setbuf(stdin,NULL);
        printf("请输入你的选择:");
        scanf("%c",&a);
        switch(a)
        {
            case 'l':
                {
                    re=sign_in();
                    flag=1;
                    sleep(1);
                    //main_menu();
                    break;
                }

            case 'r':
                {
                    sign_up();
                    sleep(2);
                    break;
                }
            case 'q':
                {
                    quit();
                    break;
                }
        }
    }while(a!='q'&&!re);
    return re;
}

//主要用来接收服务端发来的数据
void *recvthread(void *arg)
{
    packet recvpack;
    while(1)
    {
        int ret = 0;
        if((ret = recv(conn_fd,&recvpack,sizeof(packet),0))<0)          
        {
            my_err("recv",__LINE__);
            exit(1);
        }
        switch(recvpack.type)
        {
            case '1':
                {
                    printf("\n您要添加的用户%s不存在\n!",recvpack.data.to);
                    break;
                }
            case 'a':
                {
                    printf("\n[系统提示]:用户%s请求添加您为好友!请前往消息管理查看\n",recvpack.data.from);
                    pack[ke++]=recvpack;
                    break;
                }
            case 'c':
                {
                    if(recvpack.data.message[0]=='y')
                    {
                        printf("\n[系统提示]用户%s已同意添加您为好友!\n",recvpack.data.from);
                    }
                    if(recvpack.data.message[0]=='n')
                    {
                        printf("\n[系统提示]用户%s拒绝了你的好友添加!\n",recvpack.data.from);
                    }
                    break;
                }
            case '2':
                {
                    printf("\n[系统提示]您的好友列表中无%s这个人,不必删除!\n",recvpack.data.message);
                    break;
                }
            case 'b':
                {
                    printf("\n[系统提示]好友%s删除成功!\n",recvpack.data.message);
                    sleep(1);
                    break;
                }
            case '6':
                {
                    printf("\n您要添加的好友之前已经添加过了,请不要重复添加!\n");
                    break;
                }
            case '3':
                {
                    printf("\n您要私聊的好友不存在!\n");
                    break;
                }
            case 'd':
                {
                    printf("\n[私聊] %s>say:%s\n",recvpack.data.from,recvpack.data.message);
                    break;
                }
            case 'o':
                {
                    printf("\n\033[34m%s\033[0m %s->say:%s\n",recvpack.data.name[0],recvpack.data.from,recvpack.data.message);
                    break;
                }
            case '4':
                {
                    printf("\n[提示消息]您目前没有好友可查看,快快去添加吧!\n");
                    break;
                }
            case 'e':
                {
                    int i;
                    printf("\n^~~~~~~~~~~~~~好友列表~~~~~~~~~~~~~~~~~\n");
                    for(i=0;i<recvpack.data.sendfd;i++)
                    {
                        printf("\t%s\n",recvpack.data.name[i]);
                    }
                    printf("\n~~~~~~~~~~~~~~~~~[END]~~~~~~~~~~~~~~~~~\n");
                    break;
                }
            case 'h':
                {

                    if(recvpack.data.message[0]=='n')
                    {
                        printf("\n[提示消息]创建失败!您的群名已经被使用!\n");
                    }
                    else
                    {
                        printf("\n[提示消息]您的群创建成功,快去邀请小伙伴吧!\n");
                    }
                    break;
                }
            case 'j':
                {
                    if(recvpack.data.name[0][0]=='p')
                    {
                        printf("\n您已经在%s群里了,不能重复添加!\n",recvpack.data.message);
                    }
                    if(recvpack.data.name[0][0]=='m')
                    {
                        printf("\n您想进入的群%s不存在!\n",recvpack.data.message);
                    }
                    break;
                }
            case '7':
                {
                    printf("\n[系统通知]有申请加群消息,请前往消息管理查看\n");
                    pack[ke++]=recvpack;
                    break;
                }
            case '8':
                {
                    if(recvpack.data.name[0][0]=='y')
                    {
                        printf("\n群主%s同意您加入群%s!\n",recvpack.data.from,recvpack.data.message);
                    }
                    if(recvpack.data.name[0][0]=='n')
                    {
                        printf("\n群主%s拒绝了你加入群%s的申请!\n",recvpack.data.from,recvpack.data.message);
                    }
                    break;
                }
            case 'i':
                {
                    if(recvpack.data.name[0][0]=='a')
                    {
                        printf("\n您的好友已经在在%s这个群中!不用邀请啦!\n",recvpack.data.message);
                    }
                    if(recvpack.data.name[0][0]=='b')
                    {
                        printf("\n对不起,您不在%s这个群中,无法邀请好友!\n",recvpack.data.message);
                    }
                    if(recvpack.data.name[0][0]=='c')
                    {
                        printf("\n对不起,您输入的群%s不存在\n",recvpack.data.message);
                    }
                    break;
                }
            case '9':
                {
                    printf("\n[系统提示]您有新的群邀请,请前往消息管理查看!\n");

                    pack[ke++]=recvpack;
                    break;
                }
            case '0':
                {
                    if(recvpack.data.name[0][0]=='y')
                    {
                        printf("\n[系统提示]用户%s已接收您的邀请加入群%s\n",recvpack.data.from,recvpack.data.message);
                    }
                    if(recvpack.data.name[0][0]=='n')
                    {
                        printf("\n[系统提示]用户%s拒绝了你的邀请!\n",recvpack.data.from);
                    }
                    break;
                }
            case 'k':
                {
                    printf("\n您已退出群%s\n",recvpack.data.message);
                    break;
                }
            case 'm':
                {
                    printf("\n您要聊天的群不存在!\n");
                    break;
                }
            case 'n':
                {
                    if(kai==0)
                    {
                        box[qun++]=recvpack;
                        printf("\n群%s有新消息\n",recvpack.data.name[0]);
                    }
                    else
                    {
                        printf("%s->say:%s\n",recvpack.data.from,recvpack.data.message);
                    }
                    break;
                }
            case '$':
                {
                    printf("\n\t[%s被群主%s禁言%d秒]\n",recvpack.data.to,recvpack.data.from,recvpack.data.size);
                    break;
                }
            case '@':
                {
                    printf("\n\t[%s的禁言已解除!]\n",recvpack.data.to);
                    break;
                }
            case 't':
                {
                    printf("\n\033[34m%s\033[0m %s->say:%s\n",recvpack.data.name[0],recvpack.data.from,recvpack.data.message);
                    break;
                }
            case '5':
                {
                    printf("[提示消息]您目前还没有加群,快去添加吧!\n");
                    break;
                }
            case 'z':
                {
                    printf("\n^~~~~~~~~~~~~~~~~~群列表~~~~~~~~~~~~~~~^\n");
                    for(int j=0;j<recvpack.data.size;j++)
                    {
                        printf("%s\n",recvpack.data.name[j]);
                    }
                    printf("\n^~~~~~~~~~~~~~~~~~[END]~~~~~~~~~~~~~~~~^\n");

                }
                break;
            case 'u':
                {
                    printf("\n[提示]您要解散的群%s不是你的群，你不能解散它!\n",recvpack.data.message);
                    break;
                }
            case 'p':
                {
                    printf("\n[提示]您已解散群%s!\n",recvpack.data.message);
                    break;
                }
            case 's':
                {
                    printf("\n你要发给的好友不存在!\n");
                    break;
                }
            case 'x':
                {
                    printf("\n[系统提示]好友%s请求向你发送文件,请移步消息管理查看\n",recvpack.data.from);
                    pack[ke++]=recvpack;
                    break;
                }
            case 'w':
                {
                    if(recvpack.data.name[0][0]=='y')
                    {
                        printf("\n用户%s同意您发送文件%s,你可以去发送文件了!\n",recvpack.data.from,recvpack.data.message);
                    }
                    if(recvpack.data.name[0][0]=='n')
                    {
                        printf("\n用户%s拒绝你发送文件%s\n",recvpack.data.from,recvpack.data.message);
                    }
                    break;
                }
            case 'l':
                {
                    shou=1;
                    printf("\n[系统提示]对不起,对方不想接收你的文件%s!",recvpack.data.name[0]);
                    break;
                }
            case 'r':
                {
                    shou=1;
                    printf("\n[系统提示]对不起,您要发的文件，还没请求接收方的同意!");
                    break;
                }
            case 'f':
                {
                    if(recvpack.data.name[0][0]=='g')
                    {
                        printf("\n[系统提示]您输入的群不存在!\n");
                    }
                    if(recvpack.data.name[0][0]=='o')
                    {
                        printf("\n[系统提示]对不起!您不是此群群主,无权移除他人\n");
                    }
                    if(recvpack.data.name[0][0]=='p')
                    {
                        printf("\n[系统提示]群%s里本就没有%s这个人\n",recvpack.data.message,recvpack.data.to);
                    }
                    break;
                }
            case 'g':
                {
                    printf("\n[系统提示]您已将%s移出群%s\n",recvpack.data.to,recvpack.data.message);
                    break;
                }
            case '#':
                {
                    printf("\n[系统提示]您不是%s群的群主,无权使用此功能!\n",recvpack.data.message);
                    break;
                }
            case 'v':
                {
                    
                    acceptfile(recvpack);
                    break;
                }
        }
    }
}

//主函数
int main(int argc,char **argv)
{
    int re;
    int i,t;
    int ret;
    pthread_t tid,rtid;
    int serv_port;
    struct sockaddr_in serv_addr;
    char recv_buf[CHARSIZE];
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex,NULL);
    //检查参数个数
    if(argc!=5)
    {
        printf("usage: [-p] [serv_port] [-a] [serv_address]\n");
        exit(1);
    }

    //初始化服务器端地址结构
    memset(&serv_addr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family=AF_INET;
    //从命令行获取服务端端口与地址
    for(i=1;i<argc;i++)
    {
        if(strcmp("-p",argv[i])==0)
        {
            serv_port=atoi(argv[i+1]);
            if(serv_port<0||serv_port>65535)
            {
                printf("invalid serv_addr.sin_port\n");
                exit(1);
            }
            else
            {
                serv_addr.sin_port=htons(serv_port);
            }
            continue;
        }
        if(strcmp("-a",argv[i])==0)
        {
            if(inet_aton(argv[i+1],&serv_addr.sin_addr)==0)
            {
                printf("invalid server ip address\n");
                exit(1);
            }
            continue;
        }
    }
    //检查是否少输入了某项参数
    if(serv_addr.sin_port==0 ||serv_addr.sin_addr.s_addr==0)
    {
        printf("usage: [-p] [server_addr.sin_port] [-a] [serv_address]\n");
        exit(1);
    }

    //创建一个TCP套接字
    conn_fd=socket(AF_INET,SOCK_STREAM,0);
    if(conn_fd<0)
    {
        my_err("socket",__LINE__);
    }
    //向服务端发送连接请求
    if(connect(conn_fd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))<0)
    {
        my_err("connect",__LINE__);
    }
    re=login_menu();
    if(pthread_create(&tid,NULL,recvthread,NULL)!=0) //接收包的线程
    {
            my_err("pthread_create",__LINE__);
    }
    if(re==1)
    {
        main_menu();
    }
}
