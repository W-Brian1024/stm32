/*
 * main.c - STM32F103C8T6 启动流程分析与内存布局实验
 *
 * 目标: 通过 LED 和验证逻辑理解 STM32 启动全过程
 *
 * 硬件: Blue Pill, LED 在 PC13 (低电平点亮)
 */

/* ============================================================
 * 头文件包含
 * ============================================================ */
#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * 宏定义
 * ============================================================ */

/* 系统配置 */
#define SYSTEM_CLOCK_HZ     8000000UL    /* HSI = 8MHz */
#define SYSTICK_LOAD_HZ     8000UL       /* 8MHz / 1000 = 1ms */

/* GPIO 配置 */
#define LED_PORT            GPIOC
#define LED_PIN             13

/* 寄存器地址定义 */
#define PERIPH_BASE         0x40000000UL
#define APB2PERIPH_BASE     0x40010000UL  /* STM32F1 APB2 外设基址 */
#define GPIOA_BASE          (APB2PERIPH_BASE + 0x0800UL)
#define GPIOB_BASE          (APB2PERIPH_BASE + 0x0C00UL)
#define GPIOC_BASE          (APB2PERIPH_BASE + 0x1000UL)

#define RCC_BASE            0x40021000UL
#define RCC_APB2ENR_OFFSET  0x18UL
#define RCC_CR_OFFSET       0x00UL
#define RCC_CFGR_OFFSET     0x04UL
#define FLASH_ACR_BASE      0x40022000UL

#define SysTick_BASE        0xE000E010UL

/* RCC 寄存器位定义 */
#define RCC_CR_HSEON        (1UL << 16)
#define RCC_CR_HSERDY       (1UL << 17)
#define RCC_CR_PLLON        (1UL << 24)
#define RCC_CR_PLLRDY       (1UL << 25)

#define RCC_CFGR_SW_PLL     (2UL << 0)
#define RCC_CFGR_SWS_MASK   (3UL << 2)
#define RCC_CFGR_SWS_PLL    (2UL << 2)
#define RCC_CFGR_HPRE_DIV1  (0UL << 4)
#define RCC_CFGR_PPRE1_DIV2 (4UL << 8)
#define RCC_CFGR_PPRE2_DIV1 (0UL << 11)
#define RCC_CFGR_PLLMUL9    (7UL << 18)

#define RCC_APB2ENR_IOPCEN  (1UL << 4)

/* GPIO 模式定义 */
#define GPIO_MODE_INPUT     0x00UL
#define GPIO_MODE_OUTPUT_10MHZ  0x01UL
#define GPIO_MODE_OUTPUT_2MHZ   0x02UL
#define GPIO_MODE_OUTPUT_50MHZ  0x03UL

/* SysTick 控制位定义 */
#define SYSTICK_CTRL_ENABLE    (1UL << 0)
#define SYSTICK_CTRL_TICKINT   (1UL << 1)
#define SYSTICK_CTRL_CLKSOURCE (1UL << 2)
#define SYSTICK_CTRL_COUNTFLAG (1UL << 16)

/* 内存地址范围 */
#define FLASH_BASE          0x08000000UL
#define FLASH_SIZE          0x00010000UL
#define RAM_BASE            0x20000000UL
#define RAM_SIZE            0x00005000UL
#define RAM_TOP             0x20005000UL

/* 验证常量 */
#define DATA_MAGIC          0xDEADBEEFUL
#define DATA_COUNTER        0x12345678UL
#define RODATA_MARK         0xCAFEBABEUL
#define SYSTICK_EXPECT_MIN  80    /* 8MHz 时钟下的期望范围 */
#define SYSTICK_EXPECT_MAX  120

/* LED 闪烁时间 */
#define LED_BLINK_SHORT_MS  200
#define LED_BLINK_LONG_MS   800
#define LED_BLINK_FAIL_MS   100

/* ============================================================
 * 类型定义
 * ============================================================ */

typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} gpio_t;

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} systick_t;

/* ============================================================
 * 全局变量声明
 * ============================================================ */

/* 硬件寄存器指针 */
#define GPIOA  ((gpio_t *)GPIOA_BASE)
#define GPIOB  ((gpio_t *)GPIOB_BASE)
#define GPIOC  ((gpio_t *)GPIOC_BASE)

