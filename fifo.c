#include"bootpack.h"

#define FLAGS_OVERRUN    0x0001

void fifo32_init(struct FIFO32 *fifo,int size,int *buf,struct TASK *task) /*初始化FIFO缓冲区*/
{
	fifo->size=size;
	fifo->buf=buf;
	fifo->free=size;  /*缓冲区大小*/
	fifo->p=0;  /*下一个数据写入位置*/
	fifo->q=0;  /*下一个数据读出位置*/
	fifo->flags=0;
	fifo->task=task;
	
	return;
}

int fifo32_put(struct FIFO32 *fifo,int data) /*往FIFO缓冲区存储数据*/
{
	if(fifo->free==0){
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	
	fifo->buf[fifo->p]=data;
	fifo->p++;
	if(fifo->p==fifo->size){
		fifo->p=0;
	}
	fifo->free--;
	
	/*有数据传入时，唤醒休眠的任务*/
	if(fifo->task!=0){
		if(fifo->task->flags!=2){
			task_run(fifo->task,-1,0);
		}
	}

	return 0;
}

int fifo32_get(struct FIFO32 *fifo) /*从FIFO缓冲区取出数据*/
{
	int data;
	
	if(fifo->free==fifo->size){
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	
	data=fifo->buf[fifo->q];
	fifo->q++;
	if(fifo->q==fifo->size){
		fifo->q=0;
	}
	fifo->free++;
	
	return data;
}

int fifo32_status(struct FIFO32 *fifo) /*共积攒多少数据*/
{
	return fifo->size - fifo->free;
}
