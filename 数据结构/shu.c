/*************************************************************************
    > File Name: shu.c
    > Author: 
    > Mail: 
    > Created Time: 2018年10月28日 星期日 16时35分03秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define N 100

typedef struct BiTNode
{
    char data;
    struct BiTNode *lchild;
    struct BiTNode *rchild;
}BiTNode,*BiTree;

//先序遍历
void preorder(BiTNode *root)
{
    if(root)
    {
        printf("%c ",root->data);
        preorder(root->lchild);
        preorder(root->rchild);
    }
}


//根据先序遍历和中序遍历创建二叉树
BiTNode *createtree(char *pre,char *in,int n)
{
    int i=0;
    int n1=0,n2=0;
    int m1=0,m2=0;
    BiTNode *node=NULL;
    char lpre[N],rpre[N];//存放左右子树的先序序列
    char lin[N],rin[N];//存放左右子树的中序序列
    node=(BiTNode *)malloc(sizeof(BiTNode));
    if(node==NULL)
    {
        return NULL;
    }
    memset(node,0,sizeof(BiTNode));
    if(n==0)
    {
        return NULL;
    }
    node->data=pre[0];
    for(i=0;i<n;i++)
    {
        if((i<=n1)&&(in[i]!=pre[0]))
        {
            lin[n1++]=in[i];
        }
        else if(in[i]!=pre[0])
        {
            rin[n2++]=in[i];
        }
    }
    for(i=1;i<n;i++)
    {
        if(i<=n1)
        {
            lpre[m1++]=pre[i];
        }
        else
        {
            rpre[m2++]=pre[i];
        }
    }
    node->lchild=createtree(lpre,lin,n1);
    node->rchild=createtree(rpre,rin,n2);
    return node;
}

//求二叉树的高度
int Depth(BiTree root)
{
    int h1,hr,h;
    if(root==NULL) return 0;
    else
    {
        h1=Depth(root->lchild);
        hr=Depth(root->rchild);
        h=(h1>hr?h1:hr)+1;
        return h;
    }
}


//按树状打印树
void PrintTree(BiTree root,int h)
{
    if(root==NULL) return;
    PrintTree(root->rchild,h+1);
    for(int i=0;i<h;i++) 
        printf(" ");
    printf("%c\n",root->data);
    PrintTree(root->lchild,h+1);
}


int main()
{
    char pre[N];
    char in[N];
    int n=0;
    char ch;
    BiTNode *root=NULL;
    printf("请输入先序序列\n");
    while((ch=getchar())&&ch!='\n')
        pre[n++]=ch;
    printf("请输入中序序列\n");
    n=0;
    while((ch=getchar())&&ch!='\n')
        in[n++]=ch;
    root=createtree(pre,in,n);
    int h = Depth(root);
    printf("创建树如下:\n");
    PrintTree(root,h);
    printf("先序序列:\n");
    preorder(root);
    printf("\n");
    return 0;
}

