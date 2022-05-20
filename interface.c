#include <stdlib.h>
#include <string.h>
#include "interface.h"

extern unsigned int PC;
extern int isEnd,isExecutable;
extern FILE *filePointer;

/*
    setPC(unsigned int addr)

인자로 받은 addr로 PC값을 업데이트함
*/
void setPC(unsigned int addr){
    PC=addr;
}

/*
     setSP(unsigned int addr)

인자로 받은 addr 로 SP값(Resgister 29번)을 업데이트함
*/
void setSP(unsigned int addr){
    //writeReg(29,addr);
}

/*
    int loadProgram(FILE *filePointer)

인자로 파일스트림 filePointer를 받고 filePointer가 가리키는 바이너리 파일에 대해 작업을 함
PC값을 0x400000으로, SP값을 0x80000000으로, isEnd값을 FALSE로 초기화
바이너리 파일의 첫 4byte에서 명령어 갯수를, 두번째 4byte에서 데이터 갯수를 가져옴
이후, 메모리의 0x400000부터 차례대로 명령어를 저장하고 0x10000000부터 차례대로 데이터를 저장함
이 과정에서 error발생 시 isExcutable을 FALSE로 설정하고 함수 종료
*/
int loadProgram(FILE *filePointer){
    int instNum,dataNum;
    int buff[1];
    //initReg();  //레지스터 초기화
    //initMEM();  //메모리 초기화
    setPC(0x400000);  //PC 초기화
    setSP(0x80000000);  //SP 초기화
    isEnd=FALSE;  //프로그램 종료 상태 변수 초기화

    if(fread(buff,sizeof(int),1,filePointer)==0){  //바이너리 파일의 첫 4byte == 명령어의 갯수
        perror("File read error!\n");
        isExecutable=FALSE;
        return 1;
    }
    instNum=invertEndian(*buff);

     if(fread(buff,sizeof(int),1,filePointer)==0){  //바이너리 파일의 두번째 4byte == 데이터 갯수
        perror("File read error!\n");
        isExecutable=FALSE;
        return 1;
     }
    dataNum=invertEndian(*buff);

    for(int i=0;i<instNum;i++){
        if(fread(buff,sizeof(int),1,filePointer)==0){  //명령어 갯수만큼 Memory[0x400000]부터 순서대로 Write
            perror("File read error!\n");
            isExecutable=FALSE;
            return 1;
        }
        else{
            //MEM(0x400000+(i*4),*buff,1,2);
        }
    }

    for(int i=0;i<dataNum;i++){
        if(fread(buff,sizeof(int),1,filePointer)==0){  //데이터 갯수만큼 Memory[0x10000000]부터 순서대로 Write
            perror("File read error!\n");
            isExecutable=FALSE;
            return 1;
        }
        else{
            //MEM(0x10000000+(i*4),*buff,1,2);
        }
    }

    return 0;
}

/*
    void jumpProgram(unsigned int startPosition)

인자로 jump할 메모리 주소 startPosition을 받음
이 메모리 주소가 유효하지 않으면 수정하고 PC값을 해당 값으로 업데이트
jump할 메모리 주소가 Text영역을 벗어나는 경우 error
*/
void jumpProgram(unsigned int startPosition){
    if(startPosition%4!=0) startPosition=startPosition&0xfffffffc;  //올바른 메모리 주소가 아닌 경우 올바른 주소로 고침

    if((startPosition>>20)==4){
        setPC(startPosition);
        printf("PC is setted at 0x%x\n\n",PC);
    }
    else
        printf("Error: Worng Access!\n\n");  //PC 설정 값이 메모리 Text영역이 아니면 Error
}

/*
    void goProgram(void)

isEnd가 TRUE가 될 때까지 step()함수를 실행
loop를 빠져 나오면 isExcutable을 FALSE로 설정 후 프로그램 종료 문구 출력
*/
void goProgram(void){
    while(isEnd==FALSE) step();  //syscall 10을 만나기 전까지 step()을 계속 호출한다.
    isExecutable=FALSE;
    printf("-----Program End-----\n\n");
}

