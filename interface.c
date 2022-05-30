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
    RegAccess(29,addr,1);
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
            MemAccess(0x400000+(i*4),*buff,1,2);
            printf("%d ",i);  //디버깅
        }
    }

    printf("\n");  //디버깅

    for(int i=0;i<dataNum;i++){
        if(fread(buff,sizeof(int),1,filePointer)==0){  //데이터 갯수만큼 Memory[0x10000000]부터 순서대로 Write
            perror("File read error!\n");
            isExecutable=FALSE;
            return 1;
        }
        else{
            MemAccess(0x10000000+(i*4),*buff,1,2);
            printf("%d ",i);  //디버깅
        }
    }

    printf("\n");  //디버깅

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
    IR.I=invertEndian(MemAccess(PC,0,0,2));  //word단위로 PC에 위치한 명령어 하나 읽기
    int z=0;  //Zero Flag (결과값 0: 1 / 결과값 !=0: 0)
    int MEMoutput;  //메모리 load 값
    long product=0;  //mult용 register (64bits)

    printf("%0.8x\n",IR.I);

    if(ALU(IR.I,PC)==0)    isEnd=TRUE;
    else    isEnd=FALSE;
    ViewRegister();
}

/*
    void setRegister(int regNum,int value)

인자로 레지스터 번호(regNum)와 저장할 값(value)을 받음
해당 레지스터에 값을 저장함
*/
void setRegister(int regNum,int value){
    RegAccess(regNum,value,1);
    printf("%d %d\n",regNum,value);
}

/*
    void setMemory(unsigned int location,int value)

인자로 메모리 주소(location)와 저장할 값(value)을 받음
해당 메모리 위치에 값을 저장함
*/
void setMemory(unsigned int location,int value){
    MemAccess(location,value,1,2);
    printf("%0.8x %d\n",location,value);
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
    while(TRUE){
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
            ViewMemory(arg1,arg2);
        }
        else if(strcmp(cmd,"r")==0 && isExecutable==TRUE){  //현재 레지스터 내용 출력 (Command Format: r)
            ViewRegister();
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