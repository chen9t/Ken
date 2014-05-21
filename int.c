#include"bootpack.h"
#include<stdio.h>

void init_pic(void)	/*初始化PIC*/
{
	/*屏蔽两个PIC的所有中断*/
	io_out8(PIC0_IMR,0xff);
	io_out8(PIC1_IMR,0xff);
	
	/*初始化PIC0*/
	io_out8(PIC0_ICW1,0x11);	/*边沿触发模式*/
	io_out8(PIC0_ICW2,0x20);	/*IRQ0-7由INT20-27接收*/
	io_out8(PIC0_ICW3,1<<2);	/*PIC1由IRQ2连接*/
	io_out8(PIC0_ICW4,0x10);	/*无缓冲区模式*/
	
	/*初始化PIC1*/
	io_out8(PIC1_ICW1,0x11);	/*边沿触发模式*/
	io_out8(PIC1_ICW2,0x28);	/*IRQ8-15由INT28-2f接收*/
	io_out8(PIC1_ICW3,2	 );	/*PIC1由IRQ2连接*/
	io_out8(PIC1_ICW4,0x10);	/*w无缓冲区模式*/
	
	/*禁止PIC1外的所有中断*/
	io_out8(PIC0_IMR,0xfb);	/*1111011*/
	io_out8(PIC1_IMR,0xff);
	
	return;
}

void inthandler27(int *esp)
{
	io_out8(PIC0_OCW2, 0x67); 
	return;
}
