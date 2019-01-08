/*************************************************************************
    > File Name: daohangtu.c
    > Author: 
    > Mail: 
    > Created Time: 2018年12月30日 星期日 18时37分42秒
 ************************************************************************/
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <termios.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<assert.h>
#include<stdlib.h>
#include<errno.h>
#include "md5.c"



//查找图顶点信息(普通用户)
//插入图顶点操作
//修改图顶点信息
//删除图顶点操作
//插入边操作
//修改边信息操作
//删除边操作
//最短路径求解(普通用户)

#define MAXVEX 100       //最大顶点个数
#define INFINITY 32767   //表示极大值


typedef struct{
    int no; //景点序号
    char name[50];  //景点名称
    char info[200];  //景点描述
    int x;
    int y;     //景点坐标
}Info;

typedef struct
{
    int weight[3];  //1.长度,2.景色优美指数,3.绿化指数 
}Edgeinfo;

typedef struct ArcNode{
    int adjvex; //该边所指向的顶点的位置即索引
    Edgeinfo info;
    struct ArcNode *next;   //指向下一条弧的指针
}ArcNode;

typedef struct
{
    Info data2;       //顶点信息
    ArcNode *head;    //指向第一条依附于该顶点的弧
}VertexNode;

typedef struct
{
    VertexNode vertex[MAXVEX];
    int vexnum;        //顶点数
    int arcnum;        //弧数
}AdjList;

typedef struct
{
    char name[60];
}Data;


//登录菜单
void login_menu();
//管理员登录
void signadmin();
//管理员菜单
void Admin();
//游客登录
int signyou();
//校园平面简图
void pingmiantu(AdjList *G);
//绿化最好的路径
void greenway(AdjList *G);
//求最美路径
void beautyway(AdjList *G);

//求最优路径
void shortway(AdjList *G);
//采用Dijkstra算法求得从起点景点到各个终点景点的最短路线
//disk数组用于存储最短路径权值和
void Dijkstra(AdjList *G,int start,int end,int dist[],int path[][MAXVEX],int flag);
//添加新道路
void addroad(AdjList *G);
//修改道路信息
int Modifyroad(AdjList *G);
//删除某路线
void deleteroad(AdjList *G);
//查询在d和e之间的边是否存在?
int Serchroad(AdjList *G,int d,int e);
//查看景点介绍
void view(AdjList *G);
//删除某个校园景点
void deletepoint(AdjList *G);
//修改某景点,参数为要修改的顶点的名称
int Modifypoint(AdjList *G);
//添加新景点
void addpoint(AdjList *G);
//删除图中某顶点,以及删除每一个和它有关联的边
void DeleteVex(AdjList *G,int d);
//查找图中顶点
int SearchVertex(AdjList *G,char name[30]);
//输出所有路径
void print(AdjList *G);
//采用邻接表创建无向图
void Create(AdjList *G);
//输入密码
void input_mima(char *pd);
int getch();


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


