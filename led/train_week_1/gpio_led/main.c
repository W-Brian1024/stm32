/*
 * main.c - STM32F103C8T6 寄存器级点灯 (无 HAL)
 *
 * 目标: Blue Pill 开发板, LED 在 PC13 (低电平点亮)
 *
 * 通用 GPIO 操作宏, 可用于任意端口和引脚
 */

/* ============================================================
 * 第 1 步: 定义寄存器结构体和基地址
 *
 * GPIO 寄存器组 (每组 7 个寄存器, 偏移固定):
 *   CRL  (0x00) - 控制引脚 0-7
 *   CRH  (0x04) - 控制引脚 8-15
 *   IDR  (0x08) - 输入数据 (只读)
 *   ODR  (0x0C) - 输出数据
 *   BSRR (0x10) - 置位/复位 (原子操作)
 *   BRR  (0x14) - 复位 (原子操作)
 *   LCKR (0x18) - 配置锁定
 * ============================================================ */

typedef struct {
    volatile unsigned int CRL;   /* 0x00 */
    volatile unsigned int CRH;   /* 0x04 */
    volatile unsigned int IDR;   /* 0x08 */
    volatile unsigned int ODR;   /* 0x0C */
    volatile unsigned int BSRR;  /* 0x10 */
    volatile unsigned int BRR;   /* 0x14 */
    volatile unsigned int LCKR;  /* 0x18 */
} GPIO_TypeDef;

#define GPIOA  ((GPIO_TypeDef *)0x40010800)
#define GPIOB  ((GPIO_TypeDef *)0x40010C00)
#define GPIOC  ((GPIO_TypeDef *)0x40011000)
#define GPIOD  ((GPIO_TypeDef *)0x40011400)

/* RCC 寄存器 */
#define RCC_APB2ENR  (*(volatile unsigned int *)0x40021018)

/* ============================================================
 * 第 2 步: 通用 GPIO 操作宏
 * ============================================================ */

/* 使能 GPIO 端口时钟
 *   GPIOA → bit2, GPIOB → bit3, GPIOC → bit4, GPIOD → bit5
 *   port 是 GPIOA/GPIOB/... 宏, 通过地址偏移算出 bit 位置 */
#define GPIO_CLK_ENABLE(port) \
    (RCC_APB2ENR |= (1 << ((((unsigned int)(port) - 0x40010800) / 0x400) + 2)))

/* 配置引脚为推挽输出 (2MHz)
 *   pin < 8  用 CRL, pin >= 8 用 CRH
 *   每个引脚占 4 bit: CNF[1:0] MODE[1:0]
 *   推挽输出 2MHz = 0b0010 = 0x2 */
#define GPIO_SET_OUTPUT(port, pin) do {                                 \
    volatile unsigned int *cr = ((pin) < 8) ? &(port)->CRL : &(port)->CRH; \
    unsigned int shift = ((pin) < 8) ? ((pin) * 4) : (((pin) - 8) * 4); \
    *cr &= ~(0xFUL << shift);                                           \
    *cr |=  (0x2UL << shift);                                           \
} while(0)

/* 引脚控制 (BSRR: 原子操作)
 *   BSRR 低 16 bit 置位, 高 16 bit 复位 */
#define PIN_ON(port, pin)    ((port)->BSRR = (1 << ((pin) + 16)))  /* 输出低电平 */
#define PIN_OFF(port, pin)   ((port)->BSRR = (1 << (pin)))         /* 输出高电平 */
#define PIN_TOGGLE(port, pin) ((port)->ODR ^= (1 << (pin)))        /* 翻转 */

/* ============================================================
 * 第 3 步: 延时函数
 * ============================================================ */

void delay(volatile unsigned int count)
{
    while (count--) {
        /* 纯消耗时间 */
    }
}

/* ============================================================
 * 第 4 步: main 函数
 * ============================================================ */

int main(void)
{
    /* 使能 GPIOC 时钟 */
    GPIO_CLK_ENABLE(GPIOC);

    /* 配置 PC13 为推挽输出 */
    GPIO_SET_OUTPUT(GPIOC, 13);

    /* 循环翻转 PC13, LED 闪烁 */
    while (1) {
        PIN_ON(GPIOC, 13);
        delay(500000);
    }
}
