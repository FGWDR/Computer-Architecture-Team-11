#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include<windows.h>
#define SIZE 20//파일 사이즈, 넉넉하게 20으로 지정
const int M_SIZE = 1024;//메모리 사이즈
unsigned char MEM[1024];//M_SIZE를 넣었더니 "상수값" 없다는 에러가 떠서 1024로 숫자를 대신 넣음
unsigned int IR;//Instruction Register
#include"MIPS.h"

unsigned int invertEndian(unsigned int inVal);//little->big endian으로 변환
void memoryWrite(unsigned int addr, unsigned int data);//메모리에 작성
unsigned int ALU(unsigned int inst, unsigned int PC);////메모리 읽기

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
//unsigned int RegAccess(int A, unsigned int V, unsigned int nRW);// A: memory Address ,V:write value, nRW: 0->Read, 1->Write
//unsigned int MemAccess(unsigned int A, unsigned int V, int nRW, int S);
//A: memory Address , V:write value, nRW: 0->Read, 1->Write, S(size): 0->Byte, 1->Half word, 2->Word
//void viewRegister(void);
//void viewMemory(unsigned int start, unsigned int end);
    for(int i = 2;i<2+num;i++)
    {
        MemAccess(buffer[i], i, 1, 2);
    }
    int PC=0;//program counter
    for(int i = 2;i<2+num;i++)
    {
        //PC++; jump 명령어들을 고려해 PC++은 ALU안에 넣기로함
        MemAccess(buffer[i], i, 0, 2);
        MEM[i]=buffer[i];
        ALU(buffer[i], PC);
    }
    fclose(fp);
    return 0;
}

unsigned int invertEndian(unsigned int inVal)//little to big endian
{
    inVal=((inVal>>24)&0xff)|((inVal<<8)&0xff0000)|((inVal>>8)&0xff00)|((inVal<<24)&0xff000000);
    return inVal;
}