/*
    void step(void)

PC가 가리키는 메모리 주소에서 명령어를 1word 받음
이후 명령어를 Decode하고 그 결과대로 명령어를 실행
*/
void step(void){
    IR.I=invertEndian(MEM(PC,0,0,2));  //word단위로 PC에 위치한 명령어 하나 읽기
    int z=0;  //Zero Flag (결과값 0: 1 / 결과값 !=0: 0)
    int MEMoutput;  //메모리 load 값
    long product=0;  //mult용 register (64bits)

    printf("%0.8x\n",IR.I);

    if(IR.RI.opcode==0){
        switch(IR.RI.funct){
            case 0:  //shift logic left
                printf("sll   $%d, $%d, $%d\n",IR.RI.rd,IR.RI.rt,IR.RI.sh);
                //if(writeReg(IR.RI.rd,ALU(readReg(IR.RI.rt),IR.RI.sh,"SLL",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 2:  //shift arithmetic left
                printf("srl   $%d, $%d, $%d\n",IR.RI.rd,IR.RI.rt,IR.RI.sh);
                //if(writeReg(IR.RI.rd,ALU(readReg(IR.RI.rt),IR.RI.sh,"SRL",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 3:  //shift arithmetic right
                printf("sra   $%d, $%d, $%d\n",IR.RI.rd,IR.RI.rt,IR.RI.sh);
                //if(writeReg(IR.RI.rd,ALU(readReg(IR.RI.rt),IR.RI.sh,"SRA",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 8:  //jump to register
                printf("jr    $%d\n",IR.RI.rs);
                //setPC(readReg(IR.RI.rs));
                viewRegister();
                break;

            case 12:  //syscall
                printf("syscall %d\n",10);
                //printf("syscall %d\n",readReg(2));
                //if(readReg(2)==10){  //syscall 10이면 program exit
                //    isEnd=TRUE;
                //    viewRegister();
                //    printf("Program is End\n");
                //    break;
                //}
                setPC(PC+4);
                viewRegister();
                isExecutable=FALSE;
                break;

            case 16:  //mfhi
                printf("mfhi  $%d\n",IR.RI.rd);
                //if(writeReg(IR.RI.rd,readReg("HI"))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 18:  //mflo
                printf("mfhi  $%d\n",IR.RI.rd);
                //if(writeReg(IR.RI.rd,readReg("LO"))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 24:  //multiply
                printf("mult   $%d, $%d\n",IR.RI.rs,IR.RI.rt);
                //product=readReg(IR.RI.rt)<<32;
                //for(int i=0;i<32;i++){
                //    if((product&0x1)==1)    product=(ALU(product>>32,readReg(IR.RI.rs),"ADD",&z)<<32)|(product&0xffffffff);
                //    product=product>>1;
                //}
                //if(writeReg("LO",product&0xffffffff)==1)
                //    printf("Error: Instruction fail!\n");
                //if(writeReg("HI",product>>32))
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 32:  //add
                printf("add   $%d, $%d, $%d\n",IR.RI.rd,IR.RI.rs,IR.RI.rt);
                //if(writeReg(IR.RI.rd,ALU(readReg(IR.RI.rs),readReg(IR.RI.rt),"ADD",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 34:  //sub
                printf("sub   $%d, $%d, $%d\n",IR.RI.rd,IR.RI.rs,IR.RI.rt);
                //if(writeReg(IR.RI.rd,ALU(readReg(IR.RI.rs),readReg(IR.RI.rt),"SUB",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 36:  //and
                printf("and   $%d, $%d, $%d\n",IR.RI.rd,IR.RI.rs,IR.RI.rt);
                //if(writeReg(IR.RI.rd,ALU(readReg(IR.RI.rs),readReg(IR.RI.rt),"AND",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 37:  //or
                printf("or    $%d, $%d, $%d\n",IR.RI.rd,IR.RI.rs,IR.RI.rt);
                //if(writeReg(IR.RI.rd,ALU(readReg(IR.RI.rs),readReg(IR.RI.rt),"OR",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 38:  //exclusive or
                printf("xor   $%d, $%d, $%d\n",IR.RI.rd,IR.RI.rs,IR.RI.rt);
                //if(writeReg(IR.RI.rd,ALU(readReg(IR.RI.rs),readReg(IR.RI.rt),"XOR",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 39:  //or not
                printf("nor   $%d, $%d, $%d\n",IR.RI.rd,IR.RI.rs,IR.RI.rt);
                //if(writeReg(IR.RI.rd,ALU(readReg(IR.RI.rs),readReg(IR.RI.rt),"NOR",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 42:  //set less than
                printf("slt   $%d, $%d, $%d\n",IR.RI.rd,IR.RI.rs,IR.RI.rt);
                //if(ALU(readReg(IR.RI.rs),readReg(IR.RI.rt),"SUB",&z)<0)
                //    writeReg(IR.RI.rd,1);
                //else
                //    writeReg(IR.RI.rd,0);
                setPC(PC+4);
                viewRegister();
                break;

            default:  //정의되지 않은 명령어
                printf("Error: Undefined Instruction!\n");
                setPC(PC+4);
                break;
        }
    }
    else{
        switch(IR.RI.opcode){
            case 1:  //branch less than zero
                printf("bltz  $%d, %d\n",IR.RI.rs,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct));
                //if(ALU(readReg(IR.RI.rs),readReg(0),"SUB",&z)<0)
                //    setPC(PC+((IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct))*4);
                //else
                //    setPC(PC+4);
                viewRegister();
                break;
            
            case 2:  //jump
                printf("j     %d\n",(IR.RI.rs<<21)|(IR.RI.rt<<16)|(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct));
                setPC(((IR.RI.rs<<21)|(IR.RI.rt<<16)|(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct))*4);
                viewRegister();
                break;

            case 3:  //jump and link
                printf("jal   %d\n",(IR.RI.rs<<21)|(IR.RI.rt<<16)|(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct));
                //writeReg(31,PC+4);
                setPC(((IR.RI.rs<<21)|(IR.RI.rt<<16)|(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct))*4);
                viewRegister();
                break;

            case 4:  //branch equal
                printf("beq   $%d, $%d, %d\n",IR.RI.rs,IR.RI.rt,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct));
                //ALU(readReg(IR.RI.rs),readReg(IR.RI.rt),"SUB",&z);
                //if(z==0)
                //    setPC(PC+((IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct))*4);
                //else
                //    setPC(PC+4);
                viewRegister();
                break;

            case 5:  //branch not equal
                printf("bne   $%d, $%d, %d\n",IR.RI.rs,IR.RI.rt,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct));
                //ALU(readReg(IR.RI.rs),readReg(IR.RI.rt),"SUB",&z);
                //if(z!=0)
                //    setPC(PC+((IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct))*4);
                //else
                //    setPC(PC+4);
                viewRegister();
                break;

            case 8:  //add immediate value
                printf("addi  $%d, $%d, 0x%x\n",IR.RI.rt,IR.RI.rs,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct));
                //if(writeReg(IR.RI.rt,ALU(readReg(IR.RI.rs),(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),"ADD",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 10:  //set less than immediate value
                printf("slti  $%d, $%d, 0x%x\n",IR.RI.rt,IR.RI.rs,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct));
                //if(ALU(readReg(IR.RI.rs),(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),"SUB",&z)<0)  //에러 체크
                //    writeReg(IR.RI.rt,1);
                //else
                //    writeReg(IR.RI.rt,0);
                setPC(PC+4);
                viewRegister();
                break;

            case 12:  //and immediate value
                printf("andi  $%d, $%d, 0x%x\n",IR.RI.rt,IR.RI.rs,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct));
                //if(writeReg(IR.RI.rt,ALU(readReg(IR.RI.rs),(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),"AND",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 13:  //or immediate value
                printf("ori   $%d, $%d, 0x%x\n",IR.RI.rt,IR.RI.rs,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct));
                //if(writeReg(IR.RI.rt,ALU(readReg(IR.RI.rs),(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),"OR",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 14:  //exclusive or immediate value
                printf("xori  $%d, $%d, 0x%x\n",IR.RI.rt,IR.RI.rs,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct));
                //if(writeReg(IR.RI.rt,ALU(readReg(IR.RI.rs),(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),"XOR",&z))==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 15:  //load upper immediate value
                printf("lui   $%d, 0x%x\n",IR.RI.rt,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct));
                //if(writeReg(IR.RI.rt,((IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct))<<16)==1)  //에러 체크
                //    printf("Error: Instruction fail!\n");
                setPC(PC+4);
                viewRegister();
                break;

            case 32:  //load byte
                printf("lb    $%d, %d($%d)\n",IR.RI.rt,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),IR.RI.rs);
                //MEMoutput=MEM(readReg(IR.RI.rs)+(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),0,0,0);
                //if((MEMoutput&0xa0)>>7)  MEMoutput=MEMoutput|0xffffff00;  //MEMoutput이 음수면 음수로 변환
                //writeReg(IR.RI.rt,MEMoutput);
                setPC(PC+4);
                viewRegister();
                break;

            case 35:  //load word
                printf("lw    $%d, %d($%d)\n",IR.RI.rt,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),IR.RI.rs);
                //writeReg(IR.RI.rt,MEM(readReg(IR.RI.rs)+(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),0,0,2));
                setPC(PC+4);
                viewRegister();
                break;

            case 36:  //load byte unsigned
                printf("lbu   $%d, %d($%d)\n",IR.RI.rt,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),IR.RI.rs);
                //writeReg(IR.RI.rt,MEM(readReg(IR.RI.rs)+(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),0,0,0));
                setPC(PC+4);
                viewRegister();
                break;

            case 40:  //store byte
                printf("sb    $%d, %d($%d)\n",IR.RI.rt,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),IR.RI.rs);
                //MEM(readReg(IR.RI.rs)+(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),readReg(IR.RI.rt),1,0);
                setPC(PC+4);
                viewRegister();
                break;

            case 43:  //store word
                printf("sw    $%d, %d($%d)\n",IR.RI.rt,(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),IR.RI.rs);
                //MEM(readReg(IR.RI.rs)+(IR.RI.rd<<11)|(IR.RI.sh<<6)|(IR.RI.funct),readReg(IR.RI.rt),1,2);
                setPC(PC+4);
                viewRegister();
                break;

            default:  //정의되지 않은 명령어
                printf("Error: Undefined Instruction!\n");
                setPC(PC+4);
                break;
        }
    }
}

