#include "bootpack.h"

#define SHEET_USE	1

struct SHTCTL *shtctl_init(struct MEMMAN *memman,unsigned char *vram,int xsize,int ysize)	/*��ʼ��SHTCTL�ṹ*/
{
	struct SHTCTL *ctl;
	int i;
	
	ctl=(struct SHTCTL *)memman_alloc_4k(memman,sizeof(struct SHTCTL));	/*������Ӧ��С���ڴ�*/
	if(ctl==0){
		goto err;
	}
	
	ctl->map=(unsigned char *)memman_alloc_4k(memman,xsize*ysize);	/*Ϊͼ���ͼ�����ڴ�*/
	if(ctl->map==0){
		memman_free_4k(memman,(int)ctl,sizeof(struct SHTCTL));	/*mapδ���䵽�ڴ棬��ctl�ѷ��䵽�ڴ棬�����Ҫ�����ڴ��ͷ�*/
		goto err;
	}
	
	ctl->vram=vram;
	ctl->xsize=xsize;
	ctl->ysize=ysize;
	ctl->top=-1;
	for(i=0;i<MAX_SHEETS;i++){	/*���Ϊδʹ��*/
		ctl->sheets0[i].flags=0;
		ctl->sheets0[i].ctl=ctl;	/*��¼����*/
	}
	
	err:
		return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for(i=0;i<MAX_SHEETS;i++){
		if(ctl->sheets0[i].flags==0){
			sht=&ctl->sheets0[i];
			sht->flags=SHEET_USE;	/*���Ϊ����ʹ��*/
			sht->height=-1;	/*����*/
			return sht;
		}
	}
	return 0;	/*����SHEET����������ʹ��״̬*/
}

void sheet_setbuf(struct SHEET *sht,unsigned char *buf,int xsize,int ysize,int col_inv)
{
	sht->buf=buf;
	sht->bxsize=xsize;
	sht->bysize=ysize;
	sht->col_inv=col_inv;
	return;
}

void sheet_refreshmap(struct SHTCTL *ctl,int vx0,int vy0,int vx1,int vy1,int h0)	/*�ӵ����ϣ�ˢ������ͼ��*/
{
	int h,bx,by,vx,vy,bx0,by0,bx1,by1;
	unsigned char *buf,sid,*map=ctl->map;
	struct SHEET *sht;
	
	/*����ˢ�»���֮���ͼ���ͼ*/
	if(vx0<0)	{ vx0=0; }
	if(vy0<0)	{ vy0=0; }
	if(vx1>ctl->xsize)	{ vx1=ctl->xsize; }
	if(vy1>ctl->ysize)	{ vy1=ctl->ysize; }
	
	/*�ӵ����ϣ�ˢ������ͼ���ͼ*/
	for(h=h0;h<=ctl->top;h++)
	{
		sht=ctl->sheets[h];
		sid=sht-ctl->sheets0;	/*����ͼ���ͼ����*/
		buf=sht->buf;
		
		/*��bx0,by0�Լ�bx1,by1���е���*/
		bx0=vx0-sht->vx0;
		by0=vy0-sht->vy0;
		bx1=vx1-sht->vx0;
		by1=vy1-sht->vy0;
		/*ͼ���ͼ�ص������1*/
		if(	bx0<0	)	{	bx0=0;	}
		if(	by0<0	)	{	by0=0;	}
		/*ͼ���ͼ�ص������2*/
		if(	bx1>sht->bxsize	)	{	bx1=sht->bxsize;	}
		if(	by1>sht->bysize	)	{	by1=sht->bysize;	}
		
		/*ˢ��ָ�������ͼ���ͼ*/
		for(by=by0;by<by1;by++)
		{
			vy=by+sht->vy0;
			for(bx=bx0;bx<bx1;bx++)
			{
				vx=bx+sht->vx0;
				if(buf[by * sht->bxsize + bx]!=sht->col_inv){
					map[vy * ctl->xsize + vx]=sid;
				}
			}
		}
	}
	return;
}

void sheet_refreshsub(struct SHTCTL *ctl,int vx0,int vy0,int vx1,int vy1,int h0,int h1)	/*�ӵ����ϣ�ˢ������ͼ��*/
{
	int h,bx,by,vx,vy,bx0,by0,bx1,by1;
	unsigned char *buf,c,sid,*vram=ctl->vram,*map=ctl->map;
	struct SHEET *sht;
	
	/*����ˢ�»���֮���ͼ��*/
	if(vx0<0)	{ vx0=0; }
	if(vy0<0)	{ vy0=0; }
	if(vx1>ctl->xsize)	{ vx1=ctl->xsize; }
	if(vy1>ctl->ysize)	{ vy1=ctl->ysize; }
	
	/*�ӵ����ϣ�ˢ������ͼ��*/
	for(h=h0;h<=h1;h++)
	{
		sht=ctl->sheets[h];
		sid=sht-ctl->sheets0;
		buf=sht->buf;
		
		/*��bx0,by0�Լ�bx1,by1���е���*/
		bx0=vx0-sht->vx0;
		by0=vy0-sht->vy0;
		bx1=vx1-sht->vx0;
		by1=vy1-sht->vy0;
		/*ͼ���ص������1*/
		if(	bx0<0	)	{	bx0=0;	}
		if(	by0<0	)	{	by0=0;	}
		/*ͼ���ص������2*/
		if(	bx1>sht->bxsize	)	{	bx1=sht->bxsize;	}
		if(	by1>sht->bysize	)	{	by1=sht->bysize;	}
		
		/*ˢ��ָ�������ͼ��*/
		for(by=by0;by<by1;by++)
		{
			vy=by+sht->vy0;
			for(bx=bx0;bx<bx1;bx++)
			{
				vx=bx+sht->vx0;
				c=buf[by * sht->bxsize + bx];
				if(sid==map[vy * ctl->xsize + vx]){
					vram[vy * ctl->xsize + vx]=c;
				}
			}
		}
	}
	return;
}

