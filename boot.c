#include "setup.h"

extern uart0_init();
extern void puts(char *str);
extern void nand_read(unsigned int addr, unsigned char *buf, unsigned int len);


static struct tag *params;

void setup_start_tag(void)
{
	params = (struct tag *)0x30000100;

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size(tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next(params);
}

void setup_memory_tags(void)
{
	params->hdr.tag = ATAG_MEM;
	params->hdr.size = tag_size(tag_mem32);

	params->u.mem.start = 0x30000000; /* 内存的起始地址 */
	params->u.mem.size =  64*1024*1024;  /* 内存的大小:64M=4000000 */

	params = tag_next(params);
}

int strlen(char *str)
{
	int i = 0;
	while(str[i])
	{
		i++;
	}
	return i;
}

void strcpy(char *dest, char *src)
{
	while((*dest++ = *src++) != '\0');
}

void setup_commandline_tag(char *cmdline)
{
	int len = strlen(cmdline) + 1;
	params->hdr.tag  = ATAG_CMDLINE;
	params->hdr.size = (sizeof(struct tag_header) + len + 3) >> 2;

	strcpy (params->u.cmdline.cmdline, cmdline);

	params = tag_next(params);
}

void setup_end_tag(void)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}



int main(void)
{
	/* 0. 帮内核设置串口: 内核启动的开始部分会从串口打印一些信息，但是内核一开始没有初始化串口 */
	uart0_init();

	/* 定义一函数指针 */
	void (*theKernel)(int zero, int arch, unsigned int params);
	
	/* 1.从NAND FLASH里把内核读入内存 
	 *   从0x60000+64地址处读2M(0x200000)内容到0x30008000地址处
	 */
	puts("Copy kernel from nand\n\r");
	nand_read(0x60000+64, (unsigned char *)0x30008000, 0x200000); 

	/* 2.设置参数 */
	puts("Set boot params\n\r");
	setup_start_tag();
	setup_memory_tags();
	setup_commandline_tag("noinitrd root=/dev/mtdblock3 init=/linuxrc console=ttySAC0");
	setup_end_tag();
	

	/* 3.跳转执行:跳到0x30008000处执行 */
	puts("Boot kernel\n\r");
	theKernel = (void (*)(int, int, unsigned int))0x30008000;
	theKernel(0, 362, 0x30000100);
	/*
	 * mov r0  #0
	 * ldr r1, =362        @机器ID=362
	 * ldr r2, =0x30000100
	 * mov pc, #0x30008000 
	 */

	/* 如果一切正常，不会执行到这里 */	
	puts("Error!\n\r");

	return -1;
}