//采用邻接表创建无向图
void Create(AdjList *G)
{
    int i,j,m,n,weight,weight1,weight2;
    int p1,p2;
    ArcNode *pr;
    ArcNode *node1,*node2;
    FILE *fp1,*fp2;
    fp1=fopen("tubian.txt","r");
    fp2=fopen("tudingdian.txt","r");
    fscanf(fp1, "%d %d", &G->vexnum, &G->arcnum);
    if (G->vexnum < 1||G->arcnum<1||(G->arcnum> (G->vexnum*(G->vexnum-1))))
    {
        printf("input error: invalid parameters!\n");
        return;
    }
    for(i = 1; i <= G->vexnum; i++){
        G->vertex[i].data2.no = i;
        fscanf(fp2, "%s %d %d %s", G->vertex[i].data2.name,&G->vertex[i].data2.x,&G->vertex[i].data2.y,G->vertex[i].data2.info);
        G->vertex[i].head = NULL;
    }
    //读取边的起始顶点和结束顶点
    for(i = 1; i <= G->arcnum; i++){
        fscanf(fp1,"%d %d %d %d %d",&m,&n,&weight,&weight1,&weight2);
        for(j=1;j<=G->vexnum;j++){
            if(G->vertex[j].data2.no == m){
                p1=j;
            }
            if(G->vertex[j].data2.no == n){
                p2=j;
            }
        }
        //将node1链接到"p1所在链表的末尾"
        node1 = (ArcNode *)malloc(sizeof(ArcNode));
        node1->adjvex = p2;
        node1->info.weight[0]=weight;
        node1->info.weight[1]=weight1;
        node1->info.weight[2]=weight2;
        node1->next=NULL;
        if(G->vertex[p1].head == NULL){
            G->vertex[p1].head = node1;
        }
        else
        {
            pr=G->vertex[p1].head;
            while(pr->next!=NULL){
                pr=pr->next;
            }
            pr->next=node1;
        }
        node2 = (ArcNode *)malloc(sizeof(ArcNode));
        node2->adjvex = p1;
        node2->info.weight[0]=weight;
        node1->info.weight[1]=weight1;//道路美丽指数
        node1->info.weight[2]=weight2;
        node2->next=NULL;
        //将node2链接到"p2所在链表的末尾"
        if(G->vertex[p2].head == NULL)
           G->vertex[p2].head = node2;
        else
        {
            pr=G->vertex[p2].head;
            while(pr->next!=NULL){
                pr=pr->next;
            }
            pr->next=node2; 
        }
    }
    fclose(fp1);
    fclose(fp2);
}



//输出所有路径
void printlj(AdjList *G)
{
    int i;
    ArcNode *pt;
    for(i = 1; i <= G->vexnum; i++) {
        pt=G->vertex[i].head;
        printf("\n%d.%s",G->vertex[i].data2.no,G->vertex[i].data2.name);
        while(pt!=NULL){
            printf("-->[%d,长:%d]",pt->adjvex,pt->info.weight[0]);
            pt=pt->next;
        }
        printf("\n");
    }
    printf("\n\n");

}
//输出节点
void print(AdjList *G)
{
    int i,count = 0;
    printf("\n\n");
    printf("\t***********************coco西邮导航系统************************\n");
    printf("\n\n");
    for(i = 1; i <= G->vexnum; i++) {
        printf(" %d.%-8s\t",G->vertex[i].data2.no,G->vertex[i].data2.name);
        count++;
        if(count == 5) {
            printf("\n");
            count = 0;
        }
    }
    // ArcNode *pt;
    // for(i = 1; i <= G->vexnum; i++) {
    //     pt=G->vertex[i].head;
    //     printf("\n%d.%s",G->vertex[i].data2.no,G->vertex[i].data2.name);
    //     while(pt!=NULL){
    //         printf("-->[%d,长:%d]",pt->adjvex,pt->info.weight[0]);
    //         pt=pt->next;
    //     }
    //     printf("\n");
    // }
    // printf("\n\n");
}

//查找图中顶点
int SearchVertex(AdjList *G,char name[30]){
    int i;
    for(i=1;i<=G->vexnum;i++){
        if(strcmp(G->vertex[i].data2.name,name)==0){
            return i;
        }
    }
    return -1;
}



//删除图中某顶点,以及删除每一个和它有关联的边
void DeleteVex(AdjList *G,int d)   //d被删除顶点下标
{
    int i,j;
    ArcNode *pt,*pr;
    //删除与d相关联的各条边
    pt=G->vertex[d].head;
    while(pt!=NULL){
        pr=pt;
        pt=pt->next;
        G->vertex[d].head=pt;
        free(pr);
        G->arcnum--;
    }
    for(i=1;i<=G->vexnum;i++)
    {
        if(i==d){
            continue;
        }
        else
        {
            pr=G->vertex[i].head;
            pt=pr;
            while(pr!=NULL){
                if(pr->adjvex==d){
                    if(pr==pt){
                        free(pr);
                        G->vertex[i].head=NULL;
                        G->arcnum--;
                        break;
                    }
                    else
                    {
                        pt->next=pr->next;
                        G->arcnum--;
                        free(pr);
                        break;
                    }
                }
                pt=pr;
                pr=pr->next;
            }
        }
    }
    G->vertex[G->vexnum].data2.no=d;
    G->vertex[d]=G->vertex[G->vexnum];
    for(i=1;i<G->vexnum;i++)
    {
        pr=G->vertex[i].head;
        while(pr!=NULL){
            if(pr->adjvex==G->vexnum)
            {
                pr->adjvex=d;
                break;
            }
            pr=pr->next;
        }
    }
    G->vexnum--;    //顶点总数减一
}

