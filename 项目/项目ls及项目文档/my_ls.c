#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/types.h>
#include<linux/limits.h>
#include<dirent.h>
#include<grp.h>
#include<pwd.h>
#include<errno.h>

#define WU  0      //无参数
#define A   1      //-a: 显示所有文件
#define L   2      //-l: 列出文件的详细信息
#define R   4      //-R: 将目录下所有的子目录的文件都列出来
#define ZIFU  120     //一行显示的最多字符数

int fx=0;

int leavelength = ZIFU; //一行剩余长度,用于输出对齐
int maxlen;                //存放某目录下最长文件名的长度


/*错误处理函数*/
void my_err(char *err_string,int line)
{
    fprintf(stderr,"line:%d ",line);
    perror(err_string);
    exit(1);
}

//打印文件名且保证对齐
void filenot_l(char *filename,int n)
{
    int i;
    if(leavelength<maxlen)
    {
        printf("\n");
        leavelength=ZIFU;
    }
    printf("%s",filename);
    for(i=0;i<maxlen-n;i++)
    {
        printf(" ");
    }

    printf("  ");
    leavelength = leavelength-maxlen-2;
}
void print_l(struct stat buf,char *name)
{
    int t;
    char src_time[40];
    struct passwd *user;  //从该结构体中获取文件所有者的用户名
    struct group *group;   //从该结构体中获取文件所有者所属组的组名

    /*获取并打印文件类型*/
    if(S_ISLNK(buf.st_mode))
    {
        printf("l");
    }
    else if(S_ISREG(buf.st_mode))
    {
        printf("-");
    }
    else if(S_ISDIR(buf.st_mode))
    {
        printf("d");
    }
    else if(S_ISCHR(buf.st_mode))
    {

        printf("c");
    }
    else if(S_ISBLK(buf.st_mode))
    {
        printf("b");
    }
    else if(S_ISFIFO(buf.st_mode))
    {
        printf("f");
    }
    else if(S_ISSOCK(buf.st_mode))
    {
        printf("s");
    }
     /*获取并打印文件所有者的权限*/
    if(buf.st_mode & S_IRUSR){
        printf("r");
    }
    else{
        printf("-");
    }
    if(buf.st_mode & S_IWUSR){
        printf("w");
    }
    else{
        printf("-");
    }
    if(buf.st_mode & S_IXUSR){
        printf("x");
    }
    else{
        printf("-");
    }

    /*获取并打印与文件所有者同组的用户对该文件的操作权限*/
    if(buf.st_mode & S_IRGRP){
        printf("r");
    }
     else{
        printf("-");
    }
    if(buf.st_mode & S_IWGRP){
        printf("w");
    }
    else{
        printf("-");
    }
    if(buf.st_mode & S_IXGRP){
        printf("x");
    }
    else{
        printf("-");
    }

    /*获取并打印其他用户对该文件的操作权限*/
    if(buf.st_mode & S_IROTH){
        printf("r");
    }
    else{
        printf("-");
    }
    if(buf.st_mode & S_IWOTH){
        printf("w");
    }
    else
    {
        printf("-");
    }
    if(buf.st_mode & S_IXOTH){
        printf("x");
    }
    else{
        printf("-");
    }
    printf(" ");

    printf("%3d ",(int)buf.st_nlink);
    user=getpwuid(buf.st_uid);
    group=getgrgid(buf.st_gid);
    printf("%-5s",user->pw_name);
    printf("%-5s",group->gr_name);
    printf("%5d",(int)buf.st_size);
    strcpy(src_time,ctime(&buf.st_mtime));
    t=strlen(src_time);
    src_time[t-1]='\0';
    printf(" %s",src_time);
    printf(" %s",name);
    printf("\n");
}



