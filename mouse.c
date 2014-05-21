#include"bootpack.h"

struct FIFO32 *mousefifo;
int mousedata0;

void inthandler2c(int *esp)	/*鼠标的中断处理程序（上半部）*/
{
	unsigned char data;
	io_out8(PIC1_OCW2,0x64);	/*通知PIC1"IRQ-12"的中断信号已收到*/
	io_out8(PIC0_OCW2,0x62);	/*通知PIC0“IRQ-02”的中断信号已收到*/
	data=io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo,data+mousedata0);
	
	return;
}

#define KEYCMD_SENDTO_MOUSE	0xd4 /*该指令发送给键盘控制电路端口PORT_KEYCMD后，下一个数据就自动发送给鼠标*/
#define MOUSECMD_ENABLE		0xf4 /*激活鼠标指令*/

void enable_mouse(struct FIFO32 *fifo,int data0,struct MOUSE_DEC *mdec) /*激活鼠标*/
{
	/*讲FIFO缓冲区的信息保存到全局变量*/
	mousefifo=fifo;
	mousedata0=data0;
	/*鼠标有效*/
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,MOUSECMD_ENABLE);
	
	mdec->phase=0;
	
	return;
}

int mouse_decode(struct MOUSE_DEC *mdec,unsigned char dat) /*鼠标解读*/
{
	if(mdec->phase==0){
		/*第0阶段，鼠标被激活后，会返回0xfa，接收到这个数据表明鼠标已被激活，可以从中接收数据*/
		if(dat==0xfa){
			mdec->phase=1;
		}
		return 0;
	}
	if(mdec->phase==1){
		/*等待鼠标第一字节的阶段*/
		if((dat&0xc8)==0x08){	/*确定第一阶段收到的数据是否有效，见P149*/
			mdec->buf[0]=dat;
			mdec->phase=2;
		}
		return 0;
	}
	if(mdec->phase==2){
		/*等待鼠标第二字节的阶段（与鼠标的左右移动有关）*/
		mdec->buf[1]=dat;
		mdec->phase=3;
		return 0;
	}
	if(mdec->phase==3){
		/*等待鼠标第三字节的阶段（与鼠标的上下移动有关）*/
		mdec->buf[2]=dat;
		mdec->phase=1;
		
		/*处理接收到的数据*/
		mdec->btn=mdec->buf[0]&0x07;	/*鼠标按键的状态放在buf[0]的低3位*/
		mdec->x=mdec->buf[1];			/*左右移动的数据*/
		mdec->y=mdec->buf[2];			/*上下移动的数据*/
		
		/*将x和y的第8位及第8位以后全部设为1或保留为0才能正确解读x和y*/
		if((mdec->buf[0]&0x10)!=0){
			mdec->x |= 0xffffff00;
		}
		if((mdec->buf[0]&0x20)!=0){
			mdec->y |= 0xffffff00;
		}
		
		mdec->y=-mdec->y;	/*鼠标y的方向与画面符号相反*/
		
		return 1;
	}
	return -1;
}
