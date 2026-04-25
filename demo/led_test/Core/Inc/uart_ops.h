/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    uart_ops.h
  * @brief   UART ops 句柄模式接口定义
  ******************************************************************************
  * @attention
  *
  * 基于 STM32F1xx HAL 库的 ops 句柄抽象层
  * 提供统一的 UART 操作接口
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_OPS_H__
#define __UART_OPS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief UART 状态码定义
  */
typedef enum {
    UART_OPS_OK              = 0,    /* 操作成功 */
    UART_OPS_ERROR           = -1,   /* 通用错误 */
    UART_OPS_BUSY            = -2,   /* 忙碌状态 */
    UART_OPS_TIMEOUT         = -3,   /* 超时 */
    UART_OPS_INVALID_PARAM   = -4,   /* 无效参数 */
    UART_OPS_NOT_INIT        = -5,   /* 未初始化 */
} uart_ops_status_t;

/**
  * @brief UART 配置结构体
  */
typedef struct {
    uint32_t baudrate;              /* 波特率 (如 115200) */
    uint32_t word_length;           /* 字长: UART_WORDLENGTH_8B/9B */
    uint32_t stop_bits;             /* 停止位: UART_STOPBITS_1/2 */
    uint32_t parity;                /* 校验位: UART_PARITY_NONE/EVEN/ODD */
    uint32_t mode;                  /* 模式: UART_MODE_TX/RX/TX_RX */
} uart_ops_config_t;

/* 前向声明 */
typedef struct uart_ops uart_ops_t;

/**
  * @brief UART ops 句柄 - 函数指针表
  */
typedef struct {
    uart_ops_status_t (*init)   (uart_ops_t *uart, const uart_ops_config_t *config);  /* 初始化 */
    uart_ops_status_t (*read)   (uart_ops_t *uart, uint8_t *data, size_t len, uint32_t timeout);  /* 读取 */
    uart_ops_status_t (*write)  (uart_ops_t *uart, const uint8_t *data, size_t len, uint32_t timeout);  /* 写入 */
    uart_ops_status_t (*close)  (uart_ops_t *uart);  /* 关闭/去初始化 */
} uart_ops_vtable_t;

/**
  * @brief UART 实例结构体
  */
struct uart_ops {
    const uart_ops_vtable_t *vtable;    /* ops 虚表，放只读段 */
    UART_HandleTypeDef        *hal_handle;  /* HAL 句柄 */
    volatile bool             is_open;     /* 打开状态 */
    void                     *rx_buffer;   /* 接收缓冲区 */
    size_t                    rx_buf_size; /* 接收缓冲区大小 */
};

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/

/**
  * @brief 读取单字节宏
  */
#define UART_OPS_READ_BYTE(uart, byte, timeout) \
    uart_ops_read((uart), (byte), 1, (timeout))

/**
  * @brief 写入单字节宏
  */
#define UART_OPS_WRITE_BYTE(uart, byte, timeout) \
    uart_ops_write((uart), &(byte), 1, (timeout))

/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief 初始化 UART 实例
  * @param uart UART 实例指针
  * @param config 配置参数
  * @retval 操作状态
  */
uart_ops_status_t uart_ops_init(uart_ops_t *uart, const uart_ops_config_t *config);

/**
  * @brief 读取数据
  * @param uart UART 实例指针
  * @param data 接收缓冲区
  * @param len 要读取的字节数
  * @param timeout 超时时间 (ms)
  * @retval 操作状态
  */
uart_ops_status_t uart_ops_read(uart_ops_t *uart, uint8_t *data, size_t len, uint32_t timeout);

/**
  * @brief 写入数据
  * @param uart UART 实例指针
  * @param data 发送数据缓冲区
  * @param len 要发送的字节数
  * @param timeout 超时时间 (ms)
  * @retval 操作状态
  */
uart_ops_status_t uart_ops_write(uart_ops_t *uart, const uint8_t *data, size_t len, uint32_t timeout);

/**
  * @brief 关闭 UART
  * @param uart UART 实例指针
  * @retval 操作状态
  */
uart_ops_status_t uart_ops_close(uart_ops_t *uart);

/**
  * @brief 默认 ops vtable (导出供用户使用)
  */
extern const uart_ops_vtable_t uart_default_vtable;

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif
#endif /* __UART_OPS_H__ */
