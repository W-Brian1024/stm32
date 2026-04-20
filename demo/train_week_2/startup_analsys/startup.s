/* #######################################################################
 * startup.s - STM32F103C8T6 启动汇编代码
 * #######################################################################
 *
 * 职责:
 *   1. 定义中断向量表 (位于 Flash 起始地址 0x08000000)
 *   2. 实现 Reset_Handler 初始化流程
 *   3. 提供默认中断处理函数
 *
 * 启动流程:
 *   Reset_Handler:
 *     1. 拷贝 .data 段: Flash (_sidata) -> RAM (_sdata ~ _edata)
 *     2. 清零 .bss 段: RAM (_sbss ~ _ebss)
 *     3. 跳转到 main() 函数
 *
 * ####################################################################### */

/* =======================================================================
 * 汇编指令设置
 * ======================================================================= */

.syntax unified    /* 统一 ARM/Thumb 语法 */
.cpu cortex-m3     /* 目标 CPU */
.fpu softvfp       /* 软件浮点 */
.thumb             /* Thumb 指令集 */

/* =======================================================================
 * 全局符号声明
 * ======================================================================= */

.global Reset_Handler
.global g_pfnVectors
.global Default_Handler

/* =======================================================================
 * 中断向量表定义
 * ======================================================================= */

.section .isr_vector, "a", %progbits
.type g_pfnVectors, %object
.size g_pfnVectors, . - g_pfnVectors

g_pfnVectors:
    .word _estack                     /* [0]  初始栈指针 */
    .word Reset_Handler               /* [1]  复位处理 */
    .word NMI_Handler                 /* [2]  不可屏蔽中断 */
    .word HardFault_Handler           /* [3]  硬件错误 */
    .word MemManage_Handler           /* [4]  内存管理错误 */
    .word BusFault_Handler            /* [5]  总线错误 */
    .word UsageFault_Handler          /* [6]  用法错误 */
    .word 0                           /* [7]  保留 */
    .word 0                           /* [8]  保留 */
    .word 0                           /* [9]  保留 */
    .word 0                           /* [10] 保留 */
    .word SVC_Handler                 /* [11] 系统服务调用 */
    .word DebugMon_Handler            /* [12] 调试监控 */
    .word 0                           /* [13] 保留 */
    .word PendSV_Handler              /* [14] PendSV */
    .word SysTick_Handler             /* [15] SysTick 定时器 */

    /* 外部中断向量 [16-58] */
    .word WWDG_IRQHandler             /* [16] 窗口看门狗 */
    .word PVD_IRQHandler              /* [17] 电源电压检测 */
    .word TAMPER_IRQHandler           /* [18] 侵入检测 */
    .word RTC_IRQHandler              /* [19] 实时时钟 */
    .word FLASH_IRQHandler            /* [20] Flash 存储 */
    .word RCC_IRQHandler              /* [21] 复位和时钟控制 */
    .word EXTI0_IRQHandler            /* [22] 外部中断 0 */
    .word EXTI1_IRQHandler            /* [23] 外部中断 1 */
    .word EXTI2_IRQHandler            /* [24] 外部中断 2 */
    .word EXTI3_IRQHandler            /* [25] 外部中断 3 */
    .word EXTI4_IRQHandler            /* [26] 外部中断 4 */
    .word DMA1_Channel1_IRQHandler    /* [27] DMA1 通道 1 */
    .word DMA1_Channel2_IRQHandler    /* [28] DMA1 通道 2 */
    .word DMA1_Channel3_IRQHandler    /* [29] DMA1 通道 3 */
    .word DMA1_Channel4_IRQHandler    /* [30] DMA1 通道 4 */
    .word DMA1_Channel5_IRQHandler    /* [31] DMA1 通道 5 */
    .word DMA1_Channel6_IRQHandler    /* [32] DMA1 通道 6 */
    .word DMA1_Channel7_IRQHandler    /* [33] DMA1 通道 7 */
    .word ADC1_2_IRQHandler           /* [34] ADC1 和 ADC2 */
    .word USB_HP_CAN1_TX_IRQHandler   /* [35] USB 高优先级 / CAN1 发送 */
    .word USB_LP_CAN1_RX0_IRQHandler  /* [36] USB 低优先级 / CAN1 接收 0 */
    .word CAN1_RX1_IRQHandler         /* [37] CAN1 接收 1 */
    .word CAN1_SCE_IRQHandler         /* [38] CAN1 SCE */
    .word EXTI9_5_IRQHandler          /* [39] 外部中断 9-5 */
    .word TIM1_BRK_IRQHandler         /* [40] 定时器 1 刹车 */
    .word TIM1_UP_IRQHandler          /* [41] 定时器 1 更新 */
    .word TIM1_TRG_COM_IRQHandler     /* [42] 定时器 1 触发和通信 */
    .word TIM1_CC_IRQHandler          /* [43] 定时器 1 捕获比较 */
    .word TIM2_IRQHandler             /* [44] 定时器 2 */
    .word TIM3_IRQHandler             /* [45] 定时器 3 */
    .word TIM4_IRQHandler             /* [46] 定时器 4 */
    .word I2C1_EV_IRQHandler          /* [47] I2C1 事件 */
    .word I2C1_ER_IRQHandler          /* [48] I2C1 错误 */
    .word I2C2_EV_IRQHandler          /* [49] I2C2 事件 */
    .word I2C2_ER_IRQHandler          /* [50] I2C2 错误 */
    .word SPI1_IRQHandler             /* [51] SPI1 */
    .word SPI2_IRQHandler             /* [52] SPI2 */
    .word USART1_IRQHandler           /* [53] USART1 */
    .word USART2_IRQHandler           /* [54] USART2 */
    .word USART3_IRQHandler           /* [55] USART3 */
    .word EXTI15_10_IRQHandler        /* [56] 外部中断 15-10 */
    .word RTCAlarm_IRQHandler         /* [57] RTC 闹钟 */
    .word USBWakeUp_IRQHandler        /* [58] USB 唤醒 */

