/*************************************************************************
    > File Name: erchashu.c
    > Author: 
    > Mail: 
    > Created Time: 2018年10月30日 星期二 17时37分28秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#define MAXSIZE 100

typedef struct Node
{
    char data;
    struct Node *Lchild;
    struct Node *Rchild;
}BiTNode,*BiTree;

typedef struct stack
{
    BiTree data[MAXSIZE];
    int top;
}Seqstack;

int count=0;

//初始化栈
Seqstack *InitStack()
{
    Seqstack *s;
    s=(Seqstack *)malloc(sizeof(Seqstack));
    s->top=-1;
    return s;
}


//判断栈是否为空
int IsEmpty(Seqstack *s)
{
    if(s->top == -1) 
        return 1;
    else
        return 0;
}


//入栈
int Push(Seqstack *s,BiTree root)
{
    if(s->top == MAXSIZE-1)
        return 0;
    else
    {
        s->top++;
        s->data[s->top]=root;
        return 1;
    }
}

//出栈
int Pop(Seqstack *s,BiTree *root)
{
    if(IsEmpty(s))
        return 0;
    else
    {
        *root=s->data[s->top];
        s->top--;
    }
    return 1;
}

//获取栈顶元素
int Top(Seqstack*s,BiTree *root)
{
    if(IsEmpty(s))
        return 0;
    else
    {
        *root=s->data[s->top];
    }
    return 1;
}


//先序创建二叉树
void creat(BiTree *root)
{
    char ch;
    ch=getchar();
    if(ch=='#')
        *root=NULL;
    else
    {
        *root=(BiTree)malloc(sizeof(BiTNode));
        (*root)->data=ch;
        creat(&((*root)->Lchild));
        creat(&((*root)->Rchild));
    }

}


//求二叉树的高度即层次
int Treedepth(BiTree root)
{
    int h1,hr,h;
    if(root==NULL)
        return 0;
    else
    {
        h1=Treedepth(root->Lchild);
        hr=Treedepth(root->Rchild);
        h=(h1>hr?h1:hr)+1;
        return h;
    }
}

//按树状打印二叉树
void PrintTree(BiTree root,int h)
{
    if(root==NULL)
        return;
    PrintTree(root->Rchild,h+1);
    for(int i=0;i<h;i++)
        printf(" ");
    printf("%c\n",root->data);
    PrintTree(root->Lchild,h+1);
}

//中序遍历输出二叉树中叶子结点的个数
void InOrderleaf(BiTree root)
{
    if(root)
    {
        InOrderleaf(root->Lchild);
        if(root->Lchild==NULL&&root->Rchild==NULL)
        {
            count++;
        }
        InOrderleaf(root->Rchild);
    }
}



//先序递归遍历二叉树
void PreOrder(BiTree root)
{
    if(root)
    {
        printf("%c",root->data);
        PreOrder(root->Lchild);
        PreOrder(root->Rchild);
    }
}

//中序非递归遍历二叉树
void InOrder(BiTree root)
{
    Seqstack *s;
    BiTree p;
    s=InitStack();
    p=root;
    while(p!=NULL||!IsEmpty(s))
    {
        while(p!=NULL)
        {
            Push(s,p);
            p=p->Lchild;
        }
        if(!IsEmpty(s))
        {
            Pop(s,&p);
            printf("%c",p->data);
            p=p->Rchild;
        }
    }
}

//后序非递归遍历二叉树
void PostOrder(BiTree root)
{
    Seqstack *s;
    BiTree p,q;
    s=InitStack();
    p=root;
    q=NULL;
    while(p!=NULL||!IsEmpty(s))
    {
        while(p!=NULL)
        {
            Push(s,p);
            p=p->Lchild;
        }
        if(!IsEmpty(s))
        {
            Top(s,&p);
            if((p->Rchild==NULL)||(p->Rchild==q))
            {
                Pop(s,&p);
                printf("%c",p->data);
                q=p;
                p=NULL;
            }
            else
                p=p->Rchild;
        }
    }
}

int main()
{
    BiTree root;
    printf("请输入先序序列，创建二叉树:\n");
    creat(&root);
    printf("所建树如下:\n");
    PrintTree(root,1);
    InOrderleaf(root);
    printf("树的高度为: %d\n",Treedepth(root));
    printf("树中叶子结点数目为: %d\n",count);
    printf("先序遍历序列(递归)如下:\n");
    PreOrder(root);
    printf("\n中序遍历序列如下(非递归):\n");
    InOrder(root);
    printf("\n后序遍历序列(非递归)如下:\n");
    PostOrder(root);
    printf("\n");
}
