/*************************************************************************
    > File Name: yimaqi.c
    > Author: 
    > Mail: 
    > Created Time: 2018年12月27日 星期四 21时52分02秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<math.h>

#define N 200
#define M 2*N-1
int t=0;

typedef char * Huffmancode[N+1];
typedef struct
{
    int weight;
    int parent,Lchild,Rchild;
}HTNode,HuffmanTree[M+1];

typedef struct
{
    unsigned char ch;
    int p;
}Weight;

//统计文件中的字符频度
void Count(char *filename,Weight *counts)
{
    FILE *fp = fopen(filename,"r");
    unsigned char c;
    c = fgetc(fp);
    while(!feof(fp))
    {
        t++;
        counts[c].ch=c;
        counts[c].p++;
        c=fgetc(fp);
    }
    fclose(fp);
}

//在ht的前n-1项中选双亲为0且权值最小的两结点s1,s2;
void select1(HuffmanTree ht,int m,int *s1,int *s2)
{
    int f=0;
    for(int i=1;i<=m;i++){
        if(ht[i].parent==0){
            *s1=i;
            break;
        }
    }
    for(int i=1;i<=m;i++){
        if(ht[i].weight<ht[*s1].weight&&ht[i].parent==0){
            f=1;
            *s2=*s1;
            *s1=i;
        }
    
    }
    if(f==0)
    {
        for(int i=1;i<=m;i++){
            if(ht[i].parent==0&&i!=*s1){
                *s2=i;
                break;
            }
        }
        for(int j=1;j<m;j++){
            if(ht[j].weight<ht[*s2].weight&&ht[j].parent==0&&j!=*s1){
                *s2=j;
            }
        }
    }
    int temp;
    if(*s1>*s2)
    {
        temp=*s1;
        *s1=*s2;
        *s2=temp;
    }
}

//建立哈夫曼树
void CrtHuffmanTree(HuffmanTree ht,Weight w[],int n)
{
    int m,i,s1,s2;
    m=2*n-1;
    for(i=1;i<=n;i++){
        ht[i].weight=w[i].p;
        ht[i].parent=0;
        ht[i].Lchild=0;
        ht[i].Rchild=0;
    }
    for(i=n+1;i<=m;i++){
        ht[i].weight=0;
        ht[i].parent=0;
        ht[i].Lchild=0;
        ht[i].Rchild=0;
    }
    for(i=n+1;i<=m;i++)
    {
        select1(ht,i-1,&s1,&s2);
        ht[i].weight=ht[s1].weight+ht[s2].weight;
        ht[i].Lchild=s1;
        ht[i].Rchild=s2;
        ht[s1].parent=i;
        ht[s2].parent=i;
    }
}



//哈夫曼编码
void CrtHuffmanCode1(HuffmanTree ht,Huffmancode hc,int n)
{
    char *cd;
    int start,c,p;
    cd=(char *)malloc(n*sizeof(char));
    cd[n-1]='\0';
    for(int i=1;i<=n;i++)
    {
        start=n-1;
        c=i;
        p=ht[i].parent;
        while(p!=0)
        {
            --start;
            if(ht[p].Lchild==c)
                cd[start]='0';
            else
                cd[start]='1';
            c=p;
            p=ht[p].parent;
        }
        hc[i]=(char *)malloc((n-start)*sizeof(char));
        strcpy(hc[i],&cd[start]);
    }
    free(cd);
    printf("编码为:");
    for(int i=1;i<=n;i++)
    {
        printf("%s",hc[i]);
    }
    printf("\n");
}

//对源文件编码进行压缩并存储
void compress(Huffmancode hc,Weight w[],int n){
    char a[8];
    int m;
    char c,ch;
    if(t>=100) m=3;
    else if(t>9) m=2;
    else m=1;
    FILE *fp = fopen("test.souce","r");
    if(fp == NULL){
        printf("源文件打开错误!");
    }
    FILE *fout = fopen("target.code","ab");
    if(fout == NULL){
        printf("目标文件打开错误!");
    }
    char b[m];
    fputc(m+'0',fout);
    for(int i=0;i<m;i++){
        b[i]=t%10+'0';
        fputc(b[i],fout);
        t=t/10;
    }
    c=fgetc(fp);
    while(!feof(fp)){
        for(int i=1;i<=n;i++){
            if(c==w[i].ch){
                for(int j=0;j<strlen(hc[i]);j++){
                    a[t++]=hc[i][j];
                    if(t==8){
                        ch=(a[0]-48)*128+(a[1]-48)*64+(a[2]-48)*32+(a[3]-48)*16+(a[4]-48)*8+(a[5]-48)*4+(a[6]-48)*2+(a[7]-48)*1;
                        fputc(ch,fout);
                        t=0;
                    }
                }
                break;
            }     
        }
        c=fgetc(fp);
    }
    fclose(fp);
    fclose(fout);
}


// void print(HuffmanTree ht,int n)
// {
//     int m=2*n-1;
//     printf("weight parent lchild rchild \n");
//     for(int i=1;i<=m;i++)
//     {
//         printf(" %d      %d       %d        %d\n",ht[i].weight,ht[i].parent,ht[i].Lchild,ht[i].Rchild);
//     }
// }


int GetBit(FILE *fp)
{
    static int i = 7;
    static unsigned char Bchar;
    int x;
    unsigned char bit[8]={128,64,32,16,8,4,2,1};
    i++;
    if(i==8)
    {
        Bchar=fgetc(fp);
        i=0;
    }
    return (Bchar&bit[i]);
}



//哈夫曼编码的译码
void CrtHuffmantranslate(HuffmanTree ht,Weight w[],int n,FILE *fileb,FILE *filec)
{
    int head,j=0,t;
    int m=2*n-1;
    char ch;
    int ByteNum;
    ch = fgetc(fileb);
    int co=ch-'0';
    for(int i=0;i<co;i++){
        ch=fgetc(fileb);
        ByteNum += ((ch-'0')*pow(10,i));
    }
    for(int i=1;i<=m;i++)
    {
        if(ht[i].parent==0)
        {
            head=i;
        }
    }
    t=head;
    int i=1;
    while(1)
    {
        if(i>=ByteNum)
            break;
        if(GetBit(fileb)){
            t=ht[t].Rchild;
        }
        else
            t=ht[t].Lchild;
        if(ht[t].Lchild==0&&ht[t].Rchild==0)
        {
            fputc(w[t].ch,filec);
            printf("%c",w[t].ch);
            t=head;   //找到一个叶子节点，再重新指向根结点
            i++;
        }
    }
    fputc('\n',filec);
    printf("\n");
}


int main()
{
    HuffmanTree ht;
    Huffmancode hc;
    int n=0;
    Weight counts[255]={0};
    memset(counts,0,sizeof(counts));
    Weight a[130];
    memset(a,0,sizeof(a));
    Count("test.souce",counts);
    for(int i=1,j=1;i<=128;i++){
        if(counts[i].p!=0){
            a[j]=counts[i];
            j++;
            n++;
        }
    }
    printf("建立哈夫曼树...\n");
    CrtHuffmanTree(ht,a,n);
    //print(ht,n);
    printf("哈夫曼树建立成功...\n\n");
    printf("开始编码...\n");
    CrtHuffmanCode1(ht,hc,n);
    compress(hc,a,n);//压缩
    printf("编码成功\n\n"); 
    printf("开始译码...\n");
    FILE *fileb = fopen("target.code","r");
    FILE *filec = fopen("jieya.decode","ab");
    printf("译出为:\n");
    CrtHuffmantranslate(ht,a,n,fileb,filec);
    fseek(filec, 0, SEEK_END);
    printf("\n\n译码完毕,且存入文件jiema.decode咯!\n");
    fclose(fileb);
    fclose(filec);
}
