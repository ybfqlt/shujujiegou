#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAXOP 10
#define MAXSIZE 100

typedef struct 
{
    char ch; //运算符
    int pri;
}Opri;

typedef struct
{
    char data[MAXSIZE];   //存放运算符
    int top;
}Seqstack;

typedef struct
{
    float data[MAXSIZE];
    int top;
}Seq;

Opri lpri[]= {{'#',0},{'(',1},{'*',5},{'/',5},{'+',3},{'-',3},{')',6}};
Opri rpri[]= {{'#',0},{'(',6},{'*',4},{'/',4},{'+',2},{'-',2},{')',1}};

//初始化运算符栈
Seqstack *IntiStack()
{
    Seqstack *s;
    s=malloc(sizeof(Seqstack));
    s->top = -1;
    return s;
}

Seq *Init()
{
    Seq *s;
    s=malloc(sizeof(Seq));
    s->top=-1;
    return s;
}

//判运算符栈
int Emptyy(Seqstack *s)
{
    if(s->top==-1)
        return 1;
    else
        return 0;
}

//判栈操作数栈
int Emptyc(Seq *s)
{
    if(s->top==-1)
        return 1;
    else
        return 0;
}

//求左运算符的优先级
int leftpri(char op)
{
    int i;
    for(i=0;i<MAXOP;i++)
    {
        if(lpri[i].ch==op)
            return lpri[i].pri;
    }
}


//求右运算符的优先级
int rightpri(char op)
{
    int i;
    for(i=0;i<MAXOP;i++)
    {
        if(rpri[i].ch==op)
            return rpri[i].pri;
    }
}


//操作数入栈
int pushc(Seq *s,float x)
{
    if(s->top == MAXSIZE-1)
        return 0;
    else
    {
        s->top++;
        s->data[s->top]=x;
        return 1;
    }
}

//操作数出栈
int popc(Seq *s,float *x)
{
    if(Emptyc(s))
        return 0;
    else
    {
        *x = s->data[s->top];
        s->top--;
        return 1;
    }
}

//取栈顶元素
float Gettopc(Seq *s)
{
    if(Emptyc(s))
        return 0;
    else
        return (s->data[s->top]);
}

//运算符入栈
int pushy(Seqstack *s,char x)
{
    if(s->top == MAXSIZE-1)
        return 0;
    else
    {
        s->top++;
        s->data[s->top]=x;
        return 1;
    }
}

//运算符出栈
int popy(Seqstack *s,char *x)
{
    if(Emptyy(s))
        return 0;
    else
    {
        *x = s->data[s->top];
        s->top--;
        return 1;
    }
}

//取栈顶元素
char Gettopy(Seqstack *s)
{
    if(Emptyy(s))
        return 0;
    else
        return (s->data[s->top]);
}

//判断ch是否为运算符
int In(char ch,char opset[])
{
    int i;
    int a=strlen(opset);
    for(i=0;i<a;i++)
    {
        if(ch==opset[i])
        {
            return 1;
        }
    }
    return 0;
}

char compare(char op1,char op2)
{
    if(leftpri(op1)==rightpri(op2))
        return '=';
    else if(leftpri(op1)<rightpri(op2))
        return '<';
    else
        return '>';
}


float jisuan(float a,char p,float b)
{
    float t;
    switch(p)
    {
        case '+': t=a+b;
                  break;
        case '-': t=a-b;
                  break;
        case '/': t=b/a;
                  break;
        case '*': t=a*b;
                  break;
        default:printf("error");
    }
    return t;
}

float Exp()
{
    int h=0;
    char op,x,ch;
    float a,b;
    Seq*OPRD;
    Seqstack*OPTR;
    float data,val;
    OPRD = Init();  //操作数栈
    OPTR = IntiStack();  //运算数栈
    char opset[10]={'(',')','+','-','*','/','#'};
    pushy(OPTR,'#');
    printf("\n请输入表达式(以#结尾哦!):");
    ch=getchar();
    while(ch!='#'||Gettopy(OPTR)!='#')
    {
        if(!In(ch,opset))
        {
            data=ch-'0';
            ch=getchar();
            while(!In(ch,opset))
            {
                data=data*10+ch-'0';
                ch=getchar();
            }
            pushc(OPRD,data);
        }
        else
        {
            switch(compare(Gettopy(OPTR),ch))
            {
                case '<':pushy(OPTR,ch);
                         ch=getchar();
                         break;
                case '=':popy(OPTR,&x);
                          ch=getchar();
                         break;
                case '>':popy(OPTR,&op);
                         popc(OPRD,&a);
                         popc(OPRD,&b);
                         val=jisuan(a,op,b);
                         pushc(OPRD,val);
                         break;
            }
        }
    }
    val=Gettopc(OPRD);
    return val;
}

int main()
{
    float result=Exp();
    printf("最终结果为: %.2f\n\n",result);
}