void sheet_refreshsub_2(struct SHTCTL *ctl,int vx0,int vy0,int vx1,int vy1,int h0)	/*sheet_refreshsub����һ�ַ���*/
{
	int h,bx,by,vx,vy;
	unsigned char *buf,c,*vram=ctl->vram;
	struct SHEET *sht;
	
	/*����ˢ�»���֮���ͼ��*/
	if(vx0<0)	{ vx0=0; }
	if(vy0<0)	{ vy0=0; }
	if(vx1>ctl->xsize)	{ vx1=ctl->xsize; }
	if(vy1>ctl->ysize)	{ vy1=ctl->ysize; }
	
	/*�ӵ����ϣ�ˢ������ͼ��*/
	for(h=h0;h<=ctl->top;h++)
	{
		sht=ctl->sheets[h];
		buf=sht->buf;
		
		/*Ϊ�˼ӿ촦���ٶȣ���for���ķ�Χ�޶���ˢ�·�Χ֮��*/
		for(vy=vy0;vy<vy1;vy++)
		{
			by=vy-sht->vy0;	/*���Ƴ�ͼ���ڸõ��Y����*/
			if(by>=0&&by<=sht->bysize){
				for(vx=vx0;vx<vx1;vx++)
				{
					bx=vx-sht->vx0;	/*���Ƴ�ͼ���ڸõ��X����*/
					if(bx>=0&&bx<=sht->bxsize){
						c=buf[by * sht->bxsize+bx];
						if(c!=sht->col_inv){
							vram[vy * ctl->xsize + vx]=c;
						}
					}
				}
			}
		}
	}
	return;
}

void sheet_updown(struct SHEET *sht,int height)	/*�趨�װ�߶�(��δ�������Խ��޵�ѡ��)*/
{
	struct SHTCTL *ctl=sht->ctl;
	int h,old=sht->height;
	
	if(height>ctl->top+1){
		height=ctl->top+1;
	}
	if(height<-1){
		height=-1;
	}
	sht->height=height;	/*�趨�߶�*/
	
	if(old>height){	/*��ԭ����*/
		if(height>=0){
			/*���м��������*/
			for(h=old;h>height;h--){
				ctl->sheets[h]=ctl->sheets[h-1];
				ctl->sheets[h]->height=h;
			}
			ctl->sheets[height]=sht;
			sheet_refreshmap(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,height+1);
			sheet_refreshsub(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,height+1,old);
		}else{	/*����*/
			if(ctl->top>old){ /*���if��������ȥ��*/
				/*������Ľ�����*/
				for(h=old;h<ctl->top;h++){
					ctl->sheets[h]=ctl->sheets[h+1];
					ctl->sheets[h]->height=h;
				}
			}
			ctl->top--;
			sheet_refreshmap(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,0);
			sheet_refreshsub(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,0,old-1);
		}
	}
	else if(old<height){	/*����ǰ��*/
		if(old>=0){
			/*���м������ȥ*/
			for(h=old;h<height;h++){
				ctl->sheets[h]=ctl->sheets[h+1];
				ctl->sheets[h]->height=h;
			}
			ctl->sheets[height]=sht;
		}else{	/*������״̬תΪ��ʾ״̬*/
			/*����height�������������һ��*/
			for(h=ctl->top;h>=height;h--){
				ctl->sheets[h+1]=ctl->sheets[h];
				ctl->sheets[h+1]->height=h+1;
			}
			ctl->sheets[height]=sht;
			ctl->top++;
		}
		sheet_refreshmap(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,height);
		sheet_refreshsub(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,height,height);
	}
	return;
}
				
void sheet_refresh(struct SHEET *sht,int bx0,int by0,int bx1,int by1)	/*ˢ��ָ��ͼ��ָ���������Ϣ*/
{	
	if(sht->height>=0){
		sheet_refreshsub(sht->ctl,sht->vx0+bx0,sht->vy0+by0,sht->vx0+bx1,sht->vy0+by1,sht->height,sht->height);
	}
	return;
}

void sheet_slide(struct SHEET *sht,int vx0,int vy0)
{
	struct SHTCTL *ctl=sht->ctl;
	int old_vx0=sht->vx0,old_vy0=sht->vy0;	/*��ס�ƶ�ǰ����ʾλ��*/
	/*�趨�µ���ʾλ��*/
	sht->vx0=vx0;
	sht->vy0=vy0;
	
	if(sht->height>=0){
		sheet_refreshmap(ctl,old_vx0,old_vy0,old_vx0+sht->bxsize,old_vy0+sht->bysize,0);
		sheet_refreshmap(ctl,vx0,vy0,vx0+sht->bxsize,vy0+sht->bysize,sht->height);
		sheet_refreshsub(ctl,old_vx0,old_vy0,old_vx0+sht->bxsize,old_vy0+sht->bysize,0,sht->height-1);
		sheet_refreshsub(ctl,vx0,vy0,vx0+sht->bxsize,vy0+sht->bysize,sht->height,sht->height);
	}
	return;
}

void sheet_free(struct SHEET *sht)
{
	if(sht->height>=0){
		sheet_updown(sht,-1);
	}
	sht->flags=0;
	return;
}