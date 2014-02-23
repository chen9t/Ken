; haribote-ipl
; TAB=4

CYLS	EQU		10
		ORG		0x7c00		;程序装载地址
		        
		JMP		entry
		DB		0x90
		
;标准FAT12格式软盘的设置

		DB		"HARIBOTE"	;启动区的名称
		DW		512			;每个扇区的大小（必须512）
		DB		1			;簇的大小（1个扇区）
		DW		1			;FAT的起始位置
		DB		2			;FAT的个数（必须2）
		DW		224			;根目录的大小（一般224项）
		DW 		2880		;磁盘的大小
		DB		0xf0		;磁盘的种类
		DW		9			;FAT的长度（必须9）
		DW		18			;1个磁道有几个扇区（必须18）
		DW		2			;磁头数（必须2）
		DD		0			;不使用分区
		DD		2880		;重写一次磁盘大小
		DB		0,0,0x29	;不知道是神马
		DD		0xffffffff	;卷标号码
		DB		"HARIBOTEOS "	;磁盘名称
		DB		"FAT12   "		;磁盘格式名称
		RESB	18			;空出18字节
		
;程序核心

entry:
		MOV		AX,0		;初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00	;栈指针寄存器
		MOV		DS,AX
		
;读磁盘,磁盘配置
		MOV		AX,0x0820	;0x8000~0x81ff留给启动区，将剩下的程序读到0x8200后的地方
		MOV		ES,AX		;ES:BX为缓冲区地址，下面要将BX置0
		MOV		CH,0		;柱面0
		MOV		DH,0		;磁头0
		MOV		CL,2		;扇区2
readloop:
		MOV		SI,0		;记录失败次数的寄存器
retry:
		MOV		AH,0x02		;读入磁盘
		MOV		AL,1		;1个扇区
		MOV		BX,0
		MOV		DL,0x00		;A驱动器
		INT		0x13		;调用磁盘BIOS
		JNC		next		;进位标志为0，没有错，跳转到next
		;以下为检错部分
		ADD		SI,1		
		CMP		SI,5
		JAE		error
		MOV		AH,0x00
		MOV		DL,0x00		;A驱动器
		INT		0x13		;重置驱动器
		JMP		retry

next:
		MOV		AX,ES		;把内存地址后移0x200个字节，因为没有ADD	ES,0x020指令，稍微绕个弯
		ADD		AX,0x0020		
		MOV		ES,AX
		ADD		CL,1		;CL加1，读入18个扇区，若CL<=18跳转至readloop
		CMP		CL,18
		JBE		readloop
		
		MOV		CL,1
		ADD		DH,1		;磁头1
		CMP		DH,2
		JB		readloop
		
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS
		JB		readloop
		
		MOV		[0x0ff0],CH		
		JMP		0xc200		;操作系统本身内容从0xc200处开始
		
error:
		MOV 	SI,msg

;跟显卡有关的参数,并挨个显示字符
putloop:
		MOV		AL,[SI]		;AL为需要显示的字符编码,P38
		ADD		SI,1
		CMP		AL,0
		JE		fin
		
		MOV		AH,0x0e		
		MOV		BX,15
		INT		0x10
		JMP		putloop
		
fin:
		HLT
		JMP		fin
		
msg:
		DB		0x0a,0x0a
		DB		"load error"
		DB		0x0a			
		DB		0
		
		RESB	0x7dfe-$		;确保启动区的最后两个字节为55AA，P26启动区

		DB		0x55, 0xaa
		
		