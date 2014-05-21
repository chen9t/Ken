#include"bootpack.h"

struct FIFO32 *keyfifo;
int keydata0;

void inthandler21(int *esp)	/*键盘的中断处理程序（上半部）*/
{
	unsigned char data;
	io_out8(PIC0_OCW2,0x61);	/*通知PIC0"IRQ-01"的中断信号已收到*/
	data=io_in8(PORT_KEYDAT);	/*从键盘读取按键编码*/
	fifo32_put(keyfifo,data+keydata0);
	
	return;
}

#define PORT_KEYSTA					0x0064 /*键盘控制电路状态读取端口*/
#define KEYSTA_SEND_NOTREADY		0x02
#define KEYCMD_WRITE_MODE			0x60 /*键盘模式设定指令*/
#define KBC_MODE					0x47 /*激活鼠标设定指令*/

void wait_KBC_sendready(void) /*检测键盘控制电路是否准备好接收CPU指令*/
{
	for(;;){
		if( (io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0 ){ /*若键盘控制电路可以接受CPU指令，0x0064处的倒数第二位为0*/
			break;
		}
	}
}

void init_keyboard(struct FIFO32 *fifo,int data0) /*初始化键盘控制电路，使其可以发送鼠标中断*/
{
	/*讲FIFO缓冲区的信息保存到全局变量*/
	keyfifo=fifo;
	keydata0=data0;
	/*键盘控制器的初始化*/
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,KBC_MODE);
	
	return;
}
