#include "setup.h"

extern void uart0_init(void);
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

	params->u.mem.start = 0x30000000; /* �ڴ����ʼ��ַ */
	params->u.mem.size =  64*1024*1024;  /* �ڴ�Ĵ�С:64M=4000000 */

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
	/* 0. ���ں����ô���: �ں������Ŀ�ʼ���ֻ�Ӵ��ڴ�ӡһЩ��Ϣ�������ں�һ��ʼû�г�ʼ������ */
	uart0_init();

	/* ����һ����ָ�� */
	void (*theKernel)(int zero, int arch, unsigned int params);
	
	/* 1.��NAND FLASH����ں˶����ڴ� 
	 *   ��0x60000+64��ַ����2M(0x200000)���ݵ�0x30008000��ַ��
	 */
	puts("Copy kernel from nand\n\r");
	nand_read(0x60000+64, (unsigned char *)0x30008000, 0x200000); 

	/* 2.���ò��� */
	puts("Set boot params\n\r");
	setup_start_tag();
	setup_memory_tags();
	setup_commandline_tag("noinitrd root=/dev/mtdblock3 init=/linuxrc console=ttySAC0");
	setup_end_tag();
	

	/* 3.��תִ��:����0x30008000��ִ�� */
	puts("Boot kernel\n\r");
	theKernel = (void (*)(int, int, unsigned int))0x30008000;
	theKernel(0, 362, 0x30000100);
	/*
	 * mov r0  #0
	 * ldr r1, =362        @����ID=362
	 * ldr r2, =0x30000100
	 * mov pc, #0x30008000 
	 */

	/* ���һ������������ִ�е����� */	
	puts("Error!\n\r");

	return -1;
}

