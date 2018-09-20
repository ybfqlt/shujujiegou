#include<stdio.h>
#include<stdlib.h>
#define MAX 100

typedef struct NodeType
{
    int id;
    int password;
    struct NodeType *next;
}nodetype;

nodetype *phead=NULL;

void crealist(int n);
void printlist();
int isempty();
void josephus(int ipassword);

int main(void)
{
    int n=0;
    int m=0;
    do
    {
        if(n>MAX)
        {
            printf("人数过多，请重新输入!\n");
        }
        printf("请输入人数n(最多%d个):",MAX);
        scanf("%d",&n);
    }while(n>MAX);
    printf("请输入初始密码m:");
    scanf("%d",&m);
    crealist(n);
    printf("\n-----循环链表----------\n");
    printlist();
    printf("\n-----出队情况----------\n");
    josephus(m);
    return 1;
}

//创建链表
void crealist(int n)
{
    int i=0;
    int ipassword=0;
    nodetype *pnew=NULL;
    nodetype *pcur=NULL;
    if(n==0)
    {
        return;
    }
    for(i=1;i<=n;i++)
    {
        printf("输入第%d个人的密码:",i);
        scanf("%d",&ipassword);
        pnew=(nodetype *)malloc(sizeof(nodetype));
        if(!pnew)
        {
            printf("分配失败!");
            exit(-1);
        }
        pnew->id=i;
        pnew->password=ipassword;
        pnew->next=NULL;
        if(phead==NULL)
        {
            phead=pcur=pnew;
            pcur->next=phead;
        }
        else
        {
            pnew->next=pcur->next;
            pcur->next=pnew;
            pcur=pnew;
        }
    }
}

//打印链表
void printlist()
{
    nodetype *pcur=phead;
    if(!isempty())
    {
        printf("  id     password  \n");
        do
        {
            printf("%3d %7d\n",pcur->id,pcur->password);
            pcur=pcur->next;
        }while(pcur!=phead);
    }
}

//判断链表是否为空
int isempty()
{
    if(!phead)
    {
        printf("链表为空!\n");
        return 1;
    }
    return 0;
}

void josephus(int ipassword)
{
    int i=0;
    int iflag=1;
    nodetype *pprv=NULL;
    nodetype *pcur=NULL;
    nodetype *pdel=NULL;
    pprv=pcur=phead;
    if(isempty())
    {
        printf("\n队中无人!\n");
        return ;
    }
    while(pprv->next!=phead)
        pprv=pprv->next;
    while(iflag)
    {
        for(i=1;i<ipassword;i++)
        {
            pprv=pcur;
            pcur=pcur->next;
        }
        if(pprv==pcur)
            iflag=0;
        pdel=pcur;
        pprv->next=pcur->next;
        pcur=pcur->next;
        ipassword=pdel->password;
        printf("第%d个人出列\t密码:%d\n",pdel->id,pdel->password);
        free(pdel);
    }
    phead=NULL;
    getchar();
}

