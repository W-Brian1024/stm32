/*
 * main.c - STM32F103C8T6 实验2: 延时实现（阻塞 vs 精确）
 *
 * 目标: Blue Pill 开发板, LED 在 PC13 (低电平点亮)
 *
 * 对比两种延时方式:
 *   1. 软件延时 (空循环) — 不精确
 *   2. SysTick 硬件延时   — 精确
 *
 * SysTick 是 Cortex-M3 内核自带的 24 位递减计数器
 * 72MHz 下, 计 72000 个周期 = 1ms
 */

/* ============================================================
 * 第 1 步: 寄存器定义 (复用实验1)
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

#define RCC_APB2ENR  (*(volatile unsigned int *)0x40021018)

/* ============================================================
 * 第 2 步: 系统时钟配置 (HSI 8MHz → PLL ×9 → 72MHz)
 *
 * STM32F103 上电默认使用 HSI (8MHz 内部 RC 振荡器)
 * 要达到 72MHz 需要配置 PLL 倍频
 *
 * 时钟树路径:
 *   HSI (8MHz) → PLL ×9 → SYSCLK (72MHz) → HCLK (72MHz)
 *                                          → APB2 (72MHz)
 *                                          → APB1 (36MHz)
 *
 * RCC 关键寄存器:
 *   RCC_CR     (0x40021000) - 时钟控制, 控制 HSI/HSE/PLL 使能和就绪标志
 *   RCC_CFGR   (0x40021004) - 时钟配置, 选择时钟源和分频
 *   RCC_APB2ENR(0x40021018) - APB2 外设时钟使能
 * ============================================================ */

#define RCC_CR    (*(volatile unsigned int *)0x40021000)
#define RCC_CFGR  (*(volatile unsigned int *)0x40021004)

/* RCC_CR 位定义 */
#define RCC_CR_HSION      (1UL << 0)    /* HSI 使能 */
#define RCC_CR_HSIRDY     (1UL << 1)    /* HSI 就绪标志 */
#define RCC_CR_PLLON      (1UL << 24)   /* PLL 使能 */
#define RCC_CR_PLLRDY     (1UL << 25)   /* PLL 就绪标志 */

/* RCC_CFGR 位定义 */
#define RCC_CFGR_SW_HSI   (0UL << 0)    /* 系统时钟选 HSI */
#define RCC_CFGR_SW_PLL   (2UL << 0)    /* 系统时钟选 PLL */
#define RCC_CFGR_SWS_MASK (3UL << 2)    /* 系统时钟状态掩码 */
#define RCC_CFGR_SWS_PLL  (2UL << 2)    /* 系统时钟状态: PLL */
#define RCC_CFGR_HPRE_DIV1   (0UL << 4) /* AHB 不分频 */
#define RCC_CFGR_PPRE1_DIV2  (4UL << 8) /* APB1 2分频 (36MHz) */
#define RCC_CFGR_PPRE2_DIV1  (0UL << 11)/* APB2 不分频 (72MHz) */
#define RCC_CFGR_PLLSRC_HSI  (0UL << 16)/* PLL 时钟源选 HSI/2 = 4MHz */
#define RCC_CFGR_PLLMUL9  (7UL << 18)   /* PLL 倍频 ×9 */

/*
 * 配置系统时钟到 72MHz
 *
 * 步骤:
 *   1. 确保 HSI 使能并就绪 (上电默认就是)
 *   2. 配置 AHB/APB 分频
 *   3. 配置 PLL 时钟源和倍频系数: HSI/2 × 9 = 4MHz × 9 = 36MHz
 *      注意: HSI 作为 PLL 源时, 固定先 2 分频, 即 8/2=4MHz
 *      所以 ×9 = 36MHz, 不是 72MHz!
 *   4. 使能 PLL, 等待就绪
 *   5. 切换系统时钟到 PLL
 *
 * 重要发现: HSI 作为 PLL 源时, 最高只能到 36MHz (4MHz × 9)
 *           要达到 72MHz, 需要用 HSE (外部晶振) 作为 PLL 源
 *           HSE 8MHz × 9 = 72MHz
 *
 * 但 Blue Pill 板上有 8MHz 外部晶振, 所以我们用 HSE
 */
