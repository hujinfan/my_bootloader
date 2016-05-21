/* NAND FLASH 控制器 */
#define NFCONF (*((volatile unsigned long *)0x4E000000))
#define NFCONT (*((volatile unsigned long *)0x4E000004))
#define NFCMMD (*((volatile unsigned long *)0x4E000008))
#define NFADDR (*((volatile unsigned char *)0x4E00000C))
#define NFDATA (*((volatile unsigned char *)0x4E000010))
#define NFSTAT (*((volatile unsigned char *)0x4E000020))

/* GPIO */
#define GPHCON              (*(volatile unsigned long *)0x56000070)
#define GPHUP               (*(volatile unsigned long *)0x56000078)

/* UART registers*/
#define ULCON0              (*(volatile unsigned long *)0x50000000)
#define UCON0               (*(volatile unsigned long *)0x50000004)
#define UFCON0              (*(volatile unsigned long *)0x50000008)
#define UMCON0              (*(volatile unsigned long *)0x5000000c)
#define UTRSTAT0            (*(volatile unsigned long *)0x50000010)
#define UTXH0               (*(volatile unsigned char *)0x50000020)
#define URXH0               (*(volatile unsigned char *)0x50000024)
#define UBRDIV0             (*(volatile unsigned long *)0x50000028)

#define TXD0READY   (1<<2)

void nand_init(void)
{
#define TACLS   0
#define TWRPH0  1
#define TWRPH1  0
	/* 设置时序 */
	NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
	/* 初始化ECC, 禁止片选, 使能NAND Flash控制器 */
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
	/* 发出命令之后需要等待一会儿 */
	for(i = 0; i < 10; i++);
}

void nand_addr(unsigned int addr)
{
	unsigned int col  = addr%2048   /* 列地址，即页内地址 2048(NAND_PAGE_SIZE)=0x800 12bit */
	unsigned int row  = addr/2048   /* 行地址，即页地址 page=row 2048(NAND_PAGE_SIZE) */
	volatile int i;

	/* NFADDR [7:0] NAND flash memory address value */
	NFADDR = col & 0xff;           /* 低8位: Column Address A0~A7 */
	for(i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;    /* 高8位: Column Address A8~A11 */
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
      * 如果NFSTAT=0&1=0 一直循环等待
      */
    while(!(NFSTAT & 1))
}

unsigned char nand_data(void)
{
	return NFDATA;
}
 
/* 要读取整块数据 */
void nand_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
	int col = src % 2048 /* 从一页中的某处读取数据 */
	int i = 0;
	/* 1.选中 */
	nand_selcet();
	while(i < len)
	{
		/* 2.发出读命令00h */
		nand_cmd(0x00);

		/* 3.发出地址(分5步发出) */
		nand_addr(addr);

		/* 4.发出读命令30h */
		nand_cmd(0x30);

		/* 5.判断状态 */
		nand_wait_ready();

		/* 6.读数据 */
		for(; (col < 2048)&&(i < len); col++)
		{
			buf[i] = nand_data();
			i++;
			addr++;
		}

		col = 0;     /* 读完一行数据之后，到下一行从头开始读 */
	}

	/* 7.取消选中 */
	nand_deselcet();
	
}

#define PCLK            50000000    // init.c中的clock_init函数设置PCLK为50MHz
#define UART_CLK        PCLK        //  UART0的时钟源设为PCLK
#define UART_BAUD_RATE  115200      // 波特率
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)

/*
 * 初始化UART0
 * 115200,8N1,无流控
 */
void uart0_init(void)
{
    GPHCON  |= 0xa0;    // GPH2,GPH3用作TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3内部上拉

    ULCON0  = 0x03;     // 8N1(8个数据位，无较验，1个停止位)
    UCON0   = 0x05;     // 查询方式，UART时钟源为PCLK
    UFCON0  = 0x00;     // 不使用FIFO
    UMCON0  = 0x00;     // 不使用流控
    UBRDIV0 = UART_BRD; // 波特率为115200
}

/*
 * 发送一个字符
 */
void putc(unsigned char c)
{
    /* 等待，直到发送缓冲区中的数据已经全部发送出去 */
    while (!(UTRSTAT0 & TXD0READY));
    
    /* 向UTXH0寄存器中写入数据，UART即自动将它发送出去 */
    UTXH0 = c;
}

void puts(char *str)
{
	int i = 0;
	while(str[i])
	{
		putc(str[i]);
		i++;
	}
}

#if 0
/*
 * 接收字符
 */
unsigned char getc(void)
{
    /* 等待，直到接收缓冲区中的有数据 */
    while (!(UTRSTAT0 & RXD0READY));
    
    /* 直接读取URXH0寄存器，即可获得接收到的数据 */
    return URXH0;
}

/*
 * 判断一个字符是否数字
 */
int isDigit(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return 1;
    else
        return 0;       
}

/*
 * 判断一个字符是否英文字母
 */
int isLetter(unsigned char c)
{
    if (c >= 'a' && c <= 'z')
        return 1;
    else if (c >= 'A' && c <= 'Z')
        return 1;       
    else
        return 0;
}
#endif


int isBootFromNorFlash(void)
{
	volatile int *p = (volatile int *)0;   /* 定义一个指针指向0地址处 */
	int val;

	val = *p;
	*p = 0x12345678;           /* 向0地址处写值 */

	if(*p == 0x12345678)
	{
		/* 写成功，为NAND启动 */
		*p = val;  /* 恢复0地址处的值 */
		return 0;
	}
	else
	{
		/* NOR 启动: NOR不能像内存一样写 */
		return 1;
	}
}

/* copy_code_to_sdram()函数的参数从哪里来: 汇编代码中 */
void copy_code_to_sdram(unsigned char *src, unsigned char *dest, 
                                unsigned int len)
{
	int i = 0;
	/* 如果是NOR启动:片内内存不可写 */
	if(isBootFromNorFlash())
	{
		/* 进行简单的拷贝 */
		while(i < len)
		{
			dest[i] = src[i];
			i++;
		}
	}
	/* Nand 启动 */
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


