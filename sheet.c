#include "bootpack.h"

#define SHEET_USE	1

struct SHTCTL *shtctl_init(struct MEMMAN *memman,unsigned char *vram,int xsize,int ysize)	/*初始化SHTCTL结构*/
{
	struct SHTCTL *ctl;
	int i;
	
	ctl=(struct SHTCTL *)memman_alloc_4k(memman,sizeof(struct SHTCTL));	/*分配相应大小的内存*/
	if(ctl==0){
		goto err;
	}
	
	ctl->map=(unsigned char *)memman_alloc_4k(memman,xsize*ysize);	/*为图层地图分配内存*/
	if(ctl->map==0){
		memman_free_4k(memman,(int)ctl,sizeof(struct SHTCTL));	/*map未分配到内存，而ctl已分配到内存，因此需要将其内存释放*/
		goto err;
	}
	
	ctl->vram=vram;
	ctl->xsize=xsize;
	ctl->ysize=ysize;
	ctl->top=-1;
	for(i=0;i<MAX_SHEETS;i++){	/*标记为未使用*/
		ctl->sheets0[i].flags=0;
		ctl->sheets0[i].ctl=ctl;	/*记录所属*/
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
			sht->flags=SHEET_USE;	/*标记为正在使用*/
			sht->height=-1;	/*隐藏*/
			return sht;
		}
	}
	return 0;	/*所有SHEET都处于正在使用状态*/
}

void sheet_setbuf(struct SHEET *sht,unsigned char *buf,int xsize,int ysize,int col_inv)
{
	sht->buf=buf;
	sht->bxsize=xsize;
	sht->bysize=ysize;
	sht->col_inv=col_inv;
	return;
}