void system_clock_init(void)
{
    /* --- 第 1 步: 使能 HSE 并等待就绪 --- */
    RCC_CR |= (1UL << 16);   /* HSEON: 使能外部高速时钟 */
    while (!(RCC_CR & (1UL << 17)))  /* HSERDY: 等待 HSE 就绪 */
        ;

    /* --- 第 2 步: 配置 Flash 等待状态 ---
     * 72MHz 需要 2 个等待周期, 否则 Flash 读取跟不上 CPU
     * FLASH_ACR 在 0x40022000 */
    *(volatile unsigned int *)0x40022000 = 0x00000012;  /* PRFTBE=1, LATENCY=2 */

    /* --- 第 3 步: 配置 AHB/APB 分频 --- */
    /* AHB 不分频 (72MHz), APB1 2分频 (36MHz), APB2 不分频 (72MHz) */
    RCC_CFGR = RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV1;

    /* --- 第 4 步: 配置 PLL: HSE × 9 = 72MHz --- */
    RCC_CFGR |= (1UL << 16);   /* PLLSRC=1: PLL 源选 HSE */
    RCC_CFGR &= ~(1UL << 17);  /* PLLXTPRE=0: HSE 不分频 */
    RCC_CFGR |= RCC_CFGR_PLLMUL9;  /* PLL 倍频 ×9 */

    /* --- 第 5 步: 使能 PLL 并等待就绪 --- */
    RCC_CR |= RCC_CR_PLLON;
    while (!(RCC_CR & RCC_CR_PLLRDY))
        ;

    /* --- 第 6 步: 切换系统时钟到 PLL --- */
    RCC_CFGR &= ~(3UL << 0);       /* 清除 SW 位 */
    RCC_CFGR |= RCC_CFGR_SW_PLL;   /* 选择 PLL 作为系统时钟 */
    while ((RCC_CFGR & RCC_CFGR_SWS_MASK) != RCC_CFGR_SWS_PLL)
        ;

    /* 现在 SYSCLK = 72MHz */
}

/* ============================================================
 * 第 3 步: GPIO 操作宏 (复用实验1)
 * ============================================================ */

#define GPIO_CLK_ENABLE(port) \
    (RCC_APB2ENR |= (1 << ((((unsigned int)(port) - 0x40010800) / 0x400) + 2)))

#define GPIO_SET_OUTPUT(port, pin) do {                                 \
    volatile unsigned int *cr = ((pin) < 8) ? &(port)->CRL : &(port)->CRH; \
    unsigned int shift = ((pin) < 8) ? ((pin) * 4) : (((pin) - 8) * 4); \
    *cr &= ~(0xFUL << shift);                                           \
    *cr |=  (0x2UL << shift);                                           \
} while(0)

#define PIN_ON(port, pin)    ((port)->BSRR = (1 << ((pin) + 16)))  /* 输出低电平 */
#define PIN_OFF(port, pin)   ((port)->BSRR = (1 << (pin)))         /* 输出高电平 */
#define PIN_TOGGLE(port, pin) ((port)->ODR ^= (1 << (pin)))        /* 翻转 */

/* ============================================================
 * 第 3 步: SysTick 寄存器定义
 *
 * SysTick 有 4 个寄存器:
 *   CTRL (0xE000E010) - 控制寄存器
 *     bit0  ENABLE    - 使能
 *     bit1  TICKINT   - 中断使能 (计数到0时触发 SysTick_Handler)
 *     bit2  CLKSOURCE - 0=HCLK/8(9MHz), 1=HCLK(72MHz)
 *     bit16 COUNTFLAG - 计数到0时硬件置1, 读取后自动清零
 *   LOAD (0xE000E014) - 重装载值 (24位, 最大 0xFFFFFF = 16777215)
 *   VAL  (0xE000E018) - 当前计数值, 写任意值清零并重置 COUNTFLAG
 *   CALIB (0xE000E01C)- 校准值 (只读, 厂家预设)
 * ============================================================ */

