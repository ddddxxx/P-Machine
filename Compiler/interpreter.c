#include <assert.h>
#include "interpreter.h"

int dsp[DISPLAY_TABLE_SIZE];
int cur_layer;

void interpret(instruction *code) {
    int p,b,t;             /*指令指针，指令基址，栈顶指针*/
    instruction i;  /*存放当前指令*/
    int stack[STACK_SIZE];      /*栈*/
    puts("start p-machine");
    t=0;
    b=0;
    p=0;
    stack[0]=stack[1]=stack[2]=stack[3]=0;
    cur_layer=0;
    dsp[0]=0;
    do{
        i=code[p];         /*读当前指令*/
        p++;
        switch(i.f) {
            case LIT:        /*将a的值取到栈顶*/
                stack[t]=i.a;
                t++;
                break;
            case OPR:        /*数字、逻辑运算*/
                switch(i.a) {
                    case 1:
                        stack[t-1]=-stack[t-1];
                        break;
                    case 2:
                        t--;
                        stack[t-1]=stack[t-1]+stack[t];
                        break;
                    case 3:
                        t--;
                        stack[t-1]=stack[t-1]-stack[t];
                        break;
                    case 4:
                        t--;
                        stack[t-1]=stack[t-1]*stack[t];
                        break;
                    case 5:
                        t--;
                        stack[t-1]=stack[t-1]/stack[t];
                        break;
                    case 6:
                        stack[t-1]=stack[t-1]%2;
                        break;
                    case 7:
                        t--;
                        stack[t-1]=stack[t-1]%stack[t];
                        break;
                    case 8:
                        t--;
                        stack[t-1]=(stack[t-1]==stack[t]);
                        break;
                    case 9:
                        t--;
                        stack[t-1]=(stack[t-1]!=stack[t]);
                        break;
                    case 10:
                        t--;
                        stack[t-1]=(stack[t-1]<stack[t]);
                        break;
                    case 11:
                        t--;
                        stack[t-1]=(stack[t-1]<=stack[t]);
                        break;
                    case 12:
                        t--;
                        stack[t-1]=(stack[t-1]>stack[t]);
                        break;
                    case 13:
                        t--;
                        stack[t-1]=(stack[t-1]>=stack[t]);
                        break;
                }
                break;
            case LOD:       /*取相对当前过程的数据基地址为ａ的内存的值到栈顶*/
                stack[t]=stack[dsp[cur_layer-i.l]+i.a];
                t++;
                break;
            case STO:       /*栈顶的值存到相对当前过程的数据基地址为ａ的内存*/
                t--;
                stack[dsp[cur_layer-i.l]+i.a]=stack[t];
                break;
            case CAL:              /*调用子程序*/
                stack[t]=cur_layer;
                cur_layer-=i.l-1;
                assert(cur_layer<DISPLAY_TABLE_SIZE);
                stack[t+1]=dsp[cur_layer];
                dsp[cur_layer]=t;
                stack[t+2]=b;           /*将本过程基地址入栈*/
                stack[t+3]=p;           /*将当前指令指针入栈*/
                b=t;                /*改变基地址指针值为新过程的基地址*/
                p=i.a;              /*跳转*/
                break;
            case RET:
                t=b;
                p = stack[t+3];
                b = stack[t+2];
                dsp[cur_layer] = stack[t+1];
                cur_layer = stack[t];
                break;
            case INC:             /*分配内存*/
                t+=i.a;
                break;
            case JMP:             /*直接跳转*/
                p=i.a;
                break;
            case JPC:              /*条件跳转*/
                t--;
                if(stack[t]==0) {
                    p=i.a;
                }
                break;
            case SI:
                scanf("%d",&(stack[t]));
                t++;
                break;
            case SO:
                t--;
                printf("%d\n",stack[t]);
                break;
        }
    } while (p!=0);
    puts("stop p-machine");
}
