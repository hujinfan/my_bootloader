
#define S3C2440_MPLL_200MHZ     ((0x5c<<12)|(0x01<<4)|(0x02))

/* SDRAM regisers */
#define     MEM_CTL_BASE    0x48000000
#define     SDRAM_BASE      0x30000000


.text
.global _start
_start:


/* 1. �رտ��Ź� */
	ldr r0, =0x53000000   //α���ָ��ldr
	mov r1, #0            //������ָ��mov
	str r1, [r0]
	
/* 2. ����ʱ�� */
	ldr r0, =0x4c000014
	mov r1, #0x03     // FCLK:HCLK:PCLK=1:2:4, HDIVN=1,PDIVN=1
	str r1, [r0]

	 /* ���HDIVN��0��CPU������ģʽӦ�ôӡ�fast bus mode����Ϊ��asynchronous bus mode�� */
    mrc    p15, 0, r1, c1, c0, 0       /* �������ƼĴ��� */ 
    orr    r1, r1, #0xc0000000         /* ����Ϊ��asynchronous bus mode�� */
    mcr    p15, 0, r1, c1, c0, 0        /* д����ƼĴ��� */

	/* ����ʱ��Ƶ�� */
	ldr r0, =0x4c000004
	ldr r1, =S3C2440_MPLL_200MHZ      /* ���ڣ�FCLK=200MHz,HCLK=100MHz,PCLK=50MHz */
	str r1, [r0]


/* 3. ��ʼ��SDRAM */
	ldr r0, =MEM_CTL_BASE
	ldr r1, sdram_config       /* �õ�sdram_config��ŵĵ�ǰ��ַ */
	add r3, r0, #(13*4)        /* r3 = r0 + 52 */

1:
	/* copy */
	ldr r2, [r1], #4          /* ��r1����ֵ����r2��Ȼ����r1+4 */
	str r2, [r0], #4          /* ��r2����ֵ����r0��Ȼ����r0+4 */
	cmp r0, r3                /* �Ƚ�r0��r3��ֵ */
	bne 1b                    /* ���r0��r3�����, ���ص�ǰ��(b-back)��1��Ŵ� */
	
/* 4. �ض�λ: ��bootloader�����Ĵ����flash���Ƶ��������ӵ�ַ����ʹ��C������ʵ�� */
	ldr sp, =0x34000000       /* ����ջ��64M,����ַΪ0x30000000, 64M=4000000 */

	bl nand_init

	mov r0, #0                /* ��һ������ΪԴ��  r0=0 ��0��ַ����ʼ������ */
	ldr r1, =_start           /* �ڶ�������ΪĿ�ģ���һ��ָ�������ӵ�ַ�� */
	ldr r2, =__bss_start      /* ����������Ϊr2 = len = r2-r1 */
	sub r2, r2, r1            
	
	bl copy_code_to_sdram
	bl clean_bss

	

/* 5. ִ��main */
	ldr lr, =halt
	ldr pc, =main          /* ����main����ִ�� */

halt:
	b halt                /* ���main�����з��أ�����ת��halt����ѭ�� */

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
