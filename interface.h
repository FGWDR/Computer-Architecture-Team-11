#ifndef INTERFACE_H_
#   define INTERFACE_H_
#   include "MIPS.h"
#   include "ALU.h"
#   define TRUE      1
#   define FALSE     0
#   define PC_REGNUM 32

int isEnd;  //syscall 10을 만나면 프로그램 종료
int isExecutable;  //프로그램 실행 가능 여부 판단
FILE *filePointer;  //실행시킬 바이너리 파일을 위한 파일 스트림

void setPC(unsigned int addr);
void setSP(unsigned int addr);
int loadProgram(FILE *filePointer);
void jumpProgram(unsigned int startPosition);
void step(void);
void goProgram(void);
void setRegister(int regNum,int value);
void setMemory(unsigned int location,int value);
int interface(void);

#endif