//添加新景点
void addpoint(AdjList *G){
    VertexNode node;
    printf("\n\t请输入你要添加的景点名称:");
    scanf("%s",node.data2.name);
    printf("\n\t请输入景点的介绍信息:");
    scanf("%s",node.data2.info);
    printf("\n\t请输入景点的坐标(x,y):");
    scanf("%d,%d",&node.data2.x,&node.data2.y);
    node.data2.no=G->vexnum+1;
    node.head=NULL;
    G->vexnum++;
    G->vertex[G->vexnum]=node;
    printf("\n\t添加成功!\n");
}

//修改某景点,参数为要修改的顶点的名称
int Modifypoint(AdjList *G)
{
    print(G);
    printlj(G);
    int choice;
    char name[50];
    char xiugai1[50];
    char xiugai2[200];
    printf("\n\t请输入你要修改的顶点的名称:");
    scanf("%s",name);
    int d=SearchVertex(G,name);
    do{
        system("clear");
        printf("\n\n");
	    printf("\t\t|*----------------------------------------------*|\n");
	    printf("\t\t|*               1. 名称                         *|\n");
	    printf("\t\t|*               2. 信息                         *|\n");
        printf("\t\t|*               0. 退出                         *|\n");
        printf("\t\t|*----------------------------------------------*|\n");
        printf("\n\n");
        do{
            printf("\t\t请选择您要进行的修改(输入0——1):");
            scanf("%d",&choice);
        }while(choice<0 || choice>4);
        getchar();
        printf("\t\t按任意键进入下一步...");
        getch();
        printf("\n");
        switch(choice)
        {
            case 1:
            printf("\n\t你想把它改成什么：");
            scanf("%s",xiugai1);
            strcpy(G->vertex[d].data2.name,xiugai1);
            break;
            case 2:
            printf("\n\t你想把它改成什么：");
            scanf("%s",xiugai2);
            strcpy(G->vertex[d].data2.info,xiugai2);
            break;
            case 0:return 0;
        }
    }while(1);
}

//删除某个校园景点
void deletepoint(AdjList *G)
{
    char name[50];
    printf("\n\t请输入你要删除的景点的名称:");
    scanf("%s",name);
    int d=SearchVertex(G,name);
    DeleteVex(G,d);
    printf("\n\t删除成功!\n"); 
}

//查看景点介绍
void view(AdjList *G){
    char name[30];
    printf("\n\t\t请输入你要查询的景点名称:");
    scanf("%s",name);
    int d = SearchVertex(G,name);
    printf("\n\t找到咯!\n");
    printf("\n\t【%s】 =>> %s",G->vertex[d].data2.name,G->vertex[d].data2.info);
    printf("\n\t它位于:[%d,%d]\n",G->vertex[d].data2.x,G->vertex[d].data2.y);
    if(d==-1){
        printf("\n\t对不起!,你要查询的景点，不存在!\n");
    }
    printf("\n\t按任意键返回...");
    getch();   
}


//查询在d和e之间的边是否存在?
int Serchroad(AdjList *G,int d,int e)
{
    int i,j;
    ArcNode *pt,*pr;
    pr=G->vertex[d].head;
    while(pr!=NULL){
        if(pr->adjvex==e){
            return 1;
        }
        pr=pr->next;
    }
    return 0;//如果存在道路返回1,不存在道路返回0
}


