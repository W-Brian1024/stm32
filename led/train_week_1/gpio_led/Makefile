# Makefile - STM32F103C8T6 裸机 gpio_led (无 HAL)
#
# 用法: 在本目录执行
#   make              # 生成 build/gpio_led.{elf,hex,bin}
#   make clean
#   make flash        # 需要 st-flash，将 .bin 写到 0x08000000
#
# 交叉工具链: arm-none-eabi-gcc（可通过 GCC_PATH 指定目录）

TARGET    = gpio_led
BUILD_DIR = build

PREFIX = arm-none-eabi-
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif

HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# 源文件（本目录）
#######################################
C_SOURCES   = main.c
ASM_SOURCES = startup.s

#######################################
# 编译选项
#######################################
DEBUG = 1
OPT   = -Og

CPU = -mcpu=cortex-m3
MCU = $(CPU) -mthumb

C_DEFS    =
C_INCLUDES =

ASFLAGS = $(MCU) $(AS_DEFS) $(OPT) -Wall -fdata-sections -ffunction-sections
CFLAGS  = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# 链接
#######################################
LDSCRIPT = linker.ld
# 未使用 libc 时用 -nostdlib -lgcc；若以后用到 printf 等，可改为:
#   LIBS = -lc -lm -lnosys
#   LDFLAGS 前缀加上 -specs=nano.specs -specs=nosys.specs 并去掉 -nostdlib
LIBS     = -lgcc
LDFLAGS  = $(MCU) -nostdlib -T$(LDSCRIPT) $(LIBS) \
           -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

#######################################
# 构建规则
#######################################
OBJECTS  = $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES:.s=.o))

vpath %.c .
vpath %.s .

.PHONY: all clean flash

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(LDSCRIPT) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

# 需要安装 stlink 工具包中的 st-flash
flash: $(BUILD_DIR)/$(TARGET).bin
	st-flash --reset write $< 0x08000000

-include $(wildcard $(BUILD_DIR)/*.d)
