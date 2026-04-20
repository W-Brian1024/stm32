# #######################################################################
# Makefile - STM32F103C8T6 启动流程分析实验
# #######################################################################
#
# 用法:
#   make              # 构建所有输出文件
#   make clean        # 清理构建目录
#   make flash        # 烧录到板子 (需要 st-flash)
#   make debug        # 显示调试信息
#   make analyze      # 分析内存布局和符号
#
# 工具链要求:
#   arm-none-eabi-gcc (可通过 GCC_PATH 指定目录)
#   st-flash (用于烧录)
#
# #######################################################################

# #######################################################################
# 项目配置
# #######################################################################

TARGET          := startup_analysis
BUILD_DIR       := build

# #######################################################################
# 工具链配置
# #######################################################################

PREFIX          := arm-none-eabi-

ifdef GCC_PATH
    CC          := $(GCC_PATH)/$(PREFIX)gcc
    AS          := $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
    CP          := $(GCC_PATH)/$(PREFIX)objcopy
    SZ          := $(GCC_PATH)/$(PREFIX)size
    OD          := $(GCC_PATH)/$(PREFIX)objdump
    NM          := $(GCC_PATH)/$(PREFIX)nm
else
    CC          := $(PREFIX)gcc
    AS          := $(PREFIX)gcc -x assembler-with-cpp
    CP          := $(PREFIX)objcopy
    SZ          := $(PREFIX)size
    OD          := $(PREFIX)objdump
    NM          := $(PREFIX)nm
endif

HEX             = $(CP) -O ihex
BIN             = $(CP) -O binary -S

# #######################################################################
# 源文件配置
# #######################################################################

# C 源文件
C_SOURCES       := main.c

# 汇编源文件
ASM_SOURCES     := startup.s

# #######################################################################
# 编译器配置
# #######################################################################

# CPU 和 架构
CPU             := -mcpu=cortex-m3
MCU             := $(CPU) -mthumb

# 调试和优化
DEBUG           := 1
OPT             := -Og

# #######################################################################
# 编译选项
# #######################################################################

# C 定义
C_DEFS          :=

# 头文件路径
C_INCLUDES      :=

# 汇编定义
AS_DEFS         :=

# 编译选项
CFLAGS          := $(MCU)
CFLAGS          += $(C_DEFS)
CFLAGS          += $(C_INCLUDES)
CFLAGS          += $(OPT)
CFLAGS          += -Wall                    # 所有警告
CFLAGS          += -Wextra                  # 额外警告
CFLAGS          += -Wstrict-prototypes      # 严格原型检查
CFLAGS          += -Wmissing-prototypes     # 缺少原型警告
CFLAGS          += -fdata-sections          # 独立数据段
# CFLAGS          += -ffunction-sections      # 独立函数段 (禁用：导致函数被删除)
CFLAGS          += -std=c99                 # C99 标准

# 调试选项
ifeq ($(DEBUG), 1)
    CFLAGS      += -g3                     # 最大调试信息
    CFLAGS      += -gdwarf-4               # DWARF 4 格式
endif

# 依赖文件生成
CFLAGS          += -MMD -MP

# 汇编选项
ASFLAGS         := $(MCU)
ASFLAGS         += $(AS_DEFS)
ASFLAGS         += $(OPT)
ASFLAGS         += -Wall
ASFLAGS         += -fdata-sections
# ASFLAGS         += -ffunction-sections  # 禁用：导致函数被删除

# #######################################################################
# 链接器配置
# #######################################################################

# 链接脚本
LDSCRIPT        := linker.ld

# 库文件
LIBS            := -lgcc

# 链接选项
LDFLAGS         := $(MCU)
LDFLAGS         += -nostdlib               # 不使用标准库
LDFLAGS         += -T$(LDSCRIPT)           # 指定链接脚本
LDFLAGS         += $(LIBS)
LDFLAGS         += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref
# LDFLAGS         += -Wl,--gc-sections       # 删除未使用段 (禁用：导致函数被删除)
LDFLAGS         += -Wl,--print-memory-usage