/*
    void viewMemory(unsigned int start,unsigned int end)

인자로 메모리 시작 주소(start)와 끝 주소(end)를 받음
메모리 시작 주소부터 끝 주소까지 메모리에 저장된 값을 보여줌
*/
void viewMemory(unsigned int start,unsigned int end){
    printf("\n-----[Memory]-----\n");
    for(;start<=end;start++)    //printf("MEM[0x%0.8x]=%0.8x\n",start,MEM(start,0,0,2));
    printf("-----------------\n\n");
}

/*
    void viewRegister(void)

레지스터에 저장된 값들과 PC값을 보여줌
*/
void viewRegister(void){
    printf("\n-----[REGISTER]-----\n");
    for(int i=0;i<32;i++)   //printf("R%d=%0.8x\n",i,readReg(i));
    printf("PC=%0.8x\n",PC);
    printf("--------------------\n\n");
}

/*
    void setRegister(int regNum,int value)

인자로 레지스터 번호(regNum)와 저장할 값(value)을 받음
해당 레지스터에 값을 저장함
*/
void setRegister(int regNum,int value){
    //writeReg(regNum,value);
}

/*
    void setMemory(unsigned int location,int value)

인자로 메모리 주소(location)와 저장할 값(value)을 받음
해당 메모리 위치에 값을 저장함
*/
void setMemory(unsigned int location,int value){
    //MEM(location,value,1,2);
}

