/*************************************************************************
    > File Name: tubianli.c
    > Author: 
    > Mail: 
    > Created Time: 2018年11月20日 星期二 17时47分20秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//队列类型定义
typedef int datatype;
typedef char Vextype;
#define MAXSIZE 100
#define MAXVEX 20
#define MAXNUM 100

typedef char Vextype;
typedef struct
{
    datatype data[MAXSIZE];
    int rear,front;
}SeQueue;
//邻接矩阵的数据类型描述
typedef struct
{
    int arcs[MAXVEX][MAXVEX]; //边(或弧)信息 
    Vextype vex[MAXVEX];      //顶点信息
    int vexnum;               //顶点数目
    int arcnum;               //边或弧的数目
}AdjMatrix;

int visited[MAXNUM];
int visited2[MAXNUM];

//队列初始化
void Init(SeQueue *q)
{
    q=(SeQueue *)malloc(sizeof(SeQueue));
    q->front=q->rear=MAXSIZE-1;
}
//入队
int InSeQueue(SeQueue *q,datatype x)
{
    if((q->rear+1)%MAXSIZE==q->front)
    {
        printf("队满");
        return 0;
    }
    else
    {
        q->rear=(q->rear+1)%MAXSIZE;
        q->data[q->rear]=x;
        return 1;
    }
}

//出队
int OutSeQueue(SeQueue *q,datatype *x)
{
    if(q->front==q->rear)
    {
        printf("队空");
        return 0;
    }
    else
    {
        q->front=(q->front+1)%MAXSIZE;
        *x=q->data[q->front];
        return 0;
    }
}

//判空队
int Empty(SeQueue *q)
{
    if(q->front==q->rear)
        return 1;
    else
        return 0;
}

int LocateVex(AdjMatrix *G,char v)
{
    int i;
    for(i=0;i<G->vexnum;i++)
      if(G->vex[i]==v)
         return i;
    return 0;
}
//用邻接矩阵创建有向图
void Create(AdjMatrix *G)
{
    int i,j,k;
    char v1,v2;
    printf("请输入无向网中的顶点数和边数:");
    scanf("%d,%d",&G->vexnum,&G->arcnum);
    for(i=0;i<G->vexnum;i++)
       for(j=0;j<G->vexnum;j++)
          G->arcs[i][j] = 0;
    printf("请输入无向网中%d个顶点:\n",G->vexnum);
    getchar();
    for(i=0;i<G->vexnum;i++)
    {
        printf("No.%d个顶点:顶点V",i+1);
        scanf("%c",&G->vex[i]);
        getchar();
    }
    printf("请输入无向网中%d条边:\n",G->arcnum);
    for(k=0;k<G->arcnum;k++)
    {
        printf("请输入第%d条边的两个顶点(v1,v2):\n",k+1);
        scanf("%c,%c",&v1,&v2);
        getchar();
        i=LocateVex(G,v1);
        j=LocateVex(G,v2);
        G->arcs[i][j]=1;
    }
}
//打印矩阵存储的结果
void print(AdjMatrix g)
{
    for(int i=0;i<g.vexnum;i++)
    {
        for(int j=0;j<g.vexnum;j++)
        {
            printf("%d ",g.arcs[i][j]);
        }
        printf("\n");
    }
}
//找图g中v0的第一个邻接点
int FirstAdjVex(AdjMatrix g,int v0)
{
    int i;
    for(i=0;i<g.vexnum;i++)
    {
        if(g.arcs[v0][i]==1)
        {
            return i;
        }
    }
    //if(i==(g.vexnum-1))
        //return -1;
    return -1;
}

//找图g中顶点v0的下一个邻接点
int NextAdjVex(AdjMatrix g,int v0,int w)
{
    int i;
    for(i=w+1;i<g.vexnum;i++)
    {
        if(g.arcs[v0][i]==1)
        {
            return i;
        }
    }
    //if(i==(g.vexnum-1))
        //return -1;
    return -1;
}
//递归深度优先搜索遍历连通子图
void DFS(AdjMatrix g,int v0)
{
    int w;
    printf("%c",g.vex[v0]);
    visited[v0]=1;
    w=FirstAdjVex(g,v0);
    while(w!=-1)
    {
        if(!visited[w])
        {
            DFS(g,w);
        }
        w=NextAdjVex(g,v0,w);
    }
}
void TraverseG(AdjMatrix g)
{
    int v;
    for(v=0;v<g.vexnum;v++)
        visited[v]=0;
    for(v=0;v<g.vexnum;v++)
        if(!visited[v])
        {
            DFS(g,v);
        }
    printf("\n");
}

//广度优先搜索遍历连通子图
void BFS(AdjMatrix g,int v0)
{
    SeQueue Q;
    int w,v;
    printf("%c",g.vex[v0]);
    visited2[v0]=1;
    Init(&Q);
    InSeQueue(&Q,v0);
    while(!Empty(&Q))
    {
        OutSeQueue(&Q,&v);
        w=FirstAdjVex(g,v);
        while(w!=-1)
        {
            if(!visited2[w])
            {
                printf("%c",g.vex[w]);
                visited2[w]=1;
                InSeQueue(&Q,w);
            }
            w=NextAdjVex(g,v,w);
        }
    }
}

void TraverseG2(AdjMatrix g)
{
    int v;
    for(v=0;v<g.vexnum;v++)
        visited2[v]=0;
    for(v=0;v<g.vexnum;v++)
        if(!visited2[v])
            BFS(g,v);
    printf("\n");
}

//每个节点的出度和入度和度
void degree(AdjMatrix g)
{
    int out[g.vexnum];
    int in[g.vexnum];
    int sumd[g.vexnum];
    memset(in,0,sizeof(in));
    memset(out,0,sizeof(out));
    memset(sumd,0,sizeof(sumd));
    int  i,j;
    for(i=0;i<g.vexnum;i++)
    {
        for(j=0;j<g.vexnum;j++)
        {
            if(g.arcs[i][j]==1)
                out[i]++;
            if(g.arcs[j][i]==1)
                in[i]++;
        }
    }
    for(j=0;j<g.vexnum;j++)
    {
        sumd[j]=in[j]+out[j];
    }
    printf("\n顶点 出度 入度 度\n");
    for(i=0;i<g.vexnum;i++)
    {
        printf("%c    %d    %d    %d\n",g.vex[i],out[i],in[i],sumd[i]);
    }
} 


int main()
{
    AdjMatrix *G;
    G=(AdjMatrix *)malloc(sizeof(AdjMatrix));
    printf("开始创建无向网...\n");
    Create(G);
    printf("\n存储矩阵如下:\n");
    print(*G);
    printf("创建成功\n");
    printf("\n以下为图中每个节点的出度，入度，度:");
    degree(*G);
    printf("\n递归深度优先搜索遍历结果:");
    TraverseG(*G);
    printf("广度优先搜索遍历图结果为:");
    TraverseG2(*G);
}