//删除某路线
void deleteroad(AdjList *G)
{
    int i,j;
    ArcNode *pt,*pr;
    int start,end;
    system("clear");
    printf("\n\n");
    print(G);
    printlj(G);
    do{
        printf("\t请输入要删除路线的起点:");
        scanf("%d",&start);
    }while(start<=0||start>G->vexnum);
    do{
        printf("\n\t请输入要删除路线的终点:");
        scanf("%d",&end);
    }while(end<=0||end>G->vexnum);
    if(Serchroad(G,start,end)) { //如果以前有这条弧，则删除
        //在start链表后删除这个边
        pt=G->vertex[start].head;
        pr=pt;
        while(pt!=NULL){
            if(pt->adjvex==end){
                if(pt==pr){
                    G->vertex[start].head=NULL;
                    free(pt);
                    break;
                }
                else
                {
                    pr->next=pt->next;
                    free(pt);
                    break;
                }
            }
            pr=pt;
            pt=pt->next;
        }
        //在end链表上删除这个边
        pt=G->vertex[end].head;
        pr=pt;
        while(pt!=NULL){
            if(pt->adjvex==end){
                if(pt==pr){
                    G->vertex[end].head=NULL;
                    free(pt);
                    break;
                }
                else
                {
                    pr->next=pt->next;
                    free(pt);
                    break;
                }
            }
            pr=pt;
            pt=pt->next;
        }
        //边数减一
        G->arcnum--;
        printf("\n\t道路删除成功!");
    }
    else {
        printf("\n\t这条路线不存在O\n");
    }
    printf("\n\t按任意键返回...");
    getch();
}


//修改道路信息
int Modifyroad(AdjList *G)
{
    int i,j;
    ArcNode *pt,*pr;
    int start,end;
    system("clear");
    printf("\n\n");
    print(G);
    printlj(G);
    do{
        printf("\t请输入要进行修改操作的路线的起点:");
        scanf("%d",&start);
    }while(start<=0||start>G->vexnum);
    do{
        printf("\n\t请输入要进行修改操作路线的终点:");
        scanf("%d",&end);
    }while(end<=0||end>G->vexnum);
    if(Serchroad(G,start,end)) {
        pt=G->vertex[start].head;
        while(pt!=NULL)
        {
            if(pt->adjvex==end){
                int choice;
                do{
                     system("clear");
                    printf("\n\n");
                    printf("\t\t|*----------------------------------------------*|\n");
                    printf("\t\t|*               1. 路径长度                      *|\n");
                    printf("\t\t|*               2. 美丽指数                      *|\n");
                    printf("\t\t|*               3. 绿化程度                      *|\n");
                    printf("\t\t|*               0. 退出                          *|\n");
                    printf("\t\t|*-----------------------------------------------*|\n");
                    printf("\n\n");
                    do{
                        printf("\t\t请选择您要进行的修改(输入0——3):");
                        scanf("%d",&choice);
                    }while(choice<0 || choice>3);
                    getchar();
                    printf("\t\t按任意键进入下一步...");
                    getch();
                    printf("\n");
                    switch(choice)
                    {
                        case 1:
                        printf("\n\t你想把它改为多少:");
                        scanf("%d",&pt->info.weight[0]);
                        break;
                        case 2:
                        printf("\n\t你想把它改为多少:");
                        scanf("%d",&pt->info.weight[1]);
                        break;
                        case 3:
                        printf("\n\t你想把它改为多少:");
                        scanf("%d",&pt->info.weight[2]);
                        break;
                        case 0:
                        return 0;
                    }
                }while(1);
            }
            pt=pt->next;
        }
    }
    else {
        printf("\n\t这条路线不存在O\n");
    }
    return 0;
}

