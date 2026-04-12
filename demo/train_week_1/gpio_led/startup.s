/*
 * startup.s - STM32F103C8T6 启动汇编
 *
 * 职责:
 *   1. 在 Flash 最开头放置向量表 (.isr_vector 段)
 *   2. Reset_Handler: 拷贝 .data, 清零 .bss, 跳转 main
 */

/*
 * 语法说明:
 *   .syntax unified    - 使用统一 ARM/Thumb 语法
 *   .cpu cortex-m3     - 指定目标 CPU
 *   .thumb             - 使用 Thumb 指令集 (Cortex-M3 只支持 Thumb)
 */
.syntax unified
.cpu cortex-m3
.thumb

/* ============================================================
 * 全局符号声明 (C 代码中可以使用这些符号)
 * ============================================================ */
.global  Reset_Handler

/* ============================================================
 * 向量表 - 必须放在 Flash 最开头
 *
 * Cortex-M3 向量表结构:
 *   [0]  初始栈指针 (MSP), 不是函数地址, 是一个数值
 *   [1]  Reset_Handler
 *   [2]  NMI_Handler
 *   [3]  HardFault_Handler
 *   [4]  MemManage_Handler
 *   [5]  BusFault_Handler
 *   [6]  UsageFault_Handler
 *   [7-10] 保留
 *   [11] SVCall_Handler
 *   [12] DebugMon_Handler
 *   [13] 保留
 *   [14] PendSV_Handler
 *   [15] SysTick_Handler
 *   [16+] 外设中断 (STM32F103C8T6 共 43 个)
 * ============================================================ */
.section .isr_vector, "a", %progbits
.type    vector_table, %object
vector_table:
    .word   _estack            /* [0]  栈顶, 从 linker.ld 导入 */
    .word   Reset_Handler      /* [1]  复位 */
    .word   NMI_Handler        /* [2]  不可屏蔽中断 */
    .word   HardFault_Handler  /* [3]  硬件错误 */
    .word   MemManage_Handler  /* [4]  内存管理错误 */
    .word   BusFault_Handler   /* [5]  总线错误 */
    .word   UsageFault_Handler /* [6]  用法错误 */
    .word   0                  /* [7]  保留 */
    .word   0                  /* [8]  保留 */
    .word   0                  /* [9]  保留 */
    .word   0                  /* [10] 保留 */
    .word   SVC_Handler        /* [11] 系统服务调用 */
    .word   DebugMon_Handler   /* [12] 调试监控 */
    .word   0                  /* [13] 保留 */
    .word   PendSV_Handler     /* [14] PendSV */
    .word   SysTick_Handler    /* [15] SysTick 定时器 */
    /* 外设中断 [16-58], STM32F103C8T6 中等密度设备 */
    .word   WWDG_IRQHandler          /* Window Watchdog */
    .word   PVD_IRQHandler           /* PVD through EXTI Line detect */
    .word   TAMPER_IRQHandler        /* Tamper */
    .word   RTC_IRQHandler           /* RTC */
    .word   FLASH_IRQHandler         /* Flash */
    .word   RCC_IRQHandler           /* RCC */
    .word   EXTI0_IRQHandler         /* EXTI Line 0 */
    .word   EXTI1_IRQHandler         /* EXTI Line 1 */
    .word   EXTI2_IRQHandler         /* EXTI Line 2 */
    .word   EXTI3_IRQHandler         /* EXTI Line 3 */
    .word   EXTI4_IRQHandler         /* EXTI Line 4 */
    .word   DMA1_Channel1_IRQHandler /* DMA1 Channel 1 */
    .word   DMA1_Channel2_IRQHandler /* DMA1 Channel 2 */
    .word   DMA1_Channel3_IRQHandler /* DMA1 Channel 3 */
    .word   DMA1_Channel4_IRQHandler /* DMA1 Channel 4 */
    .word   DMA1_Channel5_IRQHandler /* DMA1 Channel 5 */
    .word   DMA1_Channel6_IRQHandler /* DMA1 Channel 6 */
    .word   DMA1_Channel7_IRQHandler /* DMA1 Channel 7 */
    .word   ADC1_2_IRQHandler        /* ADC1 & ADC2 */
    .word   USB_HP_CAN1_TX_IRQHandler/* USB High Priority / CAN1 TX */
    .word   USB_LP_CAN1_RX0_IRQHandler /* USB Low Priority / CAN1 RX0 */
    .word   CAN1_RX1_IRQHandler      /* CAN1 RX1 */
    .word   CAN1_SCE_IRQHandler      /* CAN1 SCE */
    .word   EXTI9_5_IRQHandler       /* EXTI Line 9..5 */
    .word   TIM1_BRK_IRQHandler      /* TIM1 Break */
    .word   TIM1_UP_IRQHandler       /* TIM1 Update */
    .word   TIM1_TRG_COM_IRQHandler  /* TIM1 Trigger and Commutation */
    .word   TIM1_CC_IRQHandler       /* TIM1 Capture Compare */
    .word   TIM2_IRQHandler          /* TIM2 */
    .word   TIM3_IRQHandler          /* TIM3 */
    .word   TIM4_IRQHandler          /* TIM4 */
    .word   I2C1_EV_IRQHandler       /* I2C1 Event */
    .word   I2C1_ER_IRQHandler       /* I2C1 Error */
    .word   I2C2_EV_IRQHandler       /* I2C2 Event */
    .word   I2C2_ER_IRQHandler       /* I2C2 Error */
    .word   SPI1_IRQHandler          /* SPI1 */
    .word   SPI2_IRQHandler          /* SPI2 */
    .word   USART1_IRQHandler        /* USART1 */
    .word   USART2_IRQHandler        /* USART2 */
    .word   USART3_IRQHandler        /* USART3 */
    .word   EXTI15_10_IRQHandler     /* EXTI Line 15..10 */
    .word   RTCAlarm_IRQHandler      /* RTC Alarm through EXTI Line */
    .word   USBWakeUp_IRQHandler     /* USB Wakeup from suspend */
    .size   vector_table, . - vector_table