void myls_dir(int mode,char *path)
{
    int flag=0;
    int i=0,k,j;
    int len[300];
    int dian=0;
    int total=0;
    DIR *dir;
    struct stat buf,buff,bbuf;
    char dianname[2][50];
    struct dirent *ptr;
    int a=strlen(path);
    char names[500][PATH_MAX+1];
    char t[PATH_MAX+1];
    char filename[NAME_MAX+1];
    char dirr[100][100];
    int recount=0;

    if((dir=opendir(path)) == NULL){
        my_err("opendir",__LINE__);
    }
    while((ptr = readdir(dir))!=NULL){
       // printf("%s\n",ptr->d_name);     //测试查看
        strncpy(names[i],path,a);
        if(!fx)
        {
            names[i][a]='\0';
            strcat(names[i],ptr->d_name);
            names[i][a+strlen(ptr->d_name)]='\0';
        }
        else
        {
            names[i][a]='/';
            names[i][a+1]='\0';
            strcat(names[i],ptr->d_name);
            names[i][a+1+strlen(ptr->d_name)]='\0';
        }
        total++;
        i++;
    }
    closedir(dir);
 
    maxlen=strlen(names[0]);
    for(j=1;j<total;j++)
    {
        if(strlen(names[j])>maxlen){
            maxlen=strlen(names[j]);
        }
    }

    for(i=0;i<total-1;i++)
    {
        for(j=0;j<total-i-1;j++)
        {
            if(strcmp(names[j],names[j+1])>0)
            {
                strcpy(t,names[j+1]);
                t[strlen(names[j+1])]='\0';
                strcpy(names[j+1],names[j]);
                names[j+1][strlen(names[j])]='\0';
                strcpy(names[j],t);
                names[j][strlen(t)+1]='\0';
            }

        }
    }
    for(i=0;i<total;i++)
    {

        /*从路径中解析出文件名*/
       // printf("%s\n",names[i]);
        for(k=0,j=0;k<strlen(names[i]);k++){
            if(names[i][k]=='/'){
                j=0;
                continue;
            }
            filename[j] = names[i][k];
            j++;
        }
        filename[j]='\0';
        len[i]=j+1;

        if(lstat(names[i],&buf) == -1)
        {
            my_err("lstat",__LINE__);
        }

        switch(mode)
        {
            case WU:{
                if(filename[0]!='.')
                {
                    filenot_l(filename,len[i]);  
                }
            }break;

            case A:{
                filenot_l(filename,len[i]);
            }break;

            case L:{
                flag=1;
                if(filename[0] !='.')
                {
                    print_l(buf,filename);
                }
            }break;
            case A+R:
            case R+L:
            case A+R+L:
            case R:{
                if(filename[0]!='.')
                {
                    if(S_ISDIR(buf.st_mode))
                    {
                        if(mode==6||mode==7)
                        {
                            if(mode==7&&dian==0)
                            {
                                strcpy(dianname[0],".");
                                lstat(dianname[0],&buff);
                                print_l(buff,dianname[0]);
                                strcpy(dianname[1],"..");
                                lstat(dianname[1],&bbuf);
                                print_l(bbuf,dianname[1]);
                                dian++;
                            }
                            print_l(buf,filename);
                        }
                        else
                        {
                            if(mode==5&&dian==0)
                            {
                                strcpy(dianname[0],".");
                                filenot_l(dianname[0],strlen(dianname[0]));
                                strcpy(dianname[1],"..");
                                filenot_l(dianname[1],strlen(dianname[0]));
                                dian++;
                            }
                            filenot_l(filename,len[i]);
                        }
                        strcpy(dirr[recount],names[i]);
                        recount++;
                        fx=1;
                    }
                    else
                    {
                        if(mode==6||mode==7)
                        {
                            if(mode==7&&dian==0)
                            {
                                strcpy(dianname[0],".");
                                lstat(dianname[0],&buff);
                                print_l(buff,dianname[0]);
                                strcpy(dianname[1],"..");
                                lstat(dianname[1],&bbuf);
                                print_l(bbuf,dianname[1]);
                                dian++;
                            }
                            print_l(buf,filename);
                        }
                        else
                        {
                            if(mode==5&&dian==0)
                            {
                                strcpy(dianname[0],".");
                                filenot_l(dianname[0],strlen(dianname[0]));
                                strcpy(dianname[1],"..");
                                filenot_l(dianname[1],strlen(dianname[0]));
                                dian++;
                            }
                            filenot_l(filename,len[i]);
                        }
                    }
                }
            }break;

            case A+L:{
                flag=1;
                print_l(buf,filename);
            }break;

            default:break;
        }
    }
    if(mode == 4||mode==5||mode==6||mode==7)
    {
        int s=0;
        printf("\n");
        for(s=0;s<recount;s++){
            printf("%s:\n",dirr[s]);
            myls_dir(mode,dirr[s]);
            printf("\n");
        }
    }
    if(!flag)
       printf("\n");
}

int main(int argc,char ** argv)
{
    int f,d,i,j,t,k=0;
    int len;
    int count=0;
    char mode[30];
    char path[50];
    int param = WU;
    struct stat buf;
    
    for(i=1;i<argc;i++)
    {
        if(argv[i][0]=='-')
        {
            count++;
            t=strlen(argv[i]);
            for(j=1;j<t;j++)
            {
                mode[k]=argv[i][j];
                k++;
            }
        }
    }
    if(count > 0)
    {
        d=strlen(mode);
        for(i=0;i<d;i++)
        {
            if(mode[i]=='a')
            {
                param += A;
            }
            else if(mode[i]=='l')
            {
                param += L;
            }
            else if(mode[i]=='R')
            {
                param += R;
            }
            else 
            {
                printf("my_ls :-%c error\n",mode[i]);
            }
        }
    }
    if(count==argc-1)
    {
        strcpy(path,"./");
        path[2]='\0';
       // printf("%s\n",path);
        myls_dir(param,path);
    }
    else
    {
        for(i=1;i<argc;i++)
        {
            if(argv[i][0]=='-')
                continue;
            else
            {
                len=strlen(argv[i]);
                strncpy(path,argv[i],len);
                //printf("%s\n",path);
                if(stat(path,&buf) == -1)
                {
                    my_err("stat",__LINE__);
                }
                if(S_ISDIR(buf.st_mode)){
                    if(path[len-1]=='/')
                    {
                        path[len]='\0';
                        myls_dir(param,path);
                    }
                    else
                    {
                        path[len]='/';
                        path[len+1]='\0';
                        myls_dir(param,path);
                    }
                    //printf("%s\n",path);
                }
                else
                {
                    myls_dir(param,path);
                }
            }
        }
    }
    return 0;
}
