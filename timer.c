#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

#define TIMER_FLAGS_ALLOC	1
#define TIMER_FLAGS_USING	2

struct TIMERCTL timerctl;	/*在这个地方实现结构体，与sheet的区别*/

void init_pit(void)	/*初始化pit*/
{
	int i;
	struct TIMER *t;
	io_out8(PIT_CTRL,0x34);
	io_out8(PIT_CNT0,0x9c);
	io_out8(PIT_CNT0,0x2e);
	
	timerctl.count=0;
	for(i=0;i<MAX_TIMER;i++){
		timerctl.timers0[i].flags=0;
	}
	
	/*初始化哨兵*/
	t=timer_alloc();
	t->timeout=0xffffffff;
	t->flags=TIMER_FLAGS_USING;
	t->next=0;
	/*将哨兵加入线性表*/
	timerctl.t0=t;
	timerctl.next=0xffffffff;
	
	return;
}

struct TIMER *timer_alloc(void)	/*分配未使用的计时器*/
{
	int i;
	
	for(i=0;i<MAX_TIMER;i++){
		if(timerctl.timers0[i].flags==0){
			timerctl.timers0[i].flags=TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	return 0;
}

void timer_free(struct TIMER *timer)
{
	timer->flags=0;
	return;
}

void timer_init(struct TIMER *timer,struct FIFO32 *fifo,int data)
{
	timer->fifo=fifo;
	timer->data=data;
	
	return;
}

void timer_settime(struct TIMER *timer,unsigned int timeout)
{
	int e;
	struct TIMER *t,*s;
	
	timer->timeout=timeout+timerctl.count;
	timer->flags=TIMER_FLAGS_USING;
	
	/*重新对定时器进行排序，所以需要暂时关闭中断*/
	e=io_load_eflags();	
	io_cli();
	
	/*插入最前面的情况*/
	t=timerctl.t0;
	if(timer->timeout <= t->timeout){
		timerctl.t0=timer;
		timer->next=t;
		timerctl.next=timer->timeout;
		/*打开中断*/
		io_store_eflags(e);
		return;
	}
	
	/*插入s与t之间的情况*/
	for(;;){
		s=t;
		t=t->next;
	
		if(timer->timeout <= t->timeout){
			s->next=timer;
			timer->next=t;
			/*打开中断*/
			io_store_eflags(e);		
			return;
		}
	}
}

void inthandler20(int *esp)
{
	struct TIMER *timer;
	char ts=0;
	io_out8(PIC0_OCW2,0x60);
	
	timerctl.count++;
	if(timerctl.next>timerctl.count){
		return;
	}
	
	timer=timerctl.t0;
	for(;;){	
		if(timer->timeout>timerctl.count){
			break;
		}
		/*超时*/
		timer->flags=TIMER_FLAGS_ALLOC;
		if(timer!=task_timer){
			fifo32_put(timer->fifo,timer->data);
		}else{
			ts=1;	/*这里直接比较是否为mt_timer定时器，主要是由于mt_timer不需要想fifo中存取数据，加快速度*/
		}
		timer=timer->next;
	}
	
	timerctl.t0=timer;
	timerctl.next=timer->timeout;	/*更新最近的超时点next*/

	if(ts!=0){
		task_switch();
	}

	return;
}
