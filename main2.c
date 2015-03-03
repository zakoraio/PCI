#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <sys/mman.h>
#include <linux/ioctl.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define	VTX_COORD4F 0x5000 
#define	VTX_COLOR4F 0x5010
#define VMODE _IOW(0xcc,0,unsigned long)
#define BIND_DMA _IOW(0xcc,1,unsigned long)
#define START_DMA _IOWR(0xcc,2,unsigned long)
#define UNBIND_DMA _IO(0xcc,5)
#define SYNC _IO(0xcc,3)
#define FLUSH _IO(0xcc,4)
#define GRAPHICS_ON 1
#define GRAPHICS_OFF 0
#define RASTER_PRIMITIVE 0x3000
#define	RASTER_EMIT 0x3004 
#define	RASTER_CLEAR 0x3008 
#define	RASTER_TARGET_FRAME 0x3100d
#define	RASTER_FLUSH 0x3FFC
#define	CFG_REBOOT 0x1000
#define	FIFO_DEPTH 0x4004 

struct u_kyouko2_device {
	unsigned int *u_control_base;
	unsigned int *u_fb_base;
}kyouko2;


struct kyouko2_dma_hdr{
	uint32_t address:14;
	uint32_t count:10;
	uint32_t opcode:8;
}hdr;



#define	KYOUKO_CONTROL_SIZE (65536)		
#define	Device_Ram (0x0020)		
unsigned long arg;
unsigned int countByte;

unsigned int U_READ_REG(unsigned int reg){
	return (*(kyouko2.u_control_base + (reg >> 2)));
}

void U_WRITE_FB(unsigned int reg, unsigned int value){
	*(kyouko2.u_fb_base + reg) = value;
}

void U_WRITE_REG(unsigned int reg, unsigned int value){
	*(kyouko2.u_control_base + (reg >> 2)) = value;
}

void u_sync(void){
	while (U_READ_REG(FIFO_DEPTH)>0);
}




float verticesX[30000][3];
float verticesY[30000][3];
float colors[30000][3][3];


/*
float verticesX[9000][3];
float verticesY[9000][3];
float colors[9000][3][3];
*/


struct Coord
{
	float x, y;
};

struct Coord mid(struct Coord a, struct Coord b)
{
	struct Coord c;
	c.x = (a.x + b.x) / 2;
	c.y = (a.y + b.y) / 2;
	return c;
}

float dist(struct Coord a, struct Coord b)
{
	return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
}


float scale(const float valueIn, const float baseMin, const float baseMax, const float limitMin, const float limitMax) {
	return ((limitMax - limitMin) * (valueIn - baseMin) / (baseMax - baseMin)) + limitMin;
}


unsigned int rand_interval(unsigned int min, unsigned int max)
{
	return (rand() % (max + 1 - min)) + min;
}


static int index = -1;
static int arrayIndex = 0;
static int limit = 1;


void drawSierpinski(int iter, struct Coord a, struct Coord b, struct Coord c)
{

	if (iter >= limit)
	{
		return;
	}
	else
	{
		drawSierpinski(iter + 1, a, mid(a, b), mid(a, c));
		index++;
		verticesX[index][0] = a.x;
		verticesX[index][1] = mid(a, b).x;
		verticesX[index][2] = mid(a, c).x;
		verticesY[index][0] = a.y;
		verticesY[index][1] = mid(a, b).y;
		verticesY[index][2] = mid(a, c).y;
		int i, j;
		for (i = 0; i<3; i++){
			for (j = 0; j<3; j++){
				colors[index][i][j] = scale(rand_interval(0, 255), 0, 255, 0, 1);
			}
		}
		drawSierpinski(iter + 1, mid(a, b), b, mid(b, c));
		index++;
		verticesX[index][0] = mid(a, b).x;
		verticesX[index][1] = b.x;
		verticesX[index][2] = mid(b, c).x;
		verticesY[index][0] = mid(a, b).y;
		verticesY[index][1] = b.y;
		verticesY[index][2] = mid(b, c).y;

		for (i = 0; i<3; i++){
			for (j = 0; j<3; j++){
				colors[index][i][j] = scale(rand_interval(0, 255), 0, 255, 0, 1);
			}
		}

		drawSierpinski(iter + 1, mid(a, c), mid(b, c), c);
		index++;
		verticesX[index][0] = mid(a, c).x;
		verticesX[index][1] = mid(b, c).x;
		verticesX[index][2] = c.x;
		verticesY[index][0] = mid(a, c).y;
		verticesY[index][1] = mid(b, c).y;
		verticesY[index][2] = c.y;

		for (i = 0; i<3; i++){
			for (j = 0; j<3; j++){
				colors[index][i][j] = scale(rand_interval(0, 255), 0, 255, 0, 1);
			}
		}
	}

}



struct Coord top, left, right;
void initSierpinski(){
left.x = -1;
left.y = 1;
top.x = 0;
top.y = -1;
right.x = 1;
right.y = 1;
}