#define RCC_APB2ENR  (*(volatile uint32_t *)(RCC_BASE + RCC_APB2ENR_OFFSET))
#define RCC_CR       (*(volatile uint32_t *)(RCC_BASE + RCC_CR_OFFSET))
#define RCC_CFGR     (*(volatile uint32_t *)(RCC_BASE + RCC_CFGR_OFFSET))
#define FLASH_ACR    (*(volatile uint32_t *)FLASH_ACR_BASE)

#define SysTick  ((systick_t *)SysTick_BASE)

/* .data 段: 有初始值的全局变量 */
uint32_t g_data_magic = DATA_MAGIC;
uint32_t g_data_counter = DATA_COUNTER;

/* .bss 段: 无初始值的全局变量 */
uint32_t g_bss_uninit;
uint32_t g_bss_another;

/* .rodata 段: 只读常量 */
const uint32_t g_rodata_mark = RODATA_MARK;

/* SysTick 中断计数 */
volatile uint32_t g_systick_count = 0;

/* ============================================================
 * 链接器符号声明
 * ============================================================ */
extern uint32_t _estack;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sidata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _etext;

/* ============================================================
 * 函数声明
 * ============================================================ */
static void system_clock_init(void);
static void gpio_init(void);
static void delay_ms(uint32_t ms);
static void led_blink_pass(void);
static void led_blink_fail(uint32_t code);
static void led_blink_forever(uint32_t code) __attribute__((noreturn));
static bool verify_data_section(void);
static bool verify_bss_section(void);
static bool verify_address_layout(void);
static bool verify_weak_symbol(void);

/* ============================================================
 * SysTick 中断处理函数
 * ============================================================ */
void SysTick_Handler(void)
{
    g_systick_count++;
}

/* ============================================================
 * 系统时钟初始化
 * ============================================================ */
static void system_clock_init(void)
{
    /* 使用 HSI (内部 8MHz) 作为系统时钟 */
    /* HSI 默认已启用，无需配置 */
    (void)RCC_CR;      /* 避免未使用警告 */
    (void)RCC_CFGR;    /* 避免未使用警告 */

    /* 系统运行在默认 8MHz HSI */
    /* 注意: SysTick 的 LOAD 值需要相应调整 */
}

/* ============================================================
 * GPIO 初始化
 * ============================================================ */
static void gpio_init(void)
{
    uint32_t temp;
    volatile uint32_t *cr;

    /* 使能 GPIOC 时钟 */
    RCC_APB2ENR |= RCC_APB2ENR_IOPCEN;

    /* 配置 PC13 为推挽输出, 2MHz */
    cr = &GPIOC->CRH;
    temp = *cr;
    temp &= ~(0xFUL << 20);
    temp |= (GPIO_MODE_OUTPUT_2MHZ << 20);
    *cr = temp;

    /* 初始状态: LED 熄灭 (高电平) */
    GPIOC->ODR |= (1UL << 13);
}

/* ============================================================
 * 延时函数 (基于 SysTick)
 * ============================================================ */
static void delay_ms(uint32_t ms)
{
    /* 使用 8MHz HSI 时钟: 8000 cycles = 1ms */
    while (ms--) {
        SysTick->LOAD = 8000UL - 1;
        SysTick->VAL = 0;
        SysTick->CTRL = SYSTICK_CTRL_ENABLE | SYSTICK_CTRL_CLKSOURCE;

        while (!(SysTick->CTRL & SYSTICK_CTRL_COUNTFLAG)) {
            /* 等待计数结束 */
        }

        SysTick->CTRL = 0;
    }
}

/* ============================================================
 * LED 指示函数
 * ============================================================ */
static void led_blink_pass(void)
{
    GPIOC->ODR |= (1UL << LED_PIN);      /* 高电平，LED 熄灭 */
    delay_ms(LED_BLINK_SHORT_MS);
    GPIOC->ODR &= ~(1UL << LED_PIN);     /* 低电平，LED 点亮 */
    delay_ms(LED_BLINK_LONG_MS);
}

static void led_blink_fail(uint32_t code)
{
    for (uint32_t i = 0; i < code; i++) {
        GPIOC->ODR |= (1UL << LED_PIN);  /* 高电平，LED 熄灭 */
        delay_ms(LED_BLINK_FAIL_MS);
        GPIOC->ODR &= ~(1UL << LED_PIN); /* 低电平，LED 点亮 */
        delay_ms(LED_BLINK_FAIL_MS);
    }
    delay_ms(1000);
}

