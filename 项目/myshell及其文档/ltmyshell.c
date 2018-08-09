#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<linux/limits.h>
#include<fcntl.h>
#include<sys/stat.h>
#include <readline/history.h>
#include<dirent.h>
#include<pwd.h>

#define normal         0 /*一般的命令*/
#define out_redirect   1 /*输出重定向*/
#define in_redirect    2 /*输入重定向*/
#define have_pipe      3 /*命令中有管道*/


char history[300][300];
int hcount=1;

/*自定义的错误处理函数***********************************/
void my_err(const char *err_string,int line)
{
    fprintf(stderr,"line:%d ",line);
    perror(err_string);
    exit(1);
}
/*打印提示符,包括用户名，主机名，显示当前目录(cd),提示符，以及某些cd某些功能******************/
void print_tishi()
{
    uid_t uid;
    struct passwd *psd;
    struct mulu *q,*p;
    int i,j,t;
    char src[40];
    char buf[40];
    uid=getuid();
    psd=getpwuid(uid);

    if(getcwd(src,40)<0)   //获取当前目录
    {
        my_err("getcwd",__LINE__);
    }
    memset(buf,0,40);      //初始化buf,将buf所指向的空间清零
    if(strcmp(src,"/home/ltt")==0)
    {
        strcpy(buf,"~");
        buf[1]='\0';
    }
    else
    {
        if(strcmp(src,"/")==0)
        {
            strcpy(buf,"/");
            buf[1]='\0';
        }
        else if(strncmp(src,"/home/ltt",9)==0)
        {
            t=strlen(src);
            for(i=9,j=1;i<t;i++,j++)
            {
                buf[j]=src[i];
            }
            buf[0]='~';
            buf[t-8]='\0';
        }
        else
        {
            t=strlen(src);
            for(i=0;i<t;i++)
            {
                buf[i]=src[i];
            }
            buf[i]='\0';
        }
    }
    printf("\033[1;35m%s@myshell-pc:\033[0m\033[1;34m%s\033[0m\033[35m$\033[0m ",psd->pw_name,buf);
}

/*将输入的命令进行分析，保存到buf中********************/
void get_input(char *buf)
{
    int len=0;
    int ch,i;
    int licount=0;
    ch = getchar();
    while(len<256&&ch!='\n')
    {
        buf[len++]=ch;
        ch=getchar();
    }
    if(len==256)
    {
        printf("commond is too long\n");
        exit(-1);//输入的命令过长，则退出程序
    }
    buf[len]='\n';
    buf[len+1]='\0';
    for(i=0;i<strlen(buf);i++)
    {
        if(buf[i]==' '||buf[i]=='\n')
        {
            licount++;
        }
    }
    if(licount!=strlen(buf))
    {
        if(hcount!=1){
            if(strcmp(history[hcount-1],buf) != 0){
                strcpy(history[hcount],buf);
                hcount = hcount + 1;
                add_history(buf);
            }
        }
        else
        {
            strcpy(history[hcount],buf);
            hcount=hcount+1;
            add_history(buf);
        }
    }
}

/*解析buf中的命令，将结果存入arglist中,命令以回车符号\n结束************************/
/*如输入命令为"ls -l /tmp",则arglist[0],arglist[1],arglist[2]分别为ls、-l、和/tmp */
void explain_input(char *buf,int *argcount,char arglist[100][256])
{
    char *p = buf;
    char *q = buf;
    int number=0;
    while(1)
    {
        if(p[0]=='\n')
            break;
        if(p[0]==' ')
            p++;
        else
        {
            q=p;
            number=0;
            while((q[0]!=' ')&& (q[0]!='\0')&&(q[0]!='\n'))
            {
                number++;
                q++;
            }
            strncpy(arglist[*argcount],p,number+1);
            arglist[*argcount][number]='\0';
            *argcount = *argcount+1;
            p=q;
        }
    }
}

