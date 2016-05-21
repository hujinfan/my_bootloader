
#define S3C2440_MPLL_200MHZ     ((0x5c<<12)|(0x01<<4)|(0x02))

/* SDRAM regisers */
#define     MEM_CTL_BASE    0x48000000
#define     SDRAM_BASE      0x30000000


.text
.global _start
_start:


/* 1. 关闭看门狗 */
	ldr r0, =0x53000000   //伪汇编指令ldr
	mov r1, #0            //立即数指令mov
	str r1, [r0]
	
/* 2. 设置时钟 */
	ldr r0, =0x4c000014
	mov r1, #0x03     // FCLK:HCLK:PCLK=1:2:4, HDIVN=1,PDIVN=1
	str r1, [r0]

	 /* 如果HDIVN非0，CPU的总线模式应该从“fast bus mode”变为“asynchronous bus mode” */
    mrc    p15, 0, r1, c1, c0, 0       /* 读出控制寄存器 */ 
    orr    r1, r1, #0xc0000000         /* 设置为“asynchronous bus mode” */
    mcr    p15, 0, r1, c1, c0, 0        /* 写入控制寄存器 */

	/* 设置时钟频率 */
	ldr r0, =0x4c000004
	ldr r1, =S3C2440_MPLL_200MHZ      /* 现在，FCLK=200MHz,HCLK=100MHz,PCLK=50MHz */
	str r1, [r0]


/* 3. 初始化SDRAM */
	ldr r0, =MEM_CTL_BASE
	adr r1, sdram_config       /* 得到sdram_config标号的当前地址 */
	add r3, r0, #(13*4)        /* r3 = r0 + 52 */

1:
	/* copy */
	ldr r2, [r1], #4          /* 从r1处读值存入r2，然后让r1+4 */
	str r2, [r0], #4          /* 从r2处读值存入r0，然后让r0+4 */
	cmp r0, r3                /* 比较r0和r3的值 */
	bne 1b                    /* 如果r0与r3不相等, 跳回到前面(b-back)的1标号处 */
	
/* 4. 重定位: 把bootloader本身的代码从flash复制到它的链接地址处，使用C函数来实现 */
	ldr sp, =0x34000000       /* 设置栈，64M,基地址为0x30000000, 64M=4000000 */

	bl nand_init

	mov r0, #0                /* 第一个参数为源，  r0=0 从0地址处开始读数据 */
	ldr r1, =_start           /* 第二个参数为目的，第一条指令标号链接地址处 */
	ldr r2, =__bss_start      /* 第三个参数为r2 = len = r2-r1 */
	sub r2, r2, r1            
	
	bl copy_code_to_sdram
	bl clean_bss

	

/* 5. 执行main */
	ldr lr, =halt
	ldr pc, =main          /* 跳到main函数执行 */

halt:
	b halt                /* 如果main函数有返回，则跳转到halt处死循环 */

sdram_config:
	.long 0x22011110	 //BWSCON
	.long 0x00000700	 //BANKCON0
	.long 0x00000700	 //BANKCON1
	.long 0x00000700	 //BANKCON2
	.long 0x00000700	 //BANKCON3  
	.long 0x00000700	 //BANKCON4
	.long 0x00000700	 //BANKCON5
	.long 0x00018005	 //BANKCON6
	.long 0x00018005	 //BANKCON7
	.long 0x008C04F4	 // REFRESH
	.long 0x000000B1	 //BANKSIZE
	.long 0x00000030	 //MRSRB6
	.long 0x00000030	 //MRSRB7