typedef struct {
    volatile unsigned int CTRL;    /* 0xE000E010 */
    volatile unsigned int LOAD;    /* 0xE000E014 */
    volatile unsigned int VAL;     /* 0xE000E018 */
    volatile unsigned int CALIB;   /* 0xE000E01C */
} SysTick_TypeDef;

#define SysTick  ((SysTick_TypeDef *)0xE000E010)

/* CTRL 寄存器位定义 */
#define SysTick_CTRL_ENABLE     (1UL << 0)   /* 使能计数器 */
#define SysTick_CTRL_TICKINT    (1UL << 1)   /* 使能中断 */
#define SysTick_CTRL_CLKSOURCE  (1UL << 2)   /* 1=HCLK(72MHz), 0=HCLK/8 */
#define SysTick_CTRL_COUNTFLAG  (1UL << 16)  /* 计数到0标志 */

/* ============================================================
 * 第 4 步: 延时函数实现
 * ============================================================ */

/* --- 方法1: 软件延时 (不精确) ---
 *
 * 原理: 纯粹消耗 CPU 周期
 * 缺点:
 *   1. 编译器优化级别不同, 延时时间会变
 *   2. 中断会打断循环, 导致时间变长
 *   3. 无法精确计算延时了多少毫秒
 */
void delay_soft(volatile unsigned int count)
{
    while (count--) {
        /* 空循环, 纯消耗时间 */
    }
}

/* --- 方法2: SysTick 精确延时 (阻塞查询方式) ---
 *
 * 原理: 硬件计数器从 LOAD 值递减到 0, COUNTFLAG 置位
 * 72MHz 下:
 *   1us = 72 个周期    → LOAD = 72 - 1
 *   1ms = 72000 个周期 → LOAD = 72000 - 1
 *
 * 24位计数器最大值 = 0xFFFFFF = 16777215
 * 最大单次延时 = 16777215 / 72000 ≈ 233ms
 * 超过 233ms 需要循环调用
 */
void delay_us(unsigned int us)
{
    SysTick->LOAD = (72UL * us) - 1;     /* 设置计数值 */
    SysTick->VAL  = 0;                    /* 清空当前值, 同时清 COUNTFLAG */
    SysTick->CTRL = SysTick_CTRL_ENABLE   /* 使能 */
                  | SysTick_CTRL_CLKSOURCE; /* 使用 72MHz 时钟 */

    /* 等待 COUNTFLAG 置位 (计数器减到 0) */
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG))
        ;

    SysTick->CTRL = 0;  /* 关闭计数器 */
}

void delay_ms(unsigned int ms)
{
    while (ms--) {
        delay_us(1000);  /* 每次 1ms */
    }
}

/* ============================================================
 * 第 5 步: main 函数
 *
 * 演示计划:
 *   阶段1: 软件延时闪 LED (肉眼看不出区别, 但时间不精确)
 *   阶段2: SysTick 精确延时闪 LED (精确 500ms)
 * ============================================================ */

int main(void)
{
    /* 第一步: 配置系统时钟到 72MHz (HSE × 9) */
    system_clock_init();

    /* 使能 GPIOC 时钟 */
    GPIO_CLK_ENABLE(GPIOC);

    /* 配置 PC13 为推挽输出 */
    GPIO_SET_OUTPUT(GPIOC, 13);

    while (1) {
        // /* --- 阶段1: 软件延时 --- */
        // /* 这个延时大约几百万次循环, 时间不精确 */
        // PIN_ON(GPIOC, 13);     /* LED 亮 (低电平) */
        // delay_soft(500000);
        // PIN_OFF(GPIOC, 13);    /* LED 灭 (高电平) */
        // delay_soft(500000);

        /* --- 阶段2: SysTick 精确延时 --- */
        /* 精确 500ms 亮, 500ms 灭 */
        PIN_ON(GPIOC, 13);
        delay_ms(500);
        PIN_OFF(GPIOC, 13);
        delay_ms(500);
    }
}