void draw_fifo(int fd){

	float one = 1.0;
	float zero = 0.0;
	int tri = 0, i = 0, j = 0;
	
	initSierpinski();
	limit = 8;
	
	drawSierpinski(0, left, top, right);
	
	printf("index = %d", index);

	U_WRITE_REG(RASTER_PRIMITIVE, 1);
	for (i = index - 1; i >= 0; i--){

		U_WRITE_REG(RASTER_PRIMITIVE, 1);
		ioctl(fd, SYNC);
		for (j = 0; j<3; j++){
			printf("(%f , %f)", verticesX[i][j], verticesY[i][j]);
			printf("Color = %f , %f  , %f", colors[i][0], colors[i][1], colors[i][2]);
			U_WRITE_REG(VTX_COORD4F, *(unsigned int*)&verticesX[i][j]);

			U_WRITE_REG(VTX_COORD4F + 4, *(unsigned int*)&verticesY[i][j]);

			U_WRITE_REG(VTX_COORD4F + 8, *(unsigned int*)&zero);

			U_WRITE_REG(VTX_COORD4F + 12, *(unsigned int*)&one);

			U_WRITE_REG(VTX_COLOR4F, *(unsigned int*)&colors[i][j][0]);


			U_WRITE_REG(VTX_COLOR4F + 4, *(unsigned int*)&colors[i][j][1]);

			U_WRITE_REG(VTX_COLOR4F + 8, *(unsigned int*)&colors[i][j][2]);

			U_WRITE_REG(VTX_COLOR4F + 12, *(unsigned int*)&zero);

			U_WRITE_REG(RASTER_EMIT, 0);
		}

		U_WRITE_REG(RASTER_PRIMITIVE, 0);

		U_WRITE_REG(RASTER_FLUSH, 1);

	}
}



int draw_dma(int arrayIndex, int bulkCount, int ch){
	
	hdr.opcode = 0x14;
	if(ch == 4){
	if(arrayIndex - bulkCount < 0)
	hdr.count = arrayIndex*3;
	else
	hdr.count = bulkCount*3;
	}
	else
	hdr.count = 3;
	hdr.address = 0x1045;
	int j = 0;
	float zero = 0.0;
	countByte = 0;
	unsigned int* buf = (unsigned int*)(arg);
	buf[countByte++] = *(unsigned int*)&hdr;
	int k = arrayIndex-bulkCount+1;
	if( ch == 4 )
	k = arrayIndex;
	    for(arrayIndex=k;arrayIndex >= k-bulkCount ;arrayIndex--){
		for (j = 0; j<3; ++j){
			buf[countByte++] = *(unsigned int*)&(colors[arrayIndex][j][0]);
			buf[countByte++] = *(unsigned int*)&(colors[arrayIndex][j][1]);
			buf[countByte++] = *(unsigned int*)&(colors[arrayIndex][j][2]);
			buf[countByte++] = *(unsigned int*)&(verticesX[arrayIndex][j]);
			buf[countByte++] = *(unsigned int*)&(verticesY[arrayIndex][j]);
			buf[countByte++] = *(unsigned int*)&zero;
}		
}
return arrayIndex;
}



int main(){

	int fd;
	int result;
	int i;
	int ramSize;
	int choice;
	printf("1.Frame Buffer Line\n2.Fifo Triangles\n3.DMA Traingles One triangle at a time\n4.DMA Triangle , Bulk loading 50 triangles at a time\n");
	scanf("%d",&choice);
	
	fd = open("/dev/kyouko2", O_RDWR);
	kyouko2.u_control_base = mmap(0, KYOUKO_CONTROL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	ioctl(fd, VMODE, GRAPHICS_ON);
	ioctl(fd, SYNC);
	
	if(choice == 1){
	result = U_READ_REG(Device_Ram);
	ramSize = result * 1024 * 1024;
	kyouko2.u_fb_base = mmap(0, ramSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x80000000);
	for (i = 200 * 1024; i<201 * 1024; i++){
		U_WRITE_FB(i, 0xFF0000);
	}
	U_WRITE_REG(RASTER_FLUSH, 1);
	}
	else if(choice == 2){
	ioctl(fd, SYNC);
	draw_fifo(fd);
	ioctl(fd, FLUSH);
	}
	else if(choice == 3 || choice == 4){
	ioctl(fd, SYNC);

	limit = 6;
	
	initSierpinski();
	drawSierpinski(0, left, top, right);
	
	ioctl(fd, BIND_DMA, &arg);
	ioctl(fd, SYNC);
	for (i = index-1; i >=0; i--) {
		if(choice == 4)
		i=draw_dma(i,30,4);
		else 
		draw_dma(i,1,3);
		arg = countByte*4; 
		
		ioctl(fd, SYNC);
		ioctl(fd, START_DMA, &arg);

		ioctl(fd, FLUSH);
		ioctl(fd, SYNC);

	}
	}
	else{
	printf("Incorrect Choice Exiting.......");
	}
	sleep(5);
	if(choice==3)
		ioctl(fd, UNBIND_DMA);
	ioctl(fd, VMODE, GRAPHICS_OFF);
	close(fd);
	return 0;
}
