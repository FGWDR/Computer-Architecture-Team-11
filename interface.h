#ifndef HEADER_H
#   define HEADER_H
#   include <stdio.h>
#   define TRUE    1
#   define FALSE   0

union instructionRegiser{
    unsigned int I;
    struct RFormat{
        unsigned int funct : 6;
        unsigned int sh : 5;
        unsigned int rd : 5;
        unsigned int rt : 5;
        unsigned int rs : 5;
        unsigned int opcode : 6;
    }RI;
}IR;  //Instruction Fatch용

unsigned int PC;  //Program Counter(초기값: 메모리 text영역 시작 주소)
int isEnd;  //syscall 10을 만나면 프로그램 종료
int isExecutable;  //프로그램 실행 가능 여부 판단
FILE *filePointer;  //실행시킬 바이너리 파일을 위한 파일 스트림

void setPC(unsigned int addr);
void setSP(unsigned int addr);
int loadProgram(FILE *filePointer);
void jumpProgram(unsigned int startPosition);
void step(void);
void goProgram(void);
void viewMemory(unsigned int start,unsigned int end);
void viewRegister(void);
void setRegister(int regNum,int value);
void setMemory(unsigned int location,int value);
unsigned int invertEndian(unsigned int inputValue);
int interface(void);

#endif