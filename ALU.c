#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<string.h>
#include <stdlib.h>
//#define SIZE 20//파일 사이즈, 넉넉하게 20으로 지정
const int M_SIZE = 1024;//메모리 사이즈
unsigned char MEM[1024];//M_SIZE를 넣었더니 "상수값" 없다는 에러가 떠서 1024로 숫자를 대신 넣음
unsigned int IR;//Instruction Register
#include"MIPS.h"

unsigned int invertEndian(unsigned int inVal);//little->big endian으로 변환
void memoryWrite(unsigned int addr, unsigned int data);//메모리에 작성
unsigned int ALU(unsigned int inst, unsigned int PC);////메모리 읽기

unsigned int invertEndian(unsigned int inVal)//little to big endian
{
    inVal=((inVal>>24)&0xff)|((inVal<<8)&0xff0000)|((inVal>>8)&0xff00)|((inVal<<24)&0xff000000);
    return inVal;
}

unsigned int ALU(unsigned int inst, unsigned int PC)//0xaabbccdd
{
    IR.I=invertEndian(inst);//big -> little
    unsigned int opcode=IR>>26;
    PC=RegAccess(32,NULL, 0);
    //-------------------------------------------
    if(IR.RI.opcode==0)//R 명령어
    {
        switch(IR.RI.funct)
        {
            //shift operations-----------------------------
            case 0://sll: rt=start, rd=dest, sh=amount
                RegAccess(IR.RI.rd,RegAccess(IR.RI.rt,NULL,0)<<IR.RI.sh,1);
                RegAccess(32, PC+4, 1);
                break;
            case 2://srl: 
                RegAccess(IR.RI.rd,RegAccess(IR.RI.rt,NULL,0)>>IR.RI.sh,1);
                RegAccess(32, PC+4, 1);
                break;
            case 3://sra: MSB 복사, 나머지 오른쪽으로 shift
                unsigned int MSB= MEM[IR.RI.rt] & 0x80000000;//0,1
                unsigned int temp;
                for(int i=0; i<IR.RI.sh; i++)
                {
                    temp = (MEM[IR.RI.rt] >> 1) + MSB;
                }
                MEM[IR.RI.rd]=temp;
                RegAccess(32, PC+4, 1);
                break;
            //
            case 8://Jump register: jr $ra
                unsigned int ra = IR >> 21;
                PC=IR.RI.ra;
                RegAccess(32, PC, 1);
                break;
            case 12://syscall
                printf("Inst: syscall\n");
                RegAccess(32, PC+4, 1);
                return 0;
            case 16://mfhi rd: Move from Hi
                unsigned int rd = (IR>>11) & 0x1f;
                IR.RI.rd = RegAccess(33, NULL, 0);
                RegAccess(32, PC+4, 1);
                break;
            case 18://mflo rd: Move from Lo
                unsigned int rd = (IR>>11) & 0x1f;
                IR.RI.rd = RegAccess(34, NULL, 0);
                RegAccess(32, PC+4, 1);
                break;
            case 24://mult: multiply rs,rt: MULT rs, rt; HI:LO = rs * rt (signed)
                unsigned int hi=(RegAccess(IR.RI.rs,NULL,0)*RegAccess(IR.RI.rt,NULL,0))>>32;//64bit중 상위 32bit
                unsigned int lo=(RegAccess(IR.RI.rs,NULL,0)*RegAccess(IR.RI.rt,NULL,0)) & 0xffffffff;//64bit중 하위 32bit
                RegAccess(33, hi, 1);
                RegAccess(34, lo, 1);
                RegAccess(32, PC+4, 1);
                break;
                //--------------------------
            case 32://add rd = rs + rt
                MEM[IR.RI.rd]=MEM[IR.RI.rs] + MEM[IR.RI.rt];
                RegAccess(32, PC+4, 1);
                break;
            case 34://sub
                MEM[IR.RI.rd] = MEM[IR.RI.rs] - MEM[IR.RI.rt];
                RegAccess(32, PC+4, 1);
                break;
            case 36://and
                MEM[rd] = MEM[rs] & MEM[rt];
                RegAccess(32, PC+4, 1);
                break;
            case 37:// or
                MEM[IR.RI.rd] = MEM[IR.RI.rs] | MEM[IR.RI.rt];
                RegAccess(32, PC+4, 1);
                break;
            case 38://xor
                MEM[IR.RI.rd] = MEM[IR.RI.rs] ^ MEM[IR.RI.rt];
                RegAccess(32, PC+4, 1);
                break;
            case 39://nor
                MEM[IR.RI.rd] = ~(MEM[IR.RI.rs] | MEM[IR.RI.rt]);
                RegAccess(32, PC+4, 1);
                break;
            case 42://slt: set less than: if(rs<rt) rd = 1; else rd = 0;
                if(IR.RI.rs<IR.RI.rt)
                {
                    IR.RI.rd=1;
                }
                else
                {
                    IR.RI.rd=0;
                }
                RegAccess(32, PC+4, 1);
                break;
            default:// default
                break;
        }
    }
    else//opcode가 000_000이 아니므로 opcode 명령어 찾기
    {
        //I format instructions
        unsigned int conAdd = IR & 0x0000ffff;
        unsigned int address = IR & 0x03ffffff;
        switch(opcode)
        {
//branch 명령어-----------------------------------------------------
            case 1://bltz rs,L: branch less than 0
                unsigned int L=(PC<<28)|address<<2;
                if(MEM[IR.RI.rs] < 0)
                {
                    ALU(L,PC);
                }
                RegAccess(32, PC, 1);
                break;
            case 2://j L: jump-> address00+4 : program counter
                unsigned int L=(PC<<28)|address<<2;
                //다음 처리될 명령어는 L이 되어야한다.
                RegAccess(32, PC+4, 1);
                ALU(L, PC);
                break;
            case 3://jal L: jump and link
                unsigned int L=(PC<<28)|address<<2;
                RegAccess(L,ALU(L, PC), 1);//다음 주소값 명령어를 레지스터에 저장
                break;
            case 4://beq rs, rt, L: branch equal
                unsigned int L=(PC<<28)|address<<2;
                if(MEM[IR.RI.rt] == MEM[IR.RI.rs])
                {
                    RegAccess(L, ALU(L, PC), 1);
                }
                else
                {
                    RegAccess(32, PC+4, 1);
                }
                break;
            case 5://bne rs, rt, L: Branch not equal
                unsigned int L=(PC<<28)|address<<2;
                if(MEM[IR.RI.rt] != MEM[IR.RI.rs])
                {
                    RegAccess(L, ALU(L, PC), 1);
                }
                else
                {
                    RegAccess(32, PC+4, 1);
                }
                break;
//immediate-------------------------------------------------------------
            case 8://addi rt,rs, imm: ADD immediate
                MEM[IR.RI.rt]=MEM[IR.RI.rs] + conAdd;
                RegAccess(32, PC+4, 1);
                break;
            case 10://slti rd,rs, imm: set less than immediate
                if(IR.RI.rs< conAdd)
                {
                    MemAccess(IR.RI.rt, 1, 1, 2);//$s1=1
                }
                else
                {
                    MemAccess(IR.RI.rt, 0, 1, 2);//$s1=0
                }
                RegAccess(32, PC+4, 1);
                break;
            case 12:// andi rt, rs, imm: AND immediate
                unsigned int imm = IR & 0xffff;
                MEM[IR.RI.rt] = MEM[IR.RI.rs] & imm;
                MemAccess(IR.RI.rt, MEM[IR.RI.rt], 1, 2);
                RegAccess(32, PC+4, 1);
                break;
            case 13://ori rt, rs, imm: OR immediate
                unsigned int imm = IR & 0xffff;
                MEM[IR.RI.rt] = MEM[IR.RI.rs] | imm;
                MemAccess(IR.RI.rt, MEM[IR.RI.rt], 1, 2);
                RegAccess(32, PC+4, 1);
                break;
            case 14://xor rt, rs, imm: XOR immediate
                unsigned int imm = IR & 0xffff;
                MEM[IR.RI.rt] = MEM[IR.RI.rs] ^ imm;
                MemAccess(IR.RI.rt, MEM[IR.RI.rt], 1, 2);
                RegAccess(32, PC+4, 1);
                break;
            case 15:// lui rt, imm: load upper immediate// 상위 16bit에 imm값 넣고 뒤 16bit는 0으로 둔다.
                unsigned int imm = (IR & 0xffff) << 16;
                MemAccess(IR.RI.rt, imm, 1, 2);
                RegAccess(32, PC+4, 1);
                break;
            case 32://lb rt, imm(rs): load byte
                unsigned imm = IR & 0xffff;//offset
                RegAccess(IR.RI.rt, MemAccess(IR.RI.rs, imm, 0, 0), 1);
                RegAccess(32, PC+4, 1);
                break;
            case 35://lw rt, imm(rs): load word
                unsigned imm = IR & 0xffff;//offset
                RegAccess(IR.RI.rt, MemAccess(IR.RI.rs, imm, 0, 2), 1);
                RegAccess(32, PC+4, 1);
                break;
            case 36://lbu rt, imm(rs): load byte unsigned
                unsigned imm = IR & 0xffff;//offset
                RegAccess(IR.RI.rt, MemAccess(IR.RI.rs, -imm, 0, 0), 1);
                RegAccess(32, PC+4, 1);
                break;
            case 40://sb rt, imm(rs): store byte/ reg->mem
                unsigned imm = IR & 0xffff;//offset
                MemAccess(IR.RI.rt, RegAccess(IR.RI.rs, imm, 0), 1, 0);
                RegAccess(32, PC+4, 1);
                break;
            case 43://sw rt, im(rs): store word
                unsigned imm = IR & 0xffff;//offset
                MemAccess(IR.RI.rt, RegAccess(IR.RI.rs, imm, 0), 1, 2);
                RegAccess(32, PC+4, 1);
                break;
            default:
                RegAccess(32, PC+4, 1);
                break;
        }
    }
}