/* ============================================================
 * 默认中断处理函数 - 所有未实现的中断统一进死循环
 * ============================================================ */
.section .text
.type Default_Handler, %function
Default_Handler:
    b   Default_Handler          /* 死循环 */
    .size Default_Handler, . - Default_Handler

/*
 * 宏: 把所有 Handler 别名指向 Default_Handler
 * 这样任何未处理的中断都会进入死循环, 方便调试
 */
.macro IRQ handler
    .weak   \handler
    .thumb_set \handler, Default_Handler
.endm

    IRQ NMI_Handler
    IRQ HardFault_Handler
    IRQ MemManage_Handler
    IRQ BusFault_Handler
    IRQ UsageFault_Handler
    IRQ SVC_Handler
    IRQ DebugMon_Handler
    IRQ PendSV_Handler
    IRQ SysTick_Handler
    IRQ WWDG_IRQHandler
    IRQ PVD_IRQHandler
    IRQ TAMPER_IRQHandler
    IRQ RTC_IRQHandler
    IRQ FLASH_IRQHandler
    IRQ RCC_IRQHandler
    IRQ EXTI0_IRQHandler
    IRQ EXTI1_IRQHandler
    IRQ EXTI2_IRQHandler
    IRQ EXTI3_IRQHandler
    IRQ EXTI4_IRQHandler
    IRQ DMA1_Channel1_IRQHandler
    IRQ DMA1_Channel2_IRQHandler
    IRQ DMA1_Channel3_IRQHandler
    IRQ DMA1_Channel4_IRQHandler
    IRQ DMA1_Channel5_IRQHandler
    IRQ DMA1_Channel6_IRQHandler
    IRQ DMA1_Channel7_IRQHandler
    IRQ ADC1_2_IRQHandler
    IRQ USB_HP_CAN1_TX_IRQHandler
    IRQ USB_LP_CAN1_RX0_IRQHandler
    IRQ CAN1_RX1_IRQHandler
    IRQ CAN1_SCE_IRQHandler
    IRQ EXTI9_5_IRQHandler
    IRQ TIM1_BRK_IRQHandler
    IRQ TIM1_UP_IRQHandler
    IRQ TIM1_TRG_COM_IRQHandler
    IRQ TIM1_CC_IRQHandler
    IRQ TIM2_IRQHandler
    IRQ TIM3_IRQHandler
    IRQ TIM4_IRQHandler
    IRQ I2C1_EV_IRQHandler
    IRQ I2C1_ER_IRQHandler
    IRQ I2C2_EV_IRQHandler
    IRQ I2C2_ER_IRQHandler
    IRQ SPI1_IRQHandler
    IRQ SPI2_IRQHandler
    IRQ USART1_IRQHandler
    IRQ USART2_IRQHandler
    IRQ USART3_IRQHandler
    IRQ EXTI15_10_IRQHandler
    IRQ RTCAlarm_IRQHandler
    IRQ USBWakeUp_IRQHandler

