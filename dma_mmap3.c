#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<fcntl.h>
#include<linux/ioctl.h>
#include<errno.h>
#include<math.h>
#include<sys/mman.h>
#define KYOUKO2_CONTROL_SIZE (65536)		//65536
#define KYOUKO2_DMA_SIZE 76
#define Device_RAM (0x0020)
#define GRAPHICS_ON 1
#define GRAPHICS_OFF 0
#define VMODE _IOW(0xCC,0,unsigned long) 
#define START_DMA _IOWR(0xCC, 2, unsigned long)
#define BIND_DMA _IOW(0xCC, 1, unsigned long)
#define SYNC _IO(0xCC,3)
#define FLUSH _IO(0xCC,4)
#define UNBIND_DMA _IO(0xCC,5)
#define STATUS 0x4008

#define CLR 0

#define R_PRIM 0x3000
#define R_EMIT 0x3004

#define D_XCOR 0x5000
#define D_YCOR 0x5004
#define D_ZCOR 0x5008
#define D_WCOR 0x500C

#define V_RCOL 0x5018
#define V_GCOL 0x5014
#define V_BCOL 0x5010
#define V_ALPH 0x501C

struct u_kyouko2_device{
	unsigned int *u_control_base;
	unsigned int *u_dma_base;
}kyouko2;

struct kyouko2_dma_hdr{
	uint32_t address:14;
	uint32_t count:10;
	uint32_t opcode:8;
}hdr;

unsigned int U_READ_REG(unsigned int rgister)
{
	return (*(kyouko2.u_control_base+(rgister>>2)));
}


void U_WRITE_REG(unsigned int rgister, unsigned int value)
{
	*(kyouko2.u_control_base+(rgister>>2))=value;

}
void U_WRITE_FL(unsigned int rgister, float fl)
{
	*(kyouko2.u_dma_base+rgister)=*(unsigned int *)&fl;
}
//Need a function like this to put values in the buffer once an array is given

void tri1(float *vert)
{

U_WRITE_FL(0,vert[0]);
U_WRITE_FL(1,vert[1]);
U_WRITE_FL(2,vert[2]);
U_WRITE_FL(3,vert[3]);
U_WRITE_FL(4,vert[4]);
U_WRITE_FL(5,vert[5]);
U_WRITE_FL(6,vert[6]);
U_WRITE_FL(7,vert[7]);
U_WRITE_FL(8,vert[8]);
U_WRITE_FL(9,vert[9]);
U_WRITE_FL(10,vert[10]);
U_WRITE_FL(11,vert[11]);
U_WRITE_FL(12,vert[12]);
U_WRITE_FL(13,vert[13]);
U_WRITE_FL(14,vert[14]);
U_WRITE_FL(15,vert[15]);
U_WRITE_FL(16,vert[16]);
U_WRITE_FL(17,vert[17]);
U_WRITE_FL(18,vert[18]);


return;
}


int main()
{
unsigned int arg;
int fd, result,i;
hdr.opcode=0x14;
hdr.count=3;
hdr.address=0x1045;

float *vert=(float *)malloc((3*6+1)*sizeof(float));
float *vert2=(float *)malloc((3*6+1)*sizeof(float));
//Header
vert[0]=vert2[0]=*(float *)&hdr;

//Color
vert[3]=vert[2]=vert2[2]=vert2[1]=0.0;
vert[1]=vert2[3]=1.0;


//Vertex:
vert[4]=vert[5]=vert2[4]=vert2[5]=-0.5;
vert[6]=vert2[6]=0.0;

//Set color
vert[7]=vert2[7]=0.0;
vert[8]=0.0;vert[9]=1.0;
vert2[8]=1.0;vert2[9]=1.0;

//Vertex:
vert[10]=vert2[10]=0.5;vert[11]=vert2[11]=0.0; vert[12]=vert2[12]=0.0;

//Set color
vert[15]=vert2[14]=0.0;
vert[14]=1.0;
vert[15]=0.0;
vert2[13]=1.0;

//Vertex:
vert[16]=vert2[16]=0.125;vert[18]=vert2[18]=0.5;vert[17]=vert2[17]=0.0;


fd=open("/dev/kyouko2", O_RDWR);
kyouko2.u_control_base = mmap(0,KYOUKO2_CONTROL_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);


int stat=U_READ_REG(STATUS);
stat&=0x02;
printf("status: %d\n",stat);

ioctl(fd,VMODE,GRAPHICS_ON);
	
ioctl(fd,BIND_DMA,&arg);

kyouko2.u_dma_base=arg;
	
printf("%x\n",kyouko2.u_dma_base);

tri1(vert);


ioctl(fd,SYNC);
ioctl(fd,FLUSH);

//This is the length of the data pushed to the buffer in bytes, calculate this dynamically, don't hard code like done here, &vert[18]-&vert[1]

arg=76;
ioctl(fd,START_DMA,&arg);
kyouko2.u_dma_base=(unsigned int *)arg;

sleep(2);

arg=76;
printf("%x\n",kyouko2.u_dma_base);

tri1(vert2);
ioctl(fd,SYNC);
ioctl(fd,FLUSH);

ioctl(fd,START_DMA,&arg);

kyouko2.u_dma_base=arg;
sleep(2);

arg=76;
tri1(vert);
ioctl(fd,SYNC);
ioctl(fd,FLUSH);
ioctl(fd,START_DMA,&arg);
kyouko2.u_dma_base=arg;
sleep(2);

tri1(vert2);
ioctl(fd,SYNC);
ioctl(fd,FLUSH);
arg=76;
ioctl(fd,START_DMA,&arg);
kyouko2.u_dma_base=arg;
sleep(2);


tri1(vert);
ioctl(fd,SYNC);
ioctl(fd,FLUSH);
arg=76;
ioctl(fd,START_DMA,&arg);
kyouko2.u_dma_base=arg;

sleep(2);

tri1(vert2);
ioctl(fd,SYNC);
ioctl(fd,FLUSH);
arg=76;
ioctl(fd,START_DMA,&arg);
kyouko2.u_dma_base=arg;

sleep(2);
tri1(vert);
ioctl(fd,SYNC);
ioctl(fd,FLUSH);
arg=76;
ioctl(fd,START_DMA,&arg);
kyouko2.u_dma_base=arg;

sleep(2);
tri1(vert2);
ioctl(fd,SYNC);
ioctl(fd,FLUSH);
arg=76;
ioctl(fd,START_DMA,&arg);
kyouko2.u_dma_base=arg;

sleep(2);
tri1(vert);
ioctl(fd,SYNC);
ioctl(fd,FLUSH);
arg=76;
ioctl(fd,START_DMA,&arg);
kyouko2.u_dma_base=arg;

sleep(2);

ioctl(fd,FLUSH);
ioctl(fd, UNBIND_DMA);
ioctl(fd,SYNC);
ioctl(fd,VMODE,GRAPHICS_OFF);

close(fd);

return 0;
}
