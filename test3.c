#include <stdio.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <stdlib.h>
 #include <netinet/in.h>
 #include <errno.h>
 #include <string.h>
 #include <arpa/inet.h>
 #include <unistd.h>
 #define MAXLINE 1024


#include <sys/types.h>  

#include <sys/stat.h>  
#include <fcntl.h>  
#include <termios.h>  
#include <stdio.h>  
#define BAUDRATE        B115200  
#define UART_DEVICE     "/dev/ttyS3"  //串口，待修改

#define FALSE  -1  
#define TRUE   0  


//added by wxn on 2080401
#include "include/CL1306.h"
#include "include/gpio.h"
#include "include/player.h"
#include "include/read.h"
#include "include/net.h"
#include "include/psam.h"
#include "include/display.h"
#include "include/keyboard.h"




//初始化读卡机系统
int System()
{
    char csn[256];
    unsigned char csnLen;
    int i;
    int ret;
    char ch;	

    if(Open_Frambuffer( "/dev/fb0")==MI_OK)
    {
	Init_Hzk();

	Set_Background(NULL,Color_blue,0);
	sleep(2);
	Set_Background(NULL,Color_red,0);
	//sleep(2);
	//Set_Background(NULL,Color_red,0);
	//sleep(2);
	//Set_Background("back.bmp",0,USE_BACK_IMG);
	//sleep(2);
	Set_Font_Color(Color_white);


	if(Insert_Hzk("/mnt/nand1-2/app/font/hzk16c_ASC.DZK",GB2312_16,HZK_MIXUER)==0)
		printf("load font library 16 succeed\n");
	if(Insert_Hzk("/mnt/nand1-2/app/font/hzk32c_ASC.DZK",GB2312_32,HZK_MIXUER)==0)
		printf("load font library 32 succeed\n");
	if(Insert_Hzk("./hzk16c_GBK.DZK",GBK_16,HZK_CHZ)==0)
		printf("load gbk font library 16 succeed\n");

	if(Insert_Hzk("/mnt/nand1-2/app/font/hzk24z_ASC.DZK",GB2312_24,HZK_CHZ)==0)
				printf("load Chinses font library 24 succeed\n");
	if(Insert_Hzk("/mnt/nand1-2/app/font/hzk24c_ASC.DZK",GB2312_24,HZK_ENZ)==0)
		printf("load English font library 24 succeed\n");

	printf("-----------------\n");

     }	
}


// 读卡
int test_read_block_m1()				//do not need to verify the key
{
	char secbuff[16];
	unsigned char len;	
	int i;
	struct card_buf temkey;
	int secter=5;
	int block=1;
	int in;
	
	temkey.mode=KEYA;
	memset(temkey.rwbuf,0xff,16);
	memset(temkey.money,0,4);
	DBG_PRINTF("%s\n",__func__);
	memset(temkey.key,0xff,6);

	int ret;
	char csn[32];
	int csnLen;
	
	Clear_Display();
#if CH
	TextOut(20,50,"请将卡片放在刷卡区",GB2312_32);
#else	
	TextOut(80,50,"Please put card",GB2312_24);
	TextOut(30,80,"at card reading area",GB2312_24);
#endif
	while(1){

			ret=Get_KeyCode();
			if(ret==4)
				return 0;
			if((ret=CardReset(csn,&csnLen))== 0x08)
				break;
	}

	
	if(ReadOneSectorDataFromCard(secbuff,&len,secter,block,WRITE_KEY,temkey.key,temkey.mode)==MI_OK)
		{

				printf("read sector len=%d\n",len);
				for(i=0;i<len;i++)
				{
					printf("%02x ",secbuff[i]);
				}
				printf("\n");
				Clear_Display();
				Set_Font_Color(Color_white);
				Show_Bmp(0,0,&bmp);
#if CH
				TextOut(100,100,"读卡成功",GB2312_32);	
#else				
				TextOut(50,100,"read card succeed",GB2312_24);
#endif			
				//Set_Background("./res/make.bmp",0,USE_BACK_IMG);
				sleep(1);
				buzz_on();
				sleep(1);
				buzz_off();
				
	}
	else 
		{
			printf(" read card erro\n");
			Clear_Display();
			//Set_Font_Color(Color_red);
			Show_Bmp(0,0,&bmp);
#if CH
			TextOut(100,100,"读卡失败",GB2312_32);
#else			
			TextOut(50,100,"read card failed",GB2312_24);
#endif		
			sleep(1);
			buzz_on();
			usleep(500000);
			buzz_off();
			buzz_on();
			usleep(500000);
			buzz_off();
			return -1;
		}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////  
/** 
*@brief  设置串口通信速率 
*@param  fd     类型 int  打开串口的文件句柄 
*@param  speed  类型 int  串口速度 
*@return  void 
*/  
int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,  
               B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300, };  
int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,   
              115200, 38400, 19200, 9600, 4800, 2400, 1200,  300, };  