//添加新道路
void addroad(AdjList *G)
{
    int i,j;
    ArcNode *pt,*pr;
    int start, end, weight;
    system("clear");
    printf("\n\n");
    print(G);
    printlj(G);
    do{
        printf("\t请输入增加路线的起点序号:");
        scanf("%d",&start);
    }while(start<1||start>G->vexnum);
    do{
        printf("\n\t请输入增加路线的终点序号:");
        scanf("%d",&end);
    }while(end<1||end>G->vexnum);
    printf("\n\t请输入增加路线的长度:");
    scanf("%d",&weight);
    if(!Serchroad(G,start,end)) { //如果以前没有这条弧，则添加
        //在start链表后加上end
        ArcNode *node=(ArcNode *)malloc(sizeof(ArcNode));
        node->adjvex=end;
        node->info.weight[0]=weight;
        pt=G->vertex[start].head;
        if(pt==NULL){
            node->next=NULL;
            G->vertex[start].head=node;
        }
        else{
            while(pt->next!=NULL){
                pt=pt->next;
            }
            node->next=pt->next;
            pt->next=node;
        }

        //在end链表后加上start
        ArcNode *node1=(ArcNode *)malloc(sizeof(ArcNode));
        node1->adjvex=start;
        node1->info.weight[0]=weight;
        pt=G->vertex[end].head;
        if(pt==NULL){
            node1->next=NULL;
            G->vertex[end].head=node1;
        }
        else{
            while(pt->next!=NULL){
                pt=pt->next;
            }
            node1->next=pt->next;
            pt->next=node1;
        }
        //边数加一
        G->arcnum++;
        printf("\n\t道路添加成功!");
    }
    else {
        printf("\n\t这条路线已经存在了O不要重复添加哟!\n");
    }
    printf("\n\t按任意键返回...");
    getch();
}



//采用Dijkstra算法求得从起点景点到各个终点景点的最短路线
//disk数组用于存储最短路径权值和
void Dijkstra(AdjList *G,int start,int end,int dist[],int path[][MAXVEX],int flag)
{
    int i,j,mindist,k,t;
    ArcNode *p;
    for(i=1;i<=G->vexnum;i++){    //初始化最短路径数组disk,和标记此顶点是否已经找到最短路径的path[i][0],等于0表示没有找到,等于1表示找到.
        dist[i]=INFINITY;
        path[i][0]=0;
    }
    p=G->vertex[start].head;
    while(p){                                  //保存源点到和此源点相关的顶点的路径的权值
        dist[p->adjvex]=p->info.weight[flag];
        path[p->adjvex][1]=start;
        path[p->adjvex][2]=p->adjvex;
        path[p->adjvex][3]=0;
        p=p->next;
    }
    path[start][0]=1;               //源点标记为为1，其最短路径为0.此顶点以后不会再用到
    for(i=2;i<=G->vexnum;i++){      //选择最最短的路径
        mindist=INFINITY;
        for(j=1;j<=G->vexnum;j++){
            if(!path[j][0]&&dist[j]<mindist){//如果此顶点没有找到最短路径且权值比最大的小
                k=j;                                         
                mindist=dist[j];
            }
        }
        if(mindist==INFINITY){  //如果没有找到最短的路径，则说明从此源点不能到任何其他顶点，直接返回.
            return;
        }
        path[k][0]=1;                  //标记找到最小路径的顶点，此顶点以后不会再用到.
        p=G->vertex[k].head;
        while(p){//如果k点紧连的下一个节点没有找到最短路径且此点目前的权值比之上一个点的最短路径加它的权值还要大
            if(!path[p->adjvex][0]&& dist[p->adjvex]>dist[k]+p->info.weight[flag]){     //更新disk，使其从保持从源点到和此顶点相关的顶点的路径最短.
                dist[p->adjvex]=dist[k]+p->info.weight[flag];
                t=1;
                while(path[k][t]!=0)      //记录最新的最短路径
                {
                    path[p->adjvex][t]=path[k][t];
                    t++;
                }
                path[p->adjvex][t]=p->adjvex;
                path[p->adjvex][t+1]=0; //path[i][t+1]之前的都是最短路径所要经过的顶点，从t+1这里停止,作为最后输出路径的判断条件
            }
            p=p->next;
        }
    }
    int count=0;
    if(dist[end]!=INFINITY){
        if(flag==0){
            printf("\n\n\t%s--->%s的最短路线为:",G->vertex[start].data2.name,G->vertex[end].data2.name);
        }
        else if(flag==1){
            printf("\n\n\t%s--->%s的最美路线为:",G->vertex[start].data2.name,G->vertex[end].data2.name);
        }
        else{
            printf("\n\n\t%s--->%s的绿化最好的路径为:",G->vertex[start].data2.name,G->vertex[end].data2.name);
        }
        for(j=1;j<=G->vexnum;j++){
            if(path[end][j]==0){
               break;
            }
            count++;
            if(count==1){
                printf(" %s",G->vertex[path[end][j]].data2.name);
            }
            else{
                printf("->%s",G->vertex[path[end][j]].data2.name);
            }
        }
        printf("\n");
        // for(i=1;i<=end;i++){
        //     printf("%d ",dist[i]);
        // }
        // printf("\n");
        if(flag==0){
            printf(".距离为%dm\n",dist[end]);
        }
        else if(flag==1){
            printf(".美丽指数为%d\n",dist[end]);
        }
        else{
            printf(".绿化程度为%d\n",dist[end]);
        }
    }else{
        if(flag==0){
            printf("从%s到%s的最小路径不存在!",G->vertex[start].data2.name,G->vertex[end].data2.name);
        }
        else if(flag==1){
            printf("从%s到%s的最美路线不存在，可能正在建造中!",G->vertex[start].data2.name,G->vertex[end].data2.name);
        }
        else{
            printf("从%s到%s绿化程度好的路线不存在哟，气门慧及时改进的哟!",G->vertex[start].data2.name,G->vertex[end].data2.name);
        }
    }
    printf("\n\t按任意键返回...");
    getch();      
}


