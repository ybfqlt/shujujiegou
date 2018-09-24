#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct polynomial
{
    float coef;        //系数
    int expn;           //指数
    struct polynomial *next;  //指向下一个节点的指针
}polynomial,*polyn;

//建立多项式
polyn createpoly()
{
    polynomial duo[50],temp;
    int flag;
    int i=0;
    polynomial *head,*rear,*s;
    int c,e;
    head=(polynomial *)malloc(sizeof(polynomial));
    rear=head;
    scanf("%d,%d",&c,&e);
    while(c!=0)
    {
        memset(&duo[i],0,sizeof(polynomial));
        duo[i].coef=c;
        duo[i].expn=e;
        i++;
        scanf("%d,%d",&c,&e);
    }
    for(int t=0;t<i-1;t++)
    {
        flag=0;
        for(int k=0;k<i-1-t;k++)
        {
            flag=1;
            if(duo[k].expn>duo[k+1].expn)
            {
                temp=duo[k+1];
                duo[k+1]=duo[k];
                duo[k]=temp;
            }
        }
        if(flag==0)
        {
            break;
        }
    }
    for(int j=0;j<i;j++)
    {
        s=(polynomial *)malloc(sizeof(polynomial));
        s->coef=duo[j].coef;
        s->expn=duo[j].expn;
        rear->next=s;
        rear=s;
    }
    rear->next=NULL;
    return(head);
}

//输出多项式
void  printpolyn(polyn p)
{
    polyn q=p->next;
    int flag=1;
    if(!q)
    {
        putchar('0');
        printf("\n");
        return;
    }
    while(q)
    {
        if(q->coef>0&&flag!=1)  putchar('+');
        if(q->coef!=1&&q->coef!=-1)
        {
            printf("%g",q->coef);
            if(q->expn==1)  putchar('X');
            else if(q->expn) printf("X^%d",q->expn);
        }
        else
        {
            if(q->coef==1)
            {
                if(!q->expn) putchar('1');
                else if(q->expn==1) putchar('X');
                else  printf("X^%d",q->expn);
            }
            if(q->coef==-1)
            {
                if(!q->expn) printf("-1");
                else if(q->expn) printf("-X");
                else printf("-X^%d",q->expn);
            } 
        }
        q=q->next;
        flag++;
    }
    printf("\n");
}

//多项式求导
polyn dao(polyn pc)
{
    polyn pd,pr=pc;
    polyn temp=pc->next;
    while(temp!=NULL)
    {
        temp->coef=temp->coef*temp->expn;
        temp->expn=temp->expn-1;
        if(temp->expn!=0)
        {
           temp=temp->next;
           pr=pr->next;
        }
        else
        {
            pd=temp;
            pr->next=temp->next;
            temp=temp->next;
            free(pd);
        }
    }
    return pc;
}



//两个多项式相加
polyn Addpolyn(polyn pa,polyn pb)
{
    polyn qa=pa->next;
    polyn qb=pb->next;
    polyn headc,pc,qc;
    pc=(polyn)malloc(sizeof(struct polynomial));
    pc->next=NULL;
    headc=pc;
    while(qa!=NULL&&qb!=NULL)
    {
        qc=(polyn)malloc(sizeof(struct polynomial));
        if(qa->expn<qb->expn)
        {
            qc->coef=qa->coef;
            qc->expn=qa->expn;
            qa=qa->next;
        }
        else if(qa->expn==qb->expn)
        {
            qc->coef=qa->coef+qb->coef;
            qc->expn=qa->expn;
            qa=qa->next;
            qb=qb->next;
        }
        else
        {
            qc->coef=qb->coef;
            qc->expn=qb->expn;
            qb=qb->next;
        }
        if(qc->coef!=0)
        {
            qc->next=pc->next;
            pc->next=qc;
            pc=qc;
        }
        else free(qc);
    }
    while(qa!=NULL)
    {
        qc=(polyn)malloc(sizeof(struct polynomial));
        qc->coef=qa->coef;
        qc->expn=qa->expn;
        qa=qa->next;
        qc->next=pc->next;
        pc->next=qc;
        pc=qc;
    }
    while(qb!=NULL)
    {
        qc=(polyn)malloc(sizeof(struct polynomial));
        qc->coef=qb->coef;
        qc->expn=qb->expn;
        qb=qb->next;
        qc->next=pc->next;
        pc->next=qc;
        pc=qc;
    }
    return headc;
}

