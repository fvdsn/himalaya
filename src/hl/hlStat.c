#include<stdio.h> 
#include"hlImg.h"
int num_img;
int num_op;
int mem_op;
int num_vec;
int mem_vec;
int num_frame;
int mem_frame;
int num_tile;
int mem_tile;
int num_frame_node;
int mem_frame_node;

#define MB 1.0f/(float)(1024*1024)
void hlStatPrint(void){
	printf("num_tile:%d\n",num_tile);
	printf("mem_tile:%fMB\n",num_tile*4096*MB);
	printf("num_op:%d\n",num_op);
	printf("mem_op:%fMB\n",num_op*(sizeof(hlOp)+sizeof(hlColor)+
					5*sizeof(float))*MB);

	printf("num_frame:%d\n",num_frame);
	printf("mem_frame:%fMB\n",num_frame*sizeof(hlFrame)*MB);

	printf("num_frame_node:%d\n",num_frame_node);
	printf("mem_frame_node:%fMB\n",num_frame_node*sizeof(hlNode)*MB);

	printf("num_img:%d\n",num_img);
	printf("mem_img:%fMB\n",num_img*sizeof(hlImg)*MB);

	printf("num_vec:%d\n",num_vec);
	printf("mem_vec:%fMB\n",(num_vec*sizeof(hlVec)+mem_vec)*MB);
}