//求最优路径
void shortway(AdjList *G)
{
    print(G);
    int dist[MAXVEX];
    int path[MAXVEX][MAXVEX];
    int start,end;
    printf("\n\t请输入你要查询的路线的起点序号:");
    scanf("%d",&start);
    printf("\n\t请输入你要查询的路线的终点序号:");
    scanf("%d",&end);
    Dijkstra(G,start,end,dist,path,0);
}

//求最美路径
void beautyway(AdjList *G)
{
    print(G);
    int dist[MAXVEX];
    int path[MAXVEX][MAXVEX];
    int start,end;
    printf("\n\t请输入你要查询的路线的起点序号:");
    scanf("%d",&start);
    printf("\n\t请输入你要查询的路线的终点序号:");
    scanf("%d",&end);
    Dijkstra(G,start,end,dist,path,1);
}


//绿化最好的路径
void greenway(AdjList *G)
{
    print(G);
    int dist[MAXVEX];
    int path[MAXVEX][MAXVEX];
    int start,end;
    printf("\n\t请输入你要查询的路线的起点序号:");
    scanf("%d",&start);
    printf("\n\t请输入你要查询的路线的终点序号:");
    scanf("%d",&end);
    Dijkstra(G,start,end,dist,path,2);
}


//校园平面简图
void pingmiantu(AdjList *G)
{
    int i,j,k;
    Data locate[30][70];
    for(i=0;i<30;i++){
        for(j=0;j<70;j++){
            memset(locate[i][j].name,0,sizeof(locate[i][j].name));
        }
    }
    for(i=1;i<=G->vexnum;i++){
        for(j=0;j<30;j++){
            for(k=0;k<70;k++)
            {
                if(G->vertex[i].data2.x==j+1&&G->vertex[i].data2.y==k+1){
                    strcpy(locate[j][k].name,G->vertex[i].data2.name);
                }
            }
        }
    }
    printf("\n-------------------------------------西邮平面简图----------------------------------------------------\n\n");
    for(i=0;i<30;i++){
        for(j=0;j<70;j++){
            printf(" %s",locate[i][j].name);
        }
        printf("\n");
    }
    printf("                                                                                                ^　南\n");
    printf("                                                                                                |\n");
    printf("                                                                                             ---|---→西\n");
    printf("                                                                                                |\n");
    printf("-----------------------------------------------------------------------------------------------------\n\n");
    printf("\t\t按任意键进入下一步...");
    getch();
}