# #######################################################################
# 构建目标
# #######################################################################

# 目标文件
OBJECTS         := $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
OBJECTS         += $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES:.s=.o))

# 源文件路径
vpath %.c .
vpath %.s .

# 伪目标
.PHONY: all clean flash debug analyze sections symbols map size

# 默认目标
all: $(BUILD_DIR)/$(TARGET).elf \
     $(BUILD_DIR)/$(TARGET).hex \
     $(BUILD_DIR)/$(TARGET).bin
	@echo ""
	@echo "=== 构建完成 ==="
	@$(SZ) $(BUILD_DIR)/$(TARGET).elf

# #######################################################################
# 编译规则
# #######################################################################

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "[CC] $<"
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "[AS] $<"
	$(AS) -c $(ASFLAGS) $< -o $@

# #######################################################################
# 链接规则
# #######################################################################

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(LDSCRIPT) Makefile
	@echo "[LD] $@"
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "[SIZE] $@"
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	@echo "[HEX] $@"
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@echo "[BIN] $@"
	$(BIN) $< $@

# #######################################################################
# 目录创建
# #######################################################################

$(BUILD_DIR):
	mkdir -p $@

# #######################################################################
# 清理规则
# #######################################################################

clean:
	@echo "[CLEAN] $(BUILD_DIR)"
	rm -rf $(BUILD_DIR)

# #######################################################################
# 烧录规则
# #######################################################################

flash: $(BUILD_DIR)/$(TARGET).bin
	@echo "[FLASH] $<"
	st-flash --reset write $< 0x08000000

# #######################################################################
# 调试和分析规则
# #######################################################################

# 显示段表信息
sections: $(BUILD_DIR)/$(TARGET).elf
	@echo ""
	@echo "=== 段表信息 ==="
	$(OD) -h $<

# 显示符号表
symbols: $(BUILD_DIR)/$(TARGET).elf
	@echo ""
	@echo "=== 符号表 (按地址排序) ==="
	$(NM) -n $<

# 显示 map 文件
map: $(BUILD_DIR)/$(TARGET).map
	@echo ""
	@echo "=== 链接 Map 文件 ==="
	less $<

# 显示内存使用
size: $(BUILD_DIR)/$(TARGET).elf
	@echo ""
	@echo "=== 内存使用统计 ==="
	$(SZ) $<

# 显示反汇编
disasm: $(BUILD_DIR)/$(TARGET).elf
	@echo ""
	@echo "=== 反汇编输出 ==="
	$(OD) -d $< | less

# 综合分析
analyze: sections symbols
	@echo ""
	@echo "=== 内存使用统计 ==="
	$(SZ) $(BUILD_DIR)/$(TARGET).elf
	@echo ""
	@echo "=== 关键链接符号 ==="
	$(NM) $(BUILD_DIR)/$(TARGET).elf | grep -E '_e(stack|text|data|bss)|_s(idata|data|bss)'

# 调试信息
debug:
	@echo "=== 配置信息 ==="
	@echo "TARGET:      $(TARGET)"
	@echo "BUILD_DIR:   $(BUILD_DIR)"
	@echo "CC:          $(CC)"
	@echo ""
	@echo "=== 源文件 ==="
	@echo "C_SOURCES:   $(C_SOURCES)"
	@echo "ASM_SOURCES: $(ASM_SOURCES)"
	@echo ""
	@echo "=== 编译选项 ==="
	@echo "CFLAGS:      $(CFLAGS)"
	@echo ""
	@echo "=== 链接选项 ==="
	@echo "LDFLAGS:     $(LDFLAGS)"

# #######################################################################
# 依赖文件包含
# #######################################################################

-include $(wildcard $(BUILD_DIR)/*.d)