static void led_blink_forever(uint32_t code)
{
    while (1) {
        led_blink_fail(code);
    }
}

/* ============================================================
 * 实验3-A: .data 段验证
 * ============================================================ */
static bool verify_data_section(void)
{
    if (g_data_magic != DATA_MAGIC) {
        return false;
    }
    if (g_data_counter != DATA_COUNTER) {
        return false;
    }
    return true;
}

/* ============================================================
 * 实验3-B: .bss 段验证
 * ============================================================ */
static bool verify_bss_section(void)
{
    if (g_bss_uninit != 0 || g_bss_another != 0) {
        return false;
    }
    return true;
}

/* ============================================================
 * 实验3-C: 地址布局验证
 * ============================================================ */
static bool verify_address_layout(void)
{
    uint32_t addr_sdata = (uint32_t)&_sdata;
    uint32_t addr_edata = (uint32_t)&_edata;
    uint32_t addr_sidata = (uint32_t)&_sidata;
    uint32_t addr_sbss = (uint32_t)&_sbss;
    uint32_t addr_ebss = (uint32_t)&_ebss;
    uint32_t addr_estack = (uint32_t)&_estack;
    uint32_t addr_etext = (uint32_t)&_etext;

    uint32_t addr_g_data_magic = (uint32_t)&g_data_magic;
    uint32_t addr_g_bss_uninit = (uint32_t)&g_bss_uninit;
    uint32_t addr_g_rodata_mark = (uint32_t)&g_rodata_mark;

    /* 验证 Flash 地址范围 */
    if (addr_etext < FLASH_BASE || addr_etext > (FLASH_BASE + FLASH_SIZE)) {
        return false;
    }
    if (addr_sidata < FLASH_BASE || addr_sidata > (FLASH_BASE + FLASH_SIZE)) {
        return false;
    }

    /* 验证 RAM 地址范围 */
    if (addr_sdata < RAM_BASE || addr_sdata > (RAM_BASE + RAM_SIZE)) {
        return false;
    }
    if (addr_sbss < RAM_BASE || addr_sbss > (RAM_BASE + RAM_SIZE)) {
        return false;
    }
    if (addr_estack != RAM_TOP) {
        return false;
    }

    /* 验证全局变量位置 */
    if (addr_g_data_magic < addr_sdata || addr_g_data_magic >= addr_edata) {
        return false;
    }
    if (addr_g_bss_uninit < addr_sbss || addr_g_bss_uninit >= addr_ebss) {
        return false;
    }
    if (addr_g_rodata_mark < FLASH_BASE || addr_g_rodata_mark > (FLASH_BASE + FLASH_SIZE)) {
        return false;
    }

    return true;
}

/* ============================================================
 * 实验3-D: 弱符号覆盖验证
 * ============================================================ */
static bool verify_weak_symbol(void)
{
    /* 配置 SysTick 为 1ms 周期 @ 8MHz */
    SysTick->LOAD = (8000UL - 1);
    SysTick->VAL = 0;
    SysTick->CTRL = SYSTICK_CTRL_ENABLE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_CLKSOURCE;

    g_systick_count = 0;
    delay_ms(100);

    SysTick->CTRL = 0;

    if (g_systick_count < SYSTICK_EXPECT_MIN || g_systick_count > SYSTICK_EXPECT_MAX) {
        return false;
    }

    return true;
}

/* ============================================================
 * 主函数
 * ============================================================ */
int main(void)
{
    system_clock_init();
    gpio_init();

    /* 实验3-A: .data 段验证 */
    if (!verify_data_section()) {
        led_blink_forever(1);
    }
    led_blink_pass();

    /* 实验3-B: .bss 段验证 */
    if (!verify_bss_section()) {
        led_blink_forever(2);
    }
    led_blink_pass();

    /* 实验3-C: 地址布局验证 */
    if (!verify_address_layout()) {
        led_blink_forever(3);
    }
    led_blink_pass();

    /* 实验3-D: 弱符号覆盖验证 */
    if (!verify_weak_symbol()) {
        led_blink_forever(4);
    }
    led_blink_pass();

    /* 全部测试通过, LED 慢闪 */
    while (1) {
        GPIOC->ODR |= (1UL << LED_PIN);   /* 高电平，LED 熄灭 */
        delay_ms(500);
        GPIOC->ODR &= ~(1UL << LED_PIN);  /* 低电平，LED 点亮 */
        delay_ms(500);
    }

    return 0;
}