/*查找命令中的可执行程序*********************/
int find_command(char *command)
{
    DIR *dp;
    struct dirent *dir;
    char *path[]={"./","/bin","/usr/bin",NULL};
    //使当前目录下的程序可以运行.
    if(strncmp(command,"./",2)==0)
    {
        command=command+2;
    }

    int i=0;
    //分别在当前目录，/bin和/usr/bin目录查找要执行的程序
    while(path[i]!=NULL)
    {
        if((dp=opendir(path[i]))==NULL)
        {
            printf("can not open /bin \n");
        }
        while((dir=readdir(dp))!=NULL)
        {
            if(strcmp(dir->d_name,command)==0)
            {
                closedir(dp);
                return 1;
            }
        }
        closedir(dp);
        i++;
    }
    return 0;
}



/*执行命令*************************************/
void do_command(int argcount,char arglist[100][256])
{
    int j;
    int flag=0;
    int how=0;      //用于指示命令中是否含有>、<、|
    int background=0;  //标识命令中是否含有后台运行标识符&
    int status,i,fd;
    char path[40];
    char *arg[argcount+1];
    char *argnext[argcount+1];
    char *file;
    pid_t pid;
    
    /*将命令取出 */
    for(i=0;i<argcount;i++)
    {
        arg[i]=(char *)arglist[i];
    }
    arg[argcount]= NULL;
     
    //设置当前工作目录，即实现cd
    for(i=0;i<argcount;i++)
    {
        if(strncmp(arg[0],"cd",2)==0)
        {
            if((arg[1]==NULL)||(strncmp(arg[1],"~",1)==0))
            {
                strncpy(path,"/home/ltt",10);
                if(chdir(path)<0)
                {
                    my_err("chdir",__LINE__);
                }
                return ;
            }
            else
            {
                if(chdir(arg[1])<0)
                {
                    //printf("%s**\n",arg[1]);
                    my_err("chdir",__LINE__);
                }
                return ;
            }
        }
    }
    for(i=0;i<argcount;i++)
    {
        if(strncmp(arg[0],"history",7)==0)
        {
            for(j=1;j<hcount;j++)
            {
                printf("%d  %s",j,history[j]);
            }
            return ;
        } 
    }

    
    /*查看命令行是否有后台运行符*/
    for(i=0;i<argcount;i++)
    {
        if(strncmp(arg[i],"&",1)==0)
        {
            if(i==argcount-1)
            {
                background = 1;
                arg[argcount-1] =NULL;
                break;
            }
            else
            {
                printf("wrong command\n");
                return ;
            }
        }
    }
    for(i=0;arg[i]!=NULL;i++)
    {
        if(strcmp(arg[i],">")==0)
        {
            flag++;
            how=out_redirect;
            if(arg[i+1]==NULL)
                flag++;
        }
        if(strcmp(arg[i],"<")==0)
        {
            flag++;
            how=in_redirect;
            if(i==0)
            {
                flag++;
            }
        }
        if(strcmp(arg[i],"|")==0){
            flag++;
            how=have_pipe;
            if(arg[i+1]==NULL)
                flag++;
            if(i==0)
                flag++;
        }
    }

    //flag大于1,说明含有多个>,<,|.本程序不支持,或者命令格式不对
    if(flag>1)
    {
        printf("wrong command\n");
        return;
    }
    if(how == out_redirect)   //命令中只含有一个输出重定向符号>
    {
        for(i=0;arg[i]!=NULL;i++)
        {
            if(strcmp(arg[i],">")==0)
            {
                file = arg[i+1];
                arg[i]=NULL;
            }
        }
    }
    if(how==in_redirect)    //命令中只含有一个输入重定向符号<
    {
        for(i=0;arg[i]!=NULL;i++)
        {
            if(strcmp(arg[i],"<")==0)
            {
                file = arg[i+1];
                arg[i]=NULL;
                
            }
        }
    }

    if(how==have_pipe)   //命令只含有一个管道符号|
    {
        //把管道后面的部分存入argnext中，管道后面的部分是一个管道命令(此命令必须能够接收standard input的数据,如less,more,wc等)
        //管道命令必须要能够接收来自前一个命令的数据成为standard input继续处理才行
        for(i=0;arg[i]!=NULL;i++)
        {
            if(strcmp(arg[i],"|")==0)
            {
                arg[i]=NULL;
                int j;
                for(j=i+1;arg[j]!=NULL;j++)
                {
                    argnext[j-i-1]=arg[j];
                }
                argnext[j-i-1]=arg[j];
                break;
            }
        }
    }
    if((pid=fork())<0)
    {
        printf("fork error\n");
        return;
    }
    switch(how)
    {
        case 0:{
            if(pid==0)            //在子进程中执行输入的命令，输入的命令中不包含>,<和|.
            {
                if(!(find_command(arg[0])))
                {
                    printf("%s:command not found!\n",arg[0]);
                    exit(0);
                }
                execvp(arg[0],arg);
                exit(0);
            }
        }break;

        case 1:{                   //输入的命令中含有输出重定向符>
            if(pid==0){
                if(!(find_command(arg[0]))){
                    printf("%s:command not found\n",arg[0]);
                    exit(0);
                }
                fd=open(file,O_RDWR|O_CREAT|O_TRUNC,0644);//把屏幕中输出流交给文件
                dup2(fd,1);
                execvp(arg[0],arg);
                exit(0);
            }
        }break;
        case 2:{                 //输入的命令中含有输入重定向<
            if(pid==0)
            {
                if(!(find_command(arg[0])))
                {
                    printf("%s:command not found\n",arg[0]);
                    exit(0);
                }
                fd=open(file,O_RDONLY);
                dup2(fd,0);
                execvp(arg[0],arg);
                exit(0);
            }
        }break;        
        case 3:{                //输入的命令中含有管道符|
            if(pid==0)
            {
                int pid2;
                int status2;
                int fd2;
                if((pid2=fork())<0)
                {
                    printf("fork2 error\n");
                    return;
                }
                else if(pid2==0)
                {
                    if(!(find_command(arg[0])))
                    {
                        printf("%s:command not found\n",arg[0]);
                        exit(0);
                    }
                    if((fd2=open("/tmp/wumingfile",O_WRONLY|O_CREAT|O_TRUNC,0644))==-1);
                    {
                        printf("open /tmp/wumingfile failed!!");
                    }
                    dup2(fd2,1);
                    execvp(arg[0],arg);
                    exit(0);
                }
                if(waitpid(pid2,&status2,0)==-1)
                {
                    printf("wait for child process error\n");
                }
                if(!(find_command(argnext[0])))
                {
                    printf("command '%s'not found\n",argnext[0]);
                    exit(0);
                }
                if((fd2=open("/tmp/wumingfile",O_RDONLY))==-1)
                {
                    my_err("open",__LINE__);
                }
                dup2(fd2,0);
                execvp(argnext[0],argnext);

                if(remove("/tmp/wumingfile")==-1) //移除文件                  
                        printf("remove error\n");
                exit(0);
            }
        }break;
        default:break;
    }
    //若命令中有&，表示后台运行，父进程直接返回，不等待子进程结束
    if(background==1)
    {
        printf("[process id: %d]\n",pid);
        return ;
    }
    //父进程等待子进程结束
    if(waitpid(pid,&status,0)==-1)
    {
        printf("wait for child process error\n");
    }
    
}



int main(int argc,char **argv)
{
    int i;
    signal( SIGINT,SIG_IGN );
    int argcount = 0;
    char arglist[100][256];
    char **arg=NULL;
    char *buf=NULL;
    buf = (char *)malloc(256);

    memset(history,0,sizeof(history));
    if(buf == NULL)
    {
        perror("malloc failed");
        exit(-1);
    }
    while(1)
    {
        /*将buf所指向的空间清零*/
        memset(buf, 0,256);
        print_tishi();
        get_input(buf);
        /*若输入的命令为exit或logout则退出本程序*/
        if(strcmp(buf,"exit\n")==0||strcmp(buf,"logout\n")==0)
        {
            break;
        }
        for(i=0;i<100;i++)
        {
            arglist[i][0]='\0';
        }
        argcount = 0;    //命令个数为0
        explain_input(buf,&argcount,arglist);
        do_command(argcount,arglist);
    }
    if(buf!=NULL)
    {
        free(buf);    //将buf所指向的内存释放,但是buf所指的地址仍然不变，原来的内存变为垃圾内存
        buf = NULL;   //将buf所指的地址置空,放弃它,防止出错.
    }
    exit(0);
}