//两个多项式相乘
polyn cheng(polyn pa,polyn pb)
{
    polyn headc,pd,head;
    polyn qc=(polyn)malloc(sizeof(struct polynomial));
    qc->next=NULL;
    polyn qa=pa->next;
    polyn qb=pb->next;
    while(qa!=NULL)
    {
        polyn pc=(polyn)malloc(sizeof(struct polynomial));
        pc->next=NULL;
        headc=pc;
        while(qb!=NULL)
        {
            pd=(polyn)malloc(sizeof(struct polynomial));
            pd->coef=qa->coef*qb->coef;
            pd->expn=qa->expn+qb->expn;
            if(pd->coef!=0)
            {
                pd->next=pc->next;
                pc->next=pd;
                pc=pd;
            }
            else
            {
                free(pd);
            }
            qb=qb->next;
        }
        qa=qa->next;
        head=Addpolyn(headc,qc);
        qc=head;
        free(pc);
        qb=pb->next;
    }
    return head;
}


//两个多项式相减
polyn subtractpolyn(polyn pa,polyn pb)
{
    polyn h=pb;
    polyn p=pb->next;
    polyn pd;
    while(p)
    {
        p->coef*=-1;
        p=p->next;
    }
    pd=Addpolyn(pa,h);
    for(p=h->next;p;p=p->next)
        p->coef*=-1;
    return pd;
}
void printselect()
{
    printf("^~^**************************************^~^\n");
    printf("*          a:显示多项式1                   *\n");
    printf("*          b:显示多项式2                   *\n");
    printf("*          g:多项式之和                    *\n");
    printf("*          h:多项式之差                    *\n");
    printf("           i:多项式相乘                    *\n");
    printf("*          j:多项式求导                    *\n");
    printf("*                                          *\n");
    printf("*c:继续计算下一个                q:退出计算*\n");
    printf("^~^**************************************^~^\n");
}

int main()
{
    char choice;
    polynomial *pa,*pb,*headpc,*pd;
    printf("welcome to polynomial of one indeterminate calculator\n" );
    do
    {
        printf("please input polynomial of one indeterninate pa<like 1,1 and 0 is over>:\n");
        pa=createpoly();
        printf("please input polynomial of one indeterninate pb<like 1,1 and 0 is over>:\n");
        pb=createpoly();
        printselect();
        do
        {
            printf("please select your operation:");
            scanf("%c",&choice);
            switch(choice)
            {
                case 'a':
                    {
                        printf("\n\tpa=");
                        printpolyn(pa);
                        printf("\n");
                        break;
                    }
                case 'b':
                    {
                        printf("\n\tpb=");
                        printpolyn(pb);
                        printf("\n");
                        break;
                    }
                case 'g':
                    {
                        headpc=Addpolyn(pa,pb);
                        printf("\n\tpa+pb=");
                        printpolyn(headpc);
                        printf("\n");
                        break;
                    }
                case 'h':
                    {
                        pd=subtractpolyn(pa,pb);
                        printf("\n\tpa-pb=");
                        printpolyn(pd);
                        printf("\n");
                        break;
                    }
                case 'i':
                    {
                        polyn headcheng;
                        headcheng=cheng(pa,pb);
                        printf("\n\tpa*pb=");
                        printpolyn(headcheng);
                        printf("\n");
                        break;
                    }
                case 'j':
                    {
                        int m;
                        polyn headdao;
                        printf("您要对1/2哪一个多项式求导:");
                        scanf("%d",&m);
                        if(m==1)
                        {
                            headdao=dao(pa);
                            printf("\n\tpa导=");
                            printpolyn(headdao);
                            printf("\n");
                        }
                        if(m==2)
                        {
                            headdao=dao(pb);
                            printf("\n\tpb导=");
                            printpolyn(headdao);
                            printf("\n");
                        }
                        break;
                    }
                dafault:
                    {
                        printf("errror");
                    }

            }
        }while(choice!='c'&&choice!='q');
    }while(choice!='q');
}