/* =======================================================================
 * 默认中断处理函数
 * ======================================================================= */

.section .text.Default_Handler, "ax", %progbits
.type Default_Handler, %function
Default_Handler:
    b .                             /* 死循环 */
    .size Default_Handler, . - Default_Handler

/* =======================================================================
 * 弱符号定义 - 允许 C 代码覆盖
 * ======================================================================= */

    .macro IRQ_HANDLER handler_name
    .weak \handler_name
    .thumb_set \handler_name, Default_Handler
    .endm

    /* 系统异常处理 */
    IRQ_HANDLER NMI_Handler
    IRQ_HANDLER HardFault_Handler
    IRQ_HANDLER MemManage_Handler
    IRQ_HANDLER BusFault_Handler
    IRQ_HANDLER UsageFault_Handler
    IRQ_HANDLER SVC_Handler
    IRQ_HANDLER DebugMon_Handler
    IRQ_HANDLER PendSV_Handler
    IRQ_HANDLER SysTick_Handler

    /* 外设中断处理 */
    IRQ_HANDLER WWDG_IRQHandler
    IRQ_HANDLER PVD_IRQHandler
    IRQ_HANDLER TAMPER_IRQHandler
    IRQ_HANDLER RTC_IRQHandler
    IRQ_HANDLER FLASH_IRQHandler
    IRQ_HANDLER RCC_IRQHandler
    IRQ_HANDLER EXTI0_IRQHandler
    IRQ_HANDLER EXTI1_IRQHandler
    IRQ_HANDLER EXTI2_IRQHandler
    IRQ_HANDLER EXTI3_IRQHandler
    IRQ_HANDLER EXTI4_IRQHandler
    IRQ_HANDLER DMA1_Channel1_IRQHandler
    IRQ_HANDLER DMA1_Channel2_IRQHandler
    IRQ_HANDLER DMA1_Channel3_IRQHandler
    IRQ_HANDLER DMA1_Channel4_IRQHandler
    IRQ_HANDLER DMA1_Channel5_IRQHandler
    IRQ_HANDLER DMA1_Channel6_IRQHandler
    IRQ_HANDLER DMA1_Channel7_IRQHandler
    IRQ_HANDLER ADC1_2_IRQHandler
    IRQ_HANDLER USB_HP_CAN1_TX_IRQHandler
    IRQ_HANDLER USB_LP_CAN1_RX0_IRQHandler
    IRQ_HANDLER CAN1_RX1_IRQHandler
    IRQ_HANDLER CAN1_SCE_IRQHandler
    IRQ_HANDLER EXTI9_5_IRQHandler
    IRQ_HANDLER TIM1_BRK_IRQHandler
    IRQ_HANDLER TIM1_UP_IRQHandler
    IRQ_HANDLER TIM1_TRG_COM_IRQHandler
    IRQ_HANDLER TIM1_CC_IRQHandler
    IRQ_HANDLER TIM2_IRQHandler
    IRQ_HANDLER TIM3_IRQHandler
    IRQ_HANDLER TIM4_IRQHandler
    IRQ_HANDLER I2C1_EV_IRQHandler
    IRQ_HANDLER I2C1_ER_IRQHandler
    IRQ_HANDLER I2C2_EV_IRQHandler
    IRQ_HANDLER I2C2_ER_IRQHandler
    IRQ_HANDLER SPI1_IRQHandler
    IRQ_HANDLER SPI2_IRQHandler
    IRQ_HANDLER USART1_IRQHandler
    IRQ_HANDLER USART2_IRQHandler
    IRQ_HANDLER USART3_IRQHandler
    IRQ_HANDLER EXTI15_10_IRQHandler
    IRQ_HANDLER RTCAlarm_IRQHandler
    IRQ_HANDLER USBWakeUp_IRQHandler