//游客登录
int signyou()
{
    int choice;
    AdjList *G;
    if ((G=(AdjList*)malloc(sizeof(AdjList))) == NULL )
        return 0;
    memset(G, 0, sizeof(AdjList));
    Create(G);
    do{
        system("clear");
        printf("\n\n");
	    printf("\t\t|===============================================\n");
	    printf("\t\t*                                              *\n");
	    printf("\t\t*          ******coco西邮导航系统*******         *\n");
	    printf("\t\t*                                              *\n");
	    printf("\t\t*----------------------------------------------*\n");
	    printf("\t\t*                                              *\n");
	    printf("\t\t*               1. 校园平面简图                  *\n");
	    printf("\t\t*               2. 查看景点信息                  *\n");
	    printf("\t\t*               3. 查询最优路线                  *\n");
        printf("\t\t*               4. 查询最美路线                  *\n");
        printf("\t\t*               5. 查询绿化最好路                 *\n");
	    printf("\t\t*               0. 退出                         *\n");
	    printf("\t\t*                                              *\n");
        printf("\n\n");
        do{
            printf("\t\t请选择您要进行的操作(输入0——5):");
            scanf("%d",&choice);
        }while(choice<0 || choice>5);
        getchar();
        printf("\t\t按任意键进入下一步...");
        getch();
        printf("\n");
        switch(choice)
        {
            case 1: pingmiantu(G);break;
            case 2: view(G); break;
            case 3: shortway(G);break;
            case 4: beautyway(G);break;
            case 5: greenway(G);break;
            case 0:return 0;
        }
    }while(1);
}



//管理员菜单
void Admin()
{
    int choice;
    AdjList *G;
    G=(AdjList*)malloc(sizeof(AdjList));
    memset(G, 0, sizeof(AdjList));
    Create(G);
    do{
        system("clear");
        printf("\n\n");
	    printf("\t\t=====================================================\n");
	    printf("\t\t*                                                   *\n");
	    printf("\t\t*              ******coco西邮导航系统*******          *\n");
	    printf("\t\t*                                                   *\n");
	    printf("\t\t*---------------------------------------------------*\n");
	    printf("\t\t*                                                   *\n");
	    printf("\t\t*                     1. 校园平面简图                 *\n");
	    printf("\t\t*                     2. 添加新景点                   *\n");
        printf("\t\t*                     3. 修改校园景点                 *\n");
        printf("\t\t*                     4. 删除校园景点                 *\n");
	    printf("\t\t*                     5. 添加新道路                   *\n");
        printf("\t\t*                     6. 修改道路信息                 *\n");
	    printf("\t\t*                     7. 删除某路线                   *\n");
	    printf("\t\t*                     0. 退出                        *\n");
        printf("\n\n");
        do{
            printf("\t请选择您要进行的操作(输入0——7):");
            scanf("%d",&choice);
        }while(choice<0 || choice>7);
        getchar();
        printf("\t\t按任意键进入下一步...");
        getch();
        printf("\n");
        switch(choice)
        {
            case 1 :pingmiantu(G); break;
            case 2 :addpoint(G);break;
            case 3 :Modifypoint(G); printf("\n\t修改成功!\n"); sleep(1); break;
            case 4 :deletepoint(G); sleep(1); break;
            case 5 :addroad(G); sleep(1);break;
            case 6 :Modifyroad(G);printf("\n\t修改成功!\n"); sleep(1); break; 
            case 7 :deleteroad(G); sleep(1);break;
            case 0 :break;
        }
    }while(choice!=0);
    int i,j;
    ArcNode *pt,*pr;
    FILE *fp1=fopen("tudingdian.txt","wt");
    if(fp1==NULL){
        printf("文件tudingdian.txt未打开!");
    }
    FILE *fp2=fopen("tubian.txt","wt");
    if(fp2==NULL){
        printf("文件tubian.txt未打开!");
    }
    fprintf(fp2,"%d %d\n",G->vexnum,G->arcnum);
    for(i = 1; i <= G->arcnum; i++){
        pt=G->vertex[i].head;
        while(pt!=NULL){
            if(pt->adjvex>i){
                fprintf(fp2,"%d %d %d %d %d\n",i,pt->adjvex,pt->info.weight[0],pt->info.weight[1],pt->info.weight[2]);
            }
            pt=pt->next;
        }
    }
    fclose(fp2);
    for(i=1;i<=G->vexnum;i++){
        fprintf(fp1,"%s %d %d %s\n", G->vertex[i].data2.name, G->vertex[i].data2.x, G->vertex[i].data2.y,G->vertex[i].data2.info);
    }
    fclose(fp1);
    printf("\n\t\tstore success!\n");
    printf("\n\t\t按任意键进入下一步...");
    getch();
}