void sheet_refreshmap(struct SHTCTL *ctl,int vx0,int vy0,int vx1,int vy1,int h0)	/*从底向上，刷新所有图层*/
{
	int h,bx,by,vx,vy,bx0,by0,bx1,by1;
	unsigned char *buf,sid,*map=ctl->map;
	struct SHEET *sht;
	
	/*避免刷新画面之外的图层地图*/
	if(vx0<0)	{ vx0=0; }
	if(vy0<0)	{ vy0=0; }
	if(vx1>ctl->xsize)	{ vx1=ctl->xsize; }
	if(vy1>ctl->ysize)	{ vy1=ctl->ysize; }
	
	/*从底向上，刷新所有图层地图*/
	for(h=h0;h<=ctl->top;h++)
	{
		sht=ctl->sheets[h];
		sid=sht-ctl->sheets0;	/*生成图层地图号码*/
		buf=sht->buf;
		
		/*对bx0,by0以及bx1,by1进行倒推*/
		bx0=vx0-sht->vx0;
		by0=vy0-sht->vy0;
		bx1=vx1-sht->vx0;
		by1=vy1-sht->vy0;
		/*图层地图重叠的情况1*/
		if(	bx0<0	)	{	bx0=0;	}
		if(	by0<0	)	{	by0=0;	}
		/*图层地图重叠的情况2*/
		if(	bx1>sht->bxsize	)	{	bx1=sht->bxsize;	}
		if(	by1>sht->bysize	)	{	by1=sht->bysize;	}
		
		/*刷新指定区域的图层地图*/
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

void sheet_refreshsub(struct SHTCTL *ctl,int vx0,int vy0,int vx1,int vy1,int h0,int h1)	/*从底向上，刷新所有图层*/
{
	int h,bx,by,vx,vy,bx0,by0,bx1,by1;
	unsigned char *buf,c,sid,*vram=ctl->vram,*map=ctl->map;
	struct SHEET *sht;
	
	/*避免刷新画面之外的图层*/
	if(vx0<0)	{ vx0=0; }
	if(vy0<0)	{ vy0=0; }
	if(vx1>ctl->xsize)	{ vx1=ctl->xsize; }
	if(vy1>ctl->ysize)	{ vy1=ctl->ysize; }
	
	/*从底向上，刷新所有图层*/
	for(h=h0;h<=h1;h++)
	{
		sht=ctl->sheets[h];
		sid=sht-ctl->sheets0;
		buf=sht->buf;
		
		/*对bx0,by0以及bx1,by1进行倒推*/
		bx0=vx0-sht->vx0;
		by0=vy0-sht->vy0;
		bx1=vx1-sht->vx0;
		by1=vy1-sht->vy0;
		/*图层重叠的情况1*/
		if(	bx0<0	)	{	bx0=0;	}
		if(	by0<0	)	{	by0=0;	}
		/*图层重叠的情况2*/
		if(	bx1>sht->bxsize	)	{	bx1=sht->bxsize;	}
		if(	by1>sht->bysize	)	{	by1=sht->bysize;	}
		
		/*刷新指定区域的图层*/
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

void sheet_refreshsub_2(struct SHTCTL *ctl,int vx0,int vy0,int vx1,int vy1,int h0)	/*sheet_refreshsub的另一种方法*/
{
	int h,bx,by,vx,vy;
	unsigned char *buf,c,*vram=ctl->vram;
	struct SHEET *sht;
	
	/*避免刷新画面之外的图层*/
	if(vx0<0)	{ vx0=0; }
	if(vy0<0)	{ vy0=0; }
	if(vx1>ctl->xsize)	{ vx1=ctl->xsize; }
	if(vy1>ctl->ysize)	{ vy1=ctl->ysize; }
	
	/*从底向上，刷新所有图层*/
	for(h=h0;h<=ctl->top;h++)
	{
		sht=ctl->sheets[h];
		buf=sht->buf;
		
		/*为了加快处理速度，将for语句的范围限定在刷新范围之内*/
		for(vy=vy0;vy<vy1;vy++)
		{
			by=vy-sht->vy0;	/*倒推出图层内该点的Y坐标*/
			if(by>=0&&by<=sht->bysize){
				for(vx=vx0;vx<vx1;vx++)
				{
					bx=vx-sht->vx0;	/*倒推出图层内该点的X坐标*/
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

void sheet_updown(struct SHEET *sht,int height)	/*设定底板高度(这段代码锻炼对界限的选择)*/
{
	struct SHTCTL *ctl=sht->ctl;
	int h,old=sht->height;
	
	if(height>ctl->top+1){
		height=ctl->top+1;
	}
	if(height<-1){
		height=-1;
	}
	sht->height=height;	/*设定高度*/
	
	if(old>height){	/*比原来低*/
		if(height>=0){
			/*把中间的往上提*/
			for(h=old;h>height;h--){
				ctl->sheets[h]=ctl->sheets[h-1];
				ctl->sheets[h]->height=h;
			}
			ctl->sheets[height]=sht;
			sheet_refreshmap(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,height+1);
			sheet_refreshsub(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,height+1,old);
		}else{	/*隐藏*/
			if(ctl->top>old){ /*这个if条件可以去掉*/
				/*把上面的降下来*/
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
	else if(old<height){	/*比以前高*/
		if(old>=0){
			/*把中间的拉下去*/
			for(h=old;h<height;h++){
				ctl->sheets[h]=ctl->sheets[h+1];
				ctl->sheets[h]->height=h;
			}
			ctl->sheets[height]=sht;
		}else{	/*由隐藏状态转为显示状态*/
			/*将在height上面的再往上提一格*/
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
				
void sheet_refresh(struct SHEET *sht,int bx0,int by0,int bx1,int by1)	/*刷新指定图层指定区域的信息*/
{	
	if(sht->height>=0){
		sheet_refreshsub(sht->ctl,sht->vx0+bx0,sht->vy0+by0,sht->vx0+bx1,sht->vy0+by1,sht->height,sht->height);
	}
	return;
}

void sheet_slide(struct SHEET *sht,int vx0,int vy0)
{
	struct SHTCTL *ctl=sht->ctl;
	int old_vx0=sht->vx0,old_vy0=sht->vy0;	/*记住移动前的显示位置*/
	/*设定新的显示位置*/
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
