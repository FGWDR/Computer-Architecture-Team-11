#ifndef MIPS_H_
#define MIPS_H_
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//메모리
//레지스터

union instructionRegister { // 32bit
	unsigned int I;
	struct RFormat {
		unsigned int funct : 6;
		unsigned int shamt : 5;
		unsigned int rd : 5;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int opcode : 6;
	} RI; //RFormat register
	struct IFormat {
		unsigned int immediate : 16;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int opcode : 6;
	} II;
	struct JFormat {
		unsigned int address : 26;
		unsigned int opcode : 6;
	} JI;
	struct bit {
		unsigned int  Zero : 1;//funct0
		unsigned int  One : 1;
		unsigned int  Two : 1;
		unsigned int  Three : 1;
		unsigned int  Four : 1;//funct4
		unsigned int  Five : 1;//funct5
		unsigned int : 20;
		unsigned int  zero : 1;//op0
		unsigned int  one : 1;
		unsigned int  two : 1;
		unsigned int  three : 1;
		unsigned int  four : 1;//op4
		unsigned int  five : 1;//op5
	} B;
}IR; //IR: instruction register

struct control_signal { //opcode로 만든 control #각 부분에서 signal을 사용한다
	unsigned int RegDst : 1;
	unsigned int Jump : 1;
	unsigned int ALUSrc : 1;
	unsigned int MemtoReg : 1;
	unsigned int RegWrite : 1;
	unsigned int MemRead : 1;
	unsigned int MemWrite : 1;
	unsigned int Branch : 1;
	unsigned int ALUOp1 : 1;
	unsigned int ALUOp0 : 1;
}convalue;

unsigned int RegAccess(int A, unsigned int V, unsigned int nRW);
unsigned int MemAccess(unsigned int A, unsigned int V, int nRW, int S);
void viewRegister(void);
void viewMemory(unsigned int start, unsigned int end);

//메모리 접근 함수
#endif /* MIPS_H_ */