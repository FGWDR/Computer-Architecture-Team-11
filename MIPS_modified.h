#ifndef MIPS_H_
#define MIPS_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//�޸�
//��������

unsigned int RegAccess(int A, unsigned int V, unsigned int nRW);
unsigned int MemAccess(unsigned int A, unsigned int V, int nRW, int S);
void PrintRegister(void);
void PrintMemory(unsigned int start, unsigned int end);

//�޸� ���� �Լ�
#endif /* MIPS_H_ */