void set_speed(int fd, int speed){  
int   i;   
int   status;   
struct termios   Opt;  
tcgetattr(fd, &Opt);   
for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {   
if  (speed == name_arr[i]) {       
  tcflush(fd, TCIOFLUSH);       
  cfsetispeed(&Opt, speed_arr[i]);    
  cfsetospeed(&Opt, speed_arr[i]);     
  status = tcsetattr(fd, TCSANOW, &Opt);    
  if  (status != 0) {          
    perror("tcsetattr fd1");    
    return;       
  }      
  tcflush(fd,TCIOFLUSH);     
}    
}  
}  
////////////////////////////////////////////////////////////////////////////////  
/** 
*@brief   设置串口数据位，停止位和效验位 
*@param  fd     类型  int  打开的串口文件句柄 
*@param  databits 类型  int 数据位   取值 为 7 或者8 
*@param  stopbits 类型  int 停止位   取值为 1 或者2 
*@param  parity  类型  int  效验类型 取值为N,E,O,,S 
*/  
int set_Parity(int fd,int databits,int stopbits,int parity)  
{   
struct termios options;   
if  ( tcgetattr( fd,&options)  !=  0) {   
    perror("SetupSerial 1");       
    return(FALSE);    
}  
options.c_cflag &= ~CSIZE;   
switch (databits) /*设置数据位数*/  
{     
case 7:       
    options.c_cflag |= CS7;   
    break;  
case 8:       
    options.c_cflag |= CS8;  
    break;     
default:      
    fprintf(stderr,"Unsupported data size\n"); return (FALSE);    
}  
switch (parity)   
{     
    case 'n':  
    case 'N':      
        options.c_cflag &= ~PARENB;   /* Clear parity enable */  
        options.c_iflag &= ~INPCK;     /* Enable parity checking */   
        break;    
    case 'o':     
    case 'O':       
        options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/    
        options.c_iflag |= INPCK;             /* Disnable parity checking */   
        break;    
    case 'e':    
    case 'E':     
        options.c_cflag |= PARENB;     /* Enable parity */      
        options.c_cflag &= ~PARODD;   /* 转换为偶效验*/       
        options.c_iflag |= INPCK;       /* Disnable parity checking */  
        break;  
    case 'S':   
    case 's':  /*as no parity*/     
        options.c_cflag &= ~PARENB;  
        options.c_cflag &= ~CSTOPB;break;    
    default:     
        fprintf(stderr,"Unsupported parity\n");      
        return (FALSE);    
    }    
/* 设置停止位*/    
switch (stopbits)  
{     
    case 1:      
        options.c_cflag &= ~CSTOPB;    
        break;    
    case 2:      
        options.c_cflag |= CSTOPB;    
       break;  
    default:      
         fprintf(stderr,"Unsupported stop bits\n");    
         return (FALSE);   
}   
/* Set input parity option */   
if (parity != 'n')     
    options.c_iflag |= INPCK;   
tcflush(fd,TCIFLUSH);  
options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/     
options.c_cc[VMIN] = 0; /* Update the options and do it NOW */  
if (tcsetattr(fd,TCSANOW,&options) != 0)     
{   
    perror("SetupSerial 3");     
    return (FALSE);    
}   
options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/  
options.c_oflag  &= ~OPOST;   /*Output*/  
return (TRUE);    
}  
////////////////////////////////////////////////////////////////////////////////  
int readNum()  
{  

int    fd, c=0, res;  

char  buf[256];  

printf("Start...\n");  
fd = open(UART_DEVICE, O_RDWR);  

if (fd < 0) {  
    perror(UART_DEVICE);  
    exit(1);  
}  

printf("Open...\n");  
set_speed(fd,115200);  
if (set_Parity(fd,8,1,'N') == FALSE)  {  
    printf("Set Parity Error\n");  
    exit (0);  
}  

printf("Reading...\n");  
while(1) {  
    res = read(fd, buf, 255);  

    if(res==0)  
        continue;  
    buf[res]=0;  

    printf("%s", buf);  
      
    if (buf[0] == 0x0d)  
        printf("\n");  
      
    if (buf[0] == '@') break;  
}  

printf("Close...\n");  
close(fd);  

return 0;  
}  




//判断socket是否断了
 bool IsSocketClosed(int clientSocket)  
{  
 char buff[32];  
 int recvBytes = recv(clientSocket, buff, sizeof(buff), MSG_PEEK);  
   
 int sockErr = errno;  
   
 //cout << "In close function, recv " << recvBytes << " bytes, err " << sockErr << endl;  
   
 if( recvBytes > 0) //Get data  
  return false;  
   
 if( (recvBytes == -1) && (sockErr == EWOULDBLOCK) ) //No receive data  
  return false;  
     
 return true;  
}  




 int main(int argc,char **argv)
 {
 char *servInetAddr = "192.168.1.119";
 int socketfd;
 struct sockaddr_in sockaddr;
 char recvline[MAXLINE], sendline[MAXLINE];
 int n;
//
// if(argc != 2)
// {
// printf("client <ipaddress> \n");
// exit(0);
// }
//
 socketfd = socket(AF_INET,SOCK_STREAM,0);
 memset(&sockaddr,0,sizeof(sockaddr));
 sockaddr.sin_family = AF_INET;
 sockaddr.sin_port = htons(6666);
 inet_pton(AF_INET,servInetAddr,&sockaddr.sin_addr);

//判断是否链接
 if((connect(socketfd,(struct sockaddr*)&sockaddr,sizeof(sockaddr))) < 0 )
 {
 printf("connect error %s errno: %d\n",strerror(errno),errno);
 exit(0);
 }

 printf("send message to server\n");

 fgets(sendline,1024,stdin);

//判断是否发数据
 if((send(socketfd,sendline,strlen(sendline),0)) < 0)
 {
 printf("send mes error: %s errno : %d",strerror(errno),errno);
 exit(0);
 }
 


readNum(); 
 //close(socketfd);
 //printf("exit\n");
 //exit(0);
 }