/* ============================================================
 * Reset_Handler - 上电后第一个执行的代码
 *
 * 执行流程:
 *   1. 拷贝 .data 段: Flash -> RAM
 *   2. 清零 .bss 段
 *   3. 跳转到 main()
 * ============================================================ */
.type Reset_Handler, %function
Reset_Handler:

    /* --- 第 1 步: 拷贝 .data 从 Flash 到 RAM --- */
    /*
     * LDR = Load Register, 从标号地址加载一个 32 位值到寄存器
     * R0 = _sdata  (RAM 中 .data 起始地址, 拷贝目标)
     * R1 = _edata  (RAM 中 .data 结束地址, 拷贝边界)
     * R2 = _sidata (Flash 中 .data 起始地址, 拷贝源)
     */
    ldr   r0, =_sdata
    ldr   r1, =_edata
    ldr   r2, =_sidata

copy_data_loop:
    cmp   r0, r1           /* 比较: 目标地址 >= 结束地址? */
    bge   zero_bss         /* 是的话, 拷贝完成, 跳去清零 .bss */

    ldr   r3, [r2]         /* 从 Flash 读 4 字节到 r3 */
    str   r3, [r0]         /* 把 r3 写入 RAM */

    adds  r0, r0, #4       /* 目标地址 +4 (移到下一个 word) */
    adds  r2, r2, #4       /* 源地址 +4 */
    b     copy_data_loop   /* 继续循环 */

    /* --- 第 2 步: 清零 .bss 段 --- */
    /*
     * R0 = _sbss (.bss 起始地址)
     * R1 = _ebss (.bss 结束地址)
     */
zero_bss:
    ldr   r0, =_sbss
    ldr   r1, =_ebss
    mov   r2, #0           /* r2 = 0, 用来填充 */

zero_bss_loop:
    cmp   r0, r1           /* 比较: 目标地址 >= 结束地址? */
    bge   call_main        /* 是的话, 清零完成, 跳去调用 main */

    str   r2, [r0]         /* 把 0 写入 RAM */
    adds  r0, r0, #4       /* 地址 +4 */
    b     zero_bss_loop    /* 继续循环 */

    /* --- 第 3 步: 跳转到 C 的 main 函数 --- */
call_main:
    bl    main             /* Branch with Link, 调用 main() */

    /* 如果 main 返回了 (正常不会), 进死循环 */
    b     .
    .size Reset_Handler, . - Reset_Handler

/* ============================================================
 * 必须提供这几个符号, 否则链接时用了 -nostdlib 会报错
 * ============================================================ */
.end