/*
    unsigned int invertEndian(unsigned int inputValue)

인자로 받은 inputValue의 Endian을 뒤바꾼 후 return
*/
unsigned int invertEndian(unsigned int inputValue){
    unsigned int first,second,third,fourth;
                                           //inputValue:0x@@$$%%&&
    first=inputValue>>24;                  //0x@@****** ----CONVERT----> 0x000000@@
    second=(inputValue>>8)&0xff00;         //0x**$$**** ----CONVERT----> 0x0000$$00
    third=(inputValue<<8)&0xff0000;        //0x****%%** ----CONVERT----> 0x00%%0000
    fourth=inputValue<<24;                 //0x******&& ----CONVERT----> 0x&&000000
    inputValue=first|second|third|fourth;  //Result:0x&&%%$$@@

    return inputValue;
}

/*
    int interface(void)

사용자에게서 command를 받고 해당 command대로 작업을 수행함
command가 잘못된 format이거나 지원하지 않는 command일 시 error
또는 프로그램이 실행 불능 상태일 때 error
*/
int interface(void){
    char command[20];  //User에게 입력받는 Command
    char *cmd;  //Command의 명령어 부분
    unsigned int arg1,arg2;  //arguments
    filePointer=NULL;  //파일 포인터 초기화

    printf("-----Simulator Start-----\n");
    printf("Supported Command: l/j/g/s/m/r/x/sr/sm\n");
    while(1){
        printf("Enter Command: ");
        gets(command);
        cmd=strtok(command," ");
        
        if(strcmp(cmd,"l")==0){  //프로그램을 메모리로 Load (Command Format: l XXXXX.bin)
            if((filePointer=fopen(strtok(NULL," "),"r"))==NULL){
                perror("File open Error!");
            }
            else{
                loadProgram(filePointer);
                isExecutable=TRUE;
                fclose(filePointer);
            }
        }
        else if(strcmp(cmd,"j")==0 && isExecutable==TRUE){  //프로그램 시작 위치 설정 (Command Format: j 0x########)
            arg1=strtol(strtok(NULL," "),NULL,16);
            jumpProgram(arg1);
        }
        else if(strcmp(cmd,"g")==0 && isExecutable==TRUE){  //현재 위치에서 프로그램 끝까지 실행 (Command Format: g)
            goProgram();
        }
        else if(strcmp(cmd,"s")==0 && isExecutable==TRUE){  //프로그램 명령어 하나 실행 (Command Format: s)
            step();
        }
        else if(strcmp(cmd,"m")==0 && isExecutable==TRUE){  //start ~ end 범위의 메모리 내용 출력 (Command Format: m <start> <end>)
            arg1=strtol(strtok(NULL," "),NULL,16);
            arg2=strtol(strtok(NULL," "),NULL,16);
            viewMemory(arg1,arg2);
        }
        else if(strcmp(cmd,"r")==0 && isExecutable==TRUE){  //현재 레지스터 내용 출력 (Command Format: r)
            viewRegister();
        }
        else if(strcmp(cmd,"x")==0){  //시뮬레이터 종료 (Command Format: x)
            printf("-----Simulator End-----\n");
            return 0;
        }
        else if(strcmp(cmd,"sr")==0 && isExecutable==TRUE){  //특정 레지스터 값 설정 (Command Format: sr <register number> <value>)
            arg1=strtol(strtok(NULL," "),NULL,10);
            arg2=strtol(strtok(NULL," "),NULL,10);
            setRegister(arg1,arg2);
        }
        else if(strcmp(cmd,"sm")==0 && isExecutable==TRUE){  //특정 메모리 위치의 값 설정 (Command Format: sm <location> <value>)
            arg1=strtol(strtok(NULL," "),NULL,16);
            arg2=strtol(strtok(NULL," "),NULL,10);
            setMemory(arg1,arg2);
        }
        else{
            if(isExecutable==FALSE)  //프로그램이 실행 불능 상태일 때
                if(isEnd==TRUE){  //프로그램이 종료 상태
                    printf("-----Program End-----\n");
                }
                else{  //프로그램이 실행 불가 상태
                    printf("Error: Program is unexecutable!\n");
                }
            else  //시뮬레이터에서 지원하지 않는 command
                printf("Error: Unsupported command!\n");
        }
    }
    
    return 0;
}