//md5加密并且与密文对比
int Md5(char a[],FILE *fp, int n)
{
    int i;
    char name[20];
	unsigned char encrypt[30];
    memcpy(encrypt,a,n);
	unsigned char decrypt[16];
	MD5_CTX md5;
	MD5Init(&md5);         		
	MD5Update(&md5,encrypt,strlen((char *)encrypt));
	MD5Final(&md5,decrypt);
    FILE *fp1=fopen("test.txt","w");
	for(i=0;i<16;i++)
	{
		fprintf(fp1,"%02x",decrypt[i]);
	}
    fclose(fp1);
    char mima[40],mima1[40];
    fscanf(fp,"%s",mima);
    FILE *fp2 = fopen("test.txt","r");
    fscanf(fp2,"%s",mima1);
    fclose(fp2);
    if(strcmp(mima,mima1)==0){
        return 1;
    }
	return 0;
}


//管理员登录
void signadmin()
{
    char name[30]={0},name1[30];
    char mima1[30];
    int i,n,count=0;
    char ch;
    FILE *fp = fopen("admin.txt","r");
    system("clear");
    printf("\n\n");
    printf("\t\t|===============***西邮导航系统***==============|\n");
    printf("\n");
    fscanf(fp,"%s ",name);
    getchar();  //吸收回车
    for(i=0;i<3;i++){
        printf("\n\t\t亲,请输入名称:");
        scanf("%s",name1);
        getchar();
        printf("\n\t\t请输入密码:");
        input_mima(mima1);
        if((strcmp(name,name1)==0)&&Md5(mima1,fp,sizeof(mima1))) {
            printf("\n\n\t\t登录成功!按任意键进入主菜单.");
            getch();
            Admin();
            break;
        }
        else{
            count++;
            printf("\n\t\t对不起输入错误，请重新输入!");
        }
        if(count==3) {
            printf("\n\t\t您已连续3次用户名或密码输入有误,登录失败!系统将退出.\n");
            usleep(3000);  //休眠函数，短时间暂停
            exit(0);
        }
    }
    fclose(fp);
}

//登录菜单
void login_menu()
{
    int a;
    do
    {
        system("clear");
        printf("\t\033[35m                                         \033[0m\n");
        printf("\033[35m\t          欢迎使用coco西邮导航系统          \033[0m\n");
        printf("\t\033[35m                                  V6.6.6 \033[0m\n");
        printf("\n");     
        printf("\t\033[35m                                         \033[0m\n");
        printf("\033[35m\t                西安邮电大学               \033[0m\n");
        printf("\t\033[35m                                         \033[0m\n");
        printf("\033[35m\t               <1>游客登录                 \033[0m\n");
        printf("\t\033[35m                                          \033[0m\n");
        printf("\033[35m\t               <2>管理员登录                \033[0m\n");
        printf("\t\033[35m                                          \033[0m\n");
        printf("\033[35m\t               <0>退出                     \033[0m\n");
        printf("\t\033[35m                                          \033[0m\n");
        setbuf(stdin,NULL);
        do{
            printf("\n\t请输入你的选择:");
            scanf("%d",&a);
        }while(a>2||a<0);
        switch(a)
        {
            case 1:
                {
                    signyou();
                    sleep(1);
                    break;
                }
            case 2:
                {
                    signadmin();
                    sleep(2);
                    break;
                }
            case 0:
                {
                    break;
                }
        }
    }while(a!=0);
}

int main()
{
    login_menu();
}
