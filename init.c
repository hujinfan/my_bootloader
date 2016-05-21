
#define NFCONF (*((volatile unsigned long *)0x4E000000))
#define NFCONT (*((volatile unsigned long *)0x4E000004))
#define NFCMMD (*((volatile unsigned long *)0x4E000008))
#define NFADDR (*((volatile unsigned char *)0x4E00000C))
#define NFDATA (*((volatile unsigned char *)0x4E000010))
#define NFSTAT (*((volatile unsigned char *)0x4E000020))


void nand_init(void)
{
#define TACLS   0
#define TWRPH0  1
#define TWRPH1  0
	/* ����ʱ�� */
	NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
	/* ��ʼ��ECC, ��ֹƬѡ, ʹ��NAND Flash������ */
	NFCONT = (1<<4)|(1<<1)|(1<<0);	
}

/* NFCONT [1] 0: Force nFCE to low (Enable chip select)
 *            1: Force nFCE to high (Disable chip select)
 */
void nand_selcet(void)
{
	NFCONT &= ~(1<<1);
}

void nand_deselcet(void)
{
	NFCONT |= (1<<1);
}

void nand_cmd(unsigned char cmd)
{
	volatile int i;
	NFCMMD = cmd;
	/* ��������֮����Ҫ�ȴ�һ��� */
	for(i = 0; i < 10; i++);
}

void nand_addr(unsigned int addr)
{
	unsigned int col  = addr%2048   /* �е�ַ����ҳ�ڵ�ַ 2048(NAND_PAGE_SIZE)=0x800 12bit */
	unsigned int row  = addr/2048   /* �е�ַ����ҳ��ַ page=row 2048(NAND_PAGE_SIZE) */
	volatile int i;

	/* NFADDR [7:0] NAND flash memory address value */
	NFADDR = col & 0xff;           /* ��8λ: Column Address A0~A7 */
	for(i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;    /* ��8λ: Column Address A8~A11 */
	for(i = 0; i < 10; i++);

	NFADDR = row & 0xff;           /* Row Address A12~A19 */	
	for(i = 0; i < 10; i++);
	NFADDR = (row >> 8) & 0xff;    /* Row Address A20~A27 */	
	for(i = 0; i < 10; i++);
	NFADDR = (row >> 16)& 0xff;    /* Row Address A28 */	
	for(i = 0; i < 10; i++);

}

void nand_wait_ready(void)
{
	/* RnB [0]  0: NAND Flash memory busy
      *          1: NAND Flash memory ready to operate 
      * ���NFSTAT=0&1=0 һֱѭ���ȴ�
      */
    while(!(NFSTAT & 1))
}

unsigned char nand_data(void)
{
	return NFDATA;
}
 
/* Ҫ��ȡ�������� */
void nand_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
	int col = src % 2048 /* ��һҳ�е�ĳ����ȡ���� */
	int i = 0;
	/* 1.ѡ�� */
	nand_selcet();
	while(i < len)
	{
		/* 2.����������00h */
		nand_cmd(0x00);

		/* 3.������ַ(��5������) */
		nand_addr(addr);

		/* 4.����������30h */
		nand_cmd(0x30);

		/* 5.�ж�״̬ */
		nand_wait_ready();

		/* 6.������ */
		for(; (col < 2048)&&(i < len); col++)
		{
			buf[i] = nand_data();
			i++;
			addr++;
		}

		col = 0;     /* ����һ������֮�󣬵���һ�д�ͷ��ʼ�� */
	}

	/* 7.ȡ��ѡ�� */
	nand_deselcet();
	
}

int isBootFromNorFlash(void)
{
	volatile int *p = (volatile int *)0;   /* ����һ��ָ��ָ��0��ַ�� */
	int val;

	val = *p;
	*p = 0x12345678;           /* ��0��ַ��дֵ */

	if(*p == 0x12345678)
	{
		/* д�ɹ���ΪNAND���� */
		*p = val;  /* �ָ�0��ַ����ֵ */
		return 0;
	}
	else
	{
		/* NOR ����: NOR�������ڴ�һ��д */
		return 1;
	}
}

/* copy_code_to_sdram()�����Ĳ�����������: �������� */
void copy_code_to_sdram(unsigned char *src, unsigned char *dest, 
                                unsigned int len)
{
	int i = 0;
	/* �����NOR����:Ƭ���ڴ治��д */
	if(isBootFromNorFlash())
	{
		/* ���м򵥵Ŀ��� */
		while(i < len)
		{
			dest[i] = src[i];
			i++;
		}
	}
	/* Nand ���� */
	else
	{
		nand_read(src, dest, len);
	}
}

void clean_bss(void)
{
	extern int __bss_start, __bss_end;
    int *p = &__bss_start;
    
    for (; p < &__bss_end; p++)
        *p = 0;
}


