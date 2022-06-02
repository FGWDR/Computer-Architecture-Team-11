#define _CRT_SECURE_NO_WARNINGS
#define REG_SIZE 32
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "MIPS.h"

static unsigned char progMEM[0x100000], dataMEM[0x100000], stackMEM[0x100000];
static unsigned int reg[REG_SIZE] //일반 레지스터
static unsigned int PC, HI, LO;  //PC, HI 레지스터, LO 레지스터

//레지스터
unsigned int RegAccess(int A, unsigned int V, unsigned int nRW) {
    // A: memory Address ,V:write value, nRW: 0->Read, 1->Write

    if (A > 31) {
		printf("Invalid register\n");
		return 1;
	}
    
    if (nRW == 0)//read
        return reg[A];

    else if (nRW == 1)//write
        return reg[A] = V;

    else {
        printf("Invalid nRW \n");
        fflush(stdout);
    }

    return 0;
}

//메모리
unsigned int MemAccess(unsigned int A, unsigned int V, int nRW, int S) {
    //A: memory Address , V:write value, nRW: 0->Read, 1->Write, S(size): 0->Byte, 1->Half word, 2->Word

    unsigned int sel, offset; //메모리 접근 변수
    unsigned char* pM = NULL; //메모리 포인터
    unsigned int R; //반환 값

    sel = A >> 20; //메모리 시작점 
    offset = A & 0xFFFFF; //메모리 주소

    // address decoding and select a physical memory
    if (sel == 0x004)      //program memory
        pM = progMEM;
    else if (sel == 0x100) //data memory
        pM = dataMEM;
    else if (sel == 0x7ff) //stack memory
        pM = stackMEM;
    else {
        printf("Empty memory\n");
        exit(1);
    }
    // offset processing for alignment

    if (S == 0) {  // byte
        if (nRW == 0) { //read
            V = pM[offset];
            return V;
        }
        else if (nRW == 1) { //write
            pM[offset] = V & 0xff;
            return V;
        }
    }
    else if (S == 1) { // half word
        if (nRW == 0) { //read
            V = (pM[offset]) | (pM[offset + 1] << 8);
            return V;
        }
        else if (nRW == 1) { //write
            pM[offset] = V & 0x00ff;
            pM[offset + 1] = (V & 0xff00) >> 8;
            return V;
        }
    }
    else if (S == 2) { // word
        if (nRW == 0) { //read
            V = pM[offset] | (pM[offset + 1] << 8) | (pM[offset + 2] << 16) | (pM[offset + 3] << 24);
            return V;
        }
        else if (nRW == 1) { //write
            pM[offset] = V & 0x000000ff; pM[offset + 1] = (V & 0x0000ff00) >> 8; pM[offset + 2] = (V & 0x00ff0000) >> 16; pM[offset + 3] = (V & 0xff000000) >> 24;
            return V;
        }
    }
    else { // error
        printf("memeory access size error");
        return 0;
    } 

    return 0;
}

//레지스터 View
void ViewRegister(void) {
    char* r[32] = { "r0","at","v0","v1","a0","a1","a2","a3","t0","t1","t2","t3","t4","t5","t6","t7","s0","s1","s2","s3","s4","s5","s6","s7","t8","t9","k0","k1","gp","sp","fp","ra" };

    printf("[REGISTER[\n");

    for (int i = 0; i < 32; i++)
        printf("R%02d [%s] = %X\n", i, r[i], reg[i]);
    fflush(stdout);
    printf("\n");

    printf("PC : %08X\n", PC);
    printf("HI : %08X\n", HI);
    printf("LO : %08X\n", LO);
    fflush(stdout);
}

//메모리 View
void ViewMemory(unsigned int start, unsigned int end) {
    unsigned int front, offset, e_offset;
    unsigned char* pM;
    front = start >> 20; //앞 12비트
    offset = start & 0xFFFFF;
    e_offset = end & 0xFFFFF;

    if (front == 0x004)
    {
        pM = progMEM;
        printf("Program Memory space\n");
        fflush(stdout);
    }
    else if (front == 0x100)
    {
        printf("Data Memory space \n");
        fflush(stdout);
        pM = dataMEM;
    }
    else if (front == 0x7FF)
    {
        printf("Stack Memory space\n");
        fflush(stdout);
        pM = stackMEM;
    }
    else {
        printf("No Memory\n");
        fflush(stdout);
        return;
    }
    for (int i = offset; i < (e_offset + 4); i++) {
        if (((i % 4) == 0) || i == offset) {
            printf("\n");
            printf("[%X] ", (front << 20) + i);
        }

        printf("%02x", pM[i]);
    }
    fflush(stdout);
    printf("\n");
}