/* =======================================================================
 * Reset_Handler - 复位处理函数
 * =======================================================================
 *
 * 功能:
 *   1. 拷贝 .data 段从 Flash 到 RAM
 *   2. 清零 .bss 段
 *   3. 跳转到 main() 函数
 *
 * 寄存器使用:
 *   r0: 目标地址 (RAM)
 *   r1: 结束地址 (边界检查)
 *   r2: 源地址 (Flash) 或 填充值
 *   r3: 临时数据寄存器
 *
 * ======================================================================= */

.section .text.Reset_Handler, "ax", %progbits
.type Reset_Handler, %function
Reset_Handler:

    /* -------------------------------------------------------------------
     * 第 1 步: 拷贝 .data 段 (Flash -> RAM)
     * -------------------------------------------------------------------
     *
     * 内存布局:
     *   Flash: _sidata -> .data 初始值
     *   RAM:   _sdata -> _edata (运行时位置)
     *
     * 拷贝过程:
     *   1. 加载地址: r0=_sdata, r1=_edata, r2=_sidata
     *   2. 循环拷贝: 每次 4 字节, 直到 r0 >= r1
     *
     * ------------------------------------------------------------------- */

    ldr     r0, =_sdata             /* 目标地址: RAM 中 .data 起始 */
    ldr     r1, =_edata             /* 结束地址: RAM 中 .data 结束 */
    ldr     r2, =_sidata            /* 源地址: Flash 中 .data 加载地址 */

    /* 拷贝循环 */
.copy_loop:
    cmp     r0, r1                  /* 比较目标地址和结束地址 */
    bge     .copy_done              /* 如果 r0 >= r1, 拷贝完成 */

    ldr     r3, [r2], #4            /* 从 Flash 加载 4 字节, r2 += 4 */
    str     r3, [r0], #4            /* 存储到 RAM, r0 += 4 */

    b       .copy_loop              /* 继续循环 */

.copy_done:

    /* -------------------------------------------------------------------
     * 第 2 步: 清零 .bss 段
     * -------------------------------------------------------------------
     *
     * 内存布局:
     *   RAM: _sbss -> _ebss (未初始化变量)
     *
     * 清零过程:
     *   1. 加载地址: r0=_sbss, r1=_ebss
     *   2. 循环清零: 每次 4 字节, 直到 r0 >= r1
     *
     * ------------------------------------------------------------------- */

    ldr     r0, =_sbss              /* 起始地址: RAM 中 .bss 开始 */
    ldr     r1, =_ebss              /* 结束地址: RAM 中 .bss 结束 */
    movs    r2, #0                  /* 清零值 */

    /* 清零循环 */
.zero_loop:
    cmp     r0, r1                  /* 比较目标地址和结束地址 */
    bge     .zero_done              /* 如果 r0 >= r1, 清零完成 */

    str     r2, [r0], #4            /* 存储 0 到 RAM, r0 += 4 */

    b       .zero_loop              /* 继续循环 */

.zero_done:

    /* -------------------------------------------------------------------
     * 第 3 步: 跳转到 main() 函数
     * -------------------------------------------------------------------
     *
     * 使用 bl (Branch with Link) 指令调用 main
     * 如果 main 返回, 进入死循环
     *
     * ------------------------------------------------------------------- */

    bl      main                    /* 调用 main() 函数 */

    /* main 不应该返回, 如果返回则进入死循环 */
    b       .                       /* 无限循环 */

    .size Reset_Handler, . - Reset_Handler

/* #######################################################################
 * 文件结束
 * ####################################################################### */

.end
