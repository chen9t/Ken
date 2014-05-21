#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

struct TASK *task_now(void)	/*返回当前正在执行的任务*/
{
	struct TASKLEVEL *tl= &taskctl->level[taskctl->now_lv];
	return tl->tasks[tl->now];
}

void task_add(struct TASK *task)	/*加入新任务*/
{
	struct TASKLEVEL *tl= &taskctl->level[task->level];	/*根据task中的level属性，找出相应队列*/
	tl->tasks[tl->running]=task;	/*将其加入相应level中的队列*/
	tl->running++;
	task->flags=2;

	return;
}

void task_remove(struct TASK *task)	/*将任务从tasks[]数组中移除*/
{
	int i;
	struct TASKLEVEL *tl= &taskctl->level[task->level];

	/*首先将位置i找出来*/
	for(i=0;i<tl->running;i++){
		if(tl->tasks[i]==task){
			break;
		}
	}
	tl->running--;
	if(i<tl->now){	/*i小于now时，需要进行相应处理*/
		tl->now--;
	}
	if(tl->now>=tl->running){
		tl->now=0;
	}
	/*移动成员*/
	for(;i<tl->running;i++){
		tl->tasks[i]=tl->tasks[i+1];
	}
	task->flags=1;

	return;
}

void task_switchsub(void)	/*切换LEVEL（寻找出最上层LEVEL），并赋值给now_lv*/
{
	int i;

	/*寻找最上层的LEVEL*/
	for(i=0;i<MAX_TASKLEVELS;i++){
		if(taskctl->level[i].running>0){
			break;
		}
	}

	taskctl->now_lv=i;
	taskctl->lv_change=0;

	return;
}

struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;

	taskctl=(struct TASKCTL *)memman_alloc_4k(memman, sizeof(struct TASKCTL));
	for(i=0;i<MAX_TASKS;i++){
		taskctl->tasks0[i].sel=(TASK_GDT0+i)*8;
		taskctl->tasks0[i].flags=0;
		set_segmdesc(gdt+TASK_GDT0+i,103,(int)&taskctl->tasks0[i].tss,AR_TSS32);
	}

	for(i=0;i<MAX_TASKLEVELS;i++){
		taskctl->level[i].running=0;
		taskctl->level[i].now=0;
	}

	/*将当前正在运行的程序设置为任务0*/
	task=task_alloc();
	task->flags=2;
	task->priority=2;	/*将超时时间定义为优先级，0.02秒*/
	task->level=0;		/*设定该任务的level*/
	task_add(task);		/*根据level添加进相关队列*/
	task_switchsub();	/*切换到相应level*/
	load_tr(task->sel);

	/*任务定时器设定*/
	task_timer=timer_alloc();
	timer_settime(task_timer,task->priority);

	return task;
}

struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;

	for(i=0;i<MAX_TASKS;i++){
		if(taskctl->tasks0[i].flags==0){

			task=&taskctl->tasks0[i];
			task->flags=1;
			task->tss.eflags = 0x00000202;
			task->tss.eax = 0; 
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;

			return task;
		}
	}
	return 0;
}

void task_run(struct TASK *task,int level,int priority)	/*将一个休眠中的任务唤醒或改变活动中任务的LEVEL*/
{
	if(level<0){	/*不改变LEVEL*/
		level=task->level;
	}

	if(priority>0){
		task->priority=priority;
	}

	if(task->flags==2 && task->level!=level){	/*改变活动中任务的LEVEL*/
		task_remove(task);	/*简单的remove任务，执行后flags会变为1，可以与休眠中的任务唤醒结合到一起实现*/
	}

	if(task->flags!=2){
		task->level=level;
		task_add(task);
	}

	taskctl->lv_change = 1; 

	return;
}

void task_switch(void)
{
	struct TASKLEVEL *tl= &taskctl->level[taskctl->now_lv];
	struct TASK *new_task,*now_task=tl->tasks[tl->now];

	tl->now++;
	if(tl->now==tl->running){
		tl->now=0;
	}

	if(taskctl->lv_change!=0){	/*在task_run()改变活动中任务level时*/
		task_switchsub();
		tl= &taskctl->level[taskctl->now_lv];
	}

	new_task=tl->tasks[tl->now];
	timer_settime(task_timer,new_task->priority);
	
	if(new_task!=now_task){	/*有可能队列中只有一项任务*/
		farjmp(0,new_task->sel);
	}
	return;
}

void task_sleep(struct TASK *task)
{
	struct TASK *now_task;
	if(task->flags==2){	/*在指定任务处于唤醒状态时*/

		now_task=task_now();
		task_remove(task);
		
		if(task==now_task){
			task_switchsub();
			now_task=task_now();
			farjmp(0,now_task->sel);
		}

	}
	return;
}