unsigned int ALU(unsigned int inst, unsigned int PC)//0xaabbccdd
{
    unsigned int IR=invertEndian(inst);//big -> little
    unsigned int opcode=IR>>26;
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
            //shift operations-----------------------------
            case 0://sll: rt=start, rd=dest, shamt=amount
                MEM[rd]=(MEM[rt]<<shamt) & 0xffffffff;
                PC+=4;
                break;
            case 2://srl: 
                MEM[rd] = (MEM[rt] >> shamt) & 0xffffffff;
                PC+=4;
                break;
            case 3://sra: MSB 복사, 나머지 오른쪽으로 shift
                unsigned int MSB= MEM[rt] & 0x80000000;//0,1
                unsigned int temp;
                for(int i=0; i<shamt; i++)
                {
                    temp = (MEM[rt] >> 1) + MSB;
                }
                MEM[rd]=temp;
                PC+=4;
                break;
            //
            case 8://Jump register: jr $ra
                unsigned int ra = IR >> 21;
                PC=ra;
                break;
            case 12://syscall
                printf("Inst: syscall\n");
                PC+=4;
                break;
            case 16://mfhi rd: Move from Hi
                unsigned int rd = (IR>>11) & 0x1f;
                rd = RegAccess(HI, hi, 0);
                PC+=4;
                break;
            case 18://mflo rd: Move from Lo
                unsigned int rd = (IR>>11) & 0x1f;
                rd = RegAccess(LO, lo, 0);
                PC+=4;
                break;
            case 24://mult: multiply rs,rt: MULT rs, rt; HI:LO = rs * rt (signed)
                unsigned int hi=(rs*rt)>>32;//64bit중 상위 32bit
                unsigned int lo=(rs*rt) & 0xffffffff;//64bit중 하위 32bit
                RegAccess(HI, hi, 1);
                RegAccess(LO, lo, 1);
                PC+=4;
                break;
                //--------------------------
            case 32://add rd = rs + rt
                MEM[rd]=MEM[rs] + MEM[rt];
                PC+=4;
                break;
            case 34://sub
                MEM[rd] = MEM[rs] - MEM[rt];
                PC+=4;
                break;
            case 36://and
                MEM[rd] = MEM[rs] & MEM[rt];
                PC+=4;
                break;
            case 37:// or
                MEM[rd] = MEM[rs] | MEM[rt];
                PC+=4;
                break;
            case 38://xor
                MEM[rd] = MEM[rs] ^ MEM[rt];
                PC+=4;
                break;
            case 39://nor
                MEM[rd] = ~(MEM[rs] | MEM[rt]);
                PC+=4;
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
                PC+=4;
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
                PC+=4;
                break;
            case 1://bltz rs,L: branch less than 0
                unsigned int address = IR & 0x03ffffff;
                unsigned int L=(PC<<28)|address<<2;
                unsigned int rs = (IR >> 21) & 0x0000001f;
                if(MEM[rs] < 0)
                {
                    ALU(L,PC);
                }
                break;
            case 2://j L: jump-> address00+4 : program counter
                unsigned int address = IR & 0x03ffffff;
                unsigned int L=(PC<<28)|address<<2;
                //다음 처리될 명령어는 L이 되어야한다.
                PC++;
                ALU(L, PC);
                break;
            case 3://jal L: jump and link
                unsigned int address = IR & 0x03ffffff;
                unsigned int L=(PC<<28)|address<<2;
                RegAccess(L,ALU(L, PC), 1);//다음 주소값 명령어를 레지스터에 저장
                break;
            case 4://beq rs, rt, L: branch equal
                unsigned int address = IR & 0x03ffffff;
                unsigned int L=(PC<<28)|address<<2;
                if(MEM[rt] == MEM[rs])
                {
                    RegAccess(L, ALU(L, PC), 1);
                }
                else
                {
                    PC+=4;
                }
                break;
            case 5://bne rs, rt, L: Branch not equal
                unsigned int address = IR & 0x03ffffff;
                unsigned int L=(PC<<28)|address<<2;
                if(MEM[rt] != MEM[rs])
                {
                    RegAccess(L, ALU(L, PC), 1);
                }
                else
                {
                    PC+=4;
                }
                break;
            case 8://addi rt,rs, imm: ADD immediate
                MEM[rt]=MEM[rs] + conAdd;
                PC+=4;
                break;
            case 10://slti rd,rs, imm: set less than immediate
                if(rs< conAdd)
                {
                    MemAccess(rt, 1, 1, 2);//$s1=1
                }
                else
                {
                    MemAccess(rt, 0, 1, 2);//$s1=0
                }
                PC+=4;
                break;
            case 12:// andi rt, rs, imm: AND immediate
                unsigned int imm = IR & 0xffff;
                MEM[rt] = MEM[rs] & imm;
                MemAccess(rt, MEM[rt], 1, 2);
                PC+=4;
                break;
            case 13://ori rt, rs, imm: OR immediate
                unsigned int imm = IR & 0xffff;
                MEM[rt] = MEM[rs] | imm;
                MemAccess(rt, MEM[rt], 1, 2);
                PC+=4;
                break;
            case 14://xor rt, rs, imm: XOR immediate
                unsigned int imm = IR & 0xffff;
                MEM[rt] = MEM[rs] ^ imm;
                MemAccess(rt, MEM[rt], 1, 2);
                PC+=4;
                break;
            case 15:// lui rt, imm: load upper immediate// 상위 16bit에 imm값 넣고 뒤 16bit는 0으로 둔다.
                unsigned int rt = IR & 0xffff0000;
                unsigned int imm = (IR & 0xffff) << 16;
                MemAccess(rt, imm, 1, 2);
                PC+=4;
                break;
            case 32://lb rt, imm(rs): load byte
                unsigned imm = IR & 0xffff;//offset
                RegAccess(rt, MemAccess(rs, imm, 0, 0), 1);
                PC+=4;
                break;
            case 35://lw rt, imm(rs): load word
                unsigned imm = IR & 0xffff;//offset
                RegAccess(rt, MemAccess(rs, imm, 0, 2), 1);
                PC+=4;
                break;
            case 36://lbu rt, imm(rs): load byte unsigned
                unsigned imm = IR & 0xffff;//offset
                RegAccess(rt, MemAccess(rs, -imm, 0, 0), 1);
                PC+=4;
                break;
            case 40://sb rt, imm(rs): store byte/ reg->mem
                unsigned imm = IR & 0xffff;//offset
                MemAccess(rt, RegAccess(rs, imm, 0), 1, 0);
                PC+=4;
                break;
            case 43://sw rt, im(rs): store word
                unsigned imm = IR & 0xffff;//offset
                MemAccess(rt, RegAccess(rs, imm, 0), 1, 2);
                PC+=4;
                break;
            default:
                PC+=4;
                break;
        }
    }
}
