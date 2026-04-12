/*
 * linker.ld - STM32F103C8T6 链接脚本
 *
 * STM32F103C8T6 内存布局:
 *   Flash: 64KB, 起始地址 0x08000000 (存放代码 + 只读数据)
 *   SRAM:  20KB, 起始地址 0x20000000 (存放变量 + 栈 + 堆)
 *
 * Cortex-M3 启动时, CPU 从向量表读取:
 *   - 地址 0x08000000: 初始栈指针 (MSP)
 *   - 地址 0x08000004: 复位向量 (Reset_Handler 地址)
 */

/* 入口符号, 链接器从这里开始解析 */
ENTRY(Reset_Handler)

/* 定义内存区域 */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* 定义栈大小 (1KB), 放在 RAM 顶部, 向下增长 */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* 段布局 */
SECTIONS
{
    /* 向量表 + 代码 + 只读数据 -> 放入 Flash */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;   /* text 段结束地址 */
    } > FLASH

    /* 已初始化全局/静态变量: 先存 Flash, 启动时拷贝到 RAM */
    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    /* 未初始化全局/静态变量: 直接在 RAM 中清零 */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM
}
