#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include<windows.h>
#define SIZE 20//파일 사이즈, 넉넉하게 20으로 지정
const int M_SIZE = 1024;//메모리 사이즈
unsigned char MEM[1024];//M_SIZE를 넣었더니 "상수값" 없다는 에러가 떠서 1024로 숫자를 대신 넣음
unsigned int IR;//Instruction Register

unsigned int invertEndian(unsigned int inVal);//little->big endian으로 변환
void memoryWrite(unsigned int addr, unsigned int data);//메모리에 작성
unsigned int memoryRead(unsigned int addr);////메모리 읽기

int main(void)
{
    int i;
    int buffer[SIZE];
    FILE* fp=NULL;
    fp=fopen("as_ex04_fct.bin","rb");
    //레지스터로부터 들어올 
    if(fp==NULL)
    {
        fprintf(stderr,"Fail to open the file");
        return 1;
    }
    fread(buffer,sizeof(int), SIZE, fp);
    printf("Number of instructions: %d  ",invertEndian(buffer[0]));
    printf("Number of data: %d\n", invertEndian(buffer[1]));
    int num=invertEndian(buffer[0]);
    for(int i = 2;i<2+num;i++)
    {
        memoryWrite(buffer[i],i);
    }
    for(int i = 2;i<2+num;i++)
    {
        memoryRead(buffer[i]);
    }
    /*for(int i=2;i<num+2;i++)
    {
        printf("%x\n",MEM[i]);
    }*/
    fclose(fp);
    return 0;
}

unsigned int invertEndian(unsigned int inVal)//little to big endian
{
    inVal=((inVal>>24)&0xff)|((inVal<<8)&0xff0000)|((inVal>>8)&0xff00)|((inVal<<24)&0xff000000);
    return inVal;
}

void memoryWrite(unsigned int addr, unsigned int data)
{
    addr=invertEndian(addr);
    MEM[data-2]=((char *)&addr,sizeof(addr));
}

unsigned int memoryRead(unsigned int addr)//0xaabbccdd
{
    unsigned int IR=addr; //instruction register
    IR=((IR<<24)&0xff000000)|((IR<<8)&0x00ff0000)|((IR>>8)&0x0000ff00)|((IR>>24)&0x000000ff);//big -> little
    unsigned int opcode=IR>>26;
    //unsigned int funct=IR&0x0000003f;
    //printf("Opc: %8x, Fct: %8x, ",opcode, funct);
    //-------------------------------------------
    //opand와 funct의 이진수 앞세자리와 뒤세자리 분리(원래 Instruction Encoding을 2차원 배열로 만들어 찾을려고 시도했음)
    //unsigned int opcodeCol=opcode>>3;
    //unsigned int opcodeRow=opcode&0x7;
    //unsigned int functCol=funct<<3;
    //unsigned int functRow=funct&0x7;
    //-------------------------------------------
    if(opcode==0)//R 명령어
    {
        unsigned int rs = IR>>21;
        unsigned int rt = (IR>>16) & 0x1f;//0001 1111
        unsigned int rd = (IR>>11) & 0x1f;
        unsigned int shamt = (IR>>6) & 0x1f;
        unsigned int funct = IR & 0x3f;//0011 1111
        switch(funct)
        {
            //shift operations---------------------
            case 0://sll: rt=start, rd=dest, shamt=amount
                MEM[rd]=(MEM[rt]<<shamt) & 0xffffffff;
                break;
            case 2://srl: 
                MEM[rd] = (MEM[rt] >> shamt) & 0xffffffff;
                break;
            case 3://sra: MSB 복사, 나머지 오른쪽으로 shift
                unsigned int MSB= MEM[rt] & 0x80000000;//0,1
                unsigned int temp;
                for(int i=0; i<shamt; i++)
                {
                    temp = (MEM[rt] >> 1) + MSB;
                }
                MEM[rd]=temp;
                break;
            //
            case 8://Jump register
                printf("Inst: jr\n");
                break;
            case 12://syscall
                printf("Inst: syscall\n");
                break;
            case 16://mfhi rd: Move from Hi
                printf("Inst: mfhi\n");
                break;
            case 18://mflo rd: Move from Lo
                printf("Inst: mflo\n");
                break;
            case 24://mult: multiply rs,rt
                printf("Inst: mult\n");
                break;
                //--------------------------
            case 32://add rd = rs + rt
                MEM[rd]=MEM[rs] + MEM[rt];
                break;
            case 34://sub
                MEM[rd] = MEM[rs] - MEM[rt];
                break;
            case 36://and
                MEM[rd] = MEM[rs] & MEM[rt];
                break;
            case 37:// or
                MEM[rd] = MEM[rs] | MEM[rt];
                break;
            case 38://xor
                MEM[rd] = MEM[rs] ^ MEM[rt];
                break;
            case 39://nor
                MEM[rd] = ~(MEM[rs] | MEM[rt]);
                break;
            case 42://slt: set less than: if(rs<rt) rd = 1; else rd = 0;
                if(rs<rt)
                {
                    rd=1;
                }
                else
                {
                    rd=0;
                }
                break;
            default:// default
                break;
        }
    }
    else//opcode가 000_000이 아니므로 opcode 명령어 찾기
    {
        //I format instructions
        unsigned int rs = (IR >> 21) & 0x0000001f;
        unsigned int rt = (IR>>16) & 0x0000001f;
        unsigned int conAdd = IR & 0x0000ffff;
        switch(opcode)
        {
            case 0://R-format
                printf("Inst: R-format\n");
                break;
            case 1://bltz rs,L: branch less than 0
                printf("Inst: bltz\n");
                break;
            case 2://j L: jump-> address+4 : program counter
                unsigned int address = IR & 0x03ffffff;
                break;
            case 3://jal L: jump and link
                printf("Inst: jal\n");
                break;
            case 4://beq rs, rt, L: branch equal
                printf("Inst: beq\n");
                break;
            case 5://bne rs, rt, L: Branch not equal
                printf("Inst: bne\n");
                break;
            case 8://addi rt,rs, imm: ADD immediate
                MEM[rt]=MEM[rs] + conAdd;
                break;
            case 10://slti rd,rs, imm: set less than immediate
                if(rs< conAdd)
                {
                    rt = 1;
                }
                else
                {
                    rt=0;
                }
                break;
            case 12:// andi rt, rs, imm: AND immediate
                printf("Inst: andi\n");
                break;
            case 13://ori rt, rs, imm: OR immediate
                printf("Inst: ori\n");
                break;
            case 14://xor rt, rs, imm: XOR immediate
                printf("Inst: xori\n");
                break;
            case 15:// lui rt, imm: load upper immediate
                printf("Inst: lui\n");
                break;
            case 32://lb rt, imm(rs): load byte
                printf("Inst: lb\n");
                break;
            case 35://lw rt, imm(rs): load word
                printf("Inst: lw\n");
                break;
            case 36://lbu rt, imm(rs): load byte unsigned
                printf("Inst: lbu\n");
                break;
            case 40://sb rt, imm(rs): store byte
                printf("Inst: sb\n");
            case 43://sw rt, im(rs): store word
                printf("Inst: sw\n");
            default:
                break;
        }
    }
}
