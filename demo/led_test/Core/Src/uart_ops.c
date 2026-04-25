/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    uart_ops.c
  * @brief   UART ops 句柄模式实现
  ******************************************************************************
  * @attention
  *
  * 基于 STM32F1xx HAL 库的 ops 句柄实现
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "uart_ops.h"
#include <string.h>
#include <stdbool.h>

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief UART init 具体实现
  */
static uart_ops_status_t uart_init_impl(uart_ops_t *uart, const uart_ops_config_t *config);

/**
  * @brief UART read 具体实现
  */
static uart_ops_status_t uart_read_impl(uart_ops_t *uart, uint8_t *data, size_t len, uint32_t timeout);

/**
  * @brief UART write 具体实现
  */
static uart_ops_status_t uart_write_impl(uart_ops_t *uart, const uint8_t *data, size_t len, uint32_t timeout);

/**
  * @brief UART close 具体实现
  */
static uart_ops_status_t uart_close_impl(uart_ops_t *uart);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* Exported functions --------------------------------------------------------*/

/**
  * @brief 默认 ops vtable - 使用 const 修饰，放只读段
  *        导出供用户使用
  */
const uart_ops_vtable_t uart_default_vtable = {
    .init  = uart_init_impl,
    .read  = uart_read_impl,
    .write = uart_write_impl,
    .close = uart_close_impl,
};

/**
  * @brief 初始化 UART 实例
  */
uart_ops_status_t uart_ops_init(uart_ops_t *uart, const uart_ops_config_t *config)
{
    if (!uart || !config || !uart->vtable) {
        return UART_OPS_INVALID_PARAM;
    }

    /* 调用 ops 表中的 init 函数 */
    return uart->vtable->init(uart, config);
}

/**
  * @brief 读取数据
  */
uart_ops_status_t uart_ops_read(uart_ops_t *uart, uint8_t *data, size_t len, uint32_t timeout)
{
    if (!uart || !data || !uart->vtable) {
        return UART_OPS_INVALID_PARAM;
    }

    if (!uart->is_open) {
        return UART_OPS_NOT_INIT;
    }

    return uart->vtable->read(uart, data, len, timeout);
}

/**
  * @brief 写入数据
  */
uart_ops_status_t uart_ops_write(uart_ops_t *uart, const uint8_t *data, size_t len, uint32_t timeout)
{
    if (!uart || !data || !uart->vtable) {
        return UART_OPS_INVALID_PARAM;
    }

    if (!uart->is_open) {
        return UART_OPS_NOT_INIT;
    }

    return uart->vtable->write(uart, data, len, timeout);
}

/**
  * @brief 关闭 UART
  */
uart_ops_status_t uart_ops_close(uart_ops_t *uart)
{
    if (!uart || !uart->vtable) {
        return UART_OPS_INVALID_PARAM;
    }

    uart_ops_status_t ret = uart->vtable->close(uart);
    if (ret == UART_OPS_OK) {
        uart->is_open = false;
    }

    return ret;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief UART init 具体实现
  */
static uart_ops_status_t uart_init_impl(uart_ops_t *uart, const uart_ops_config_t *config)
{
    if (!uart->hal_handle) {
        return UART_OPS_INVALID_PARAM;
    }

    /* 配置 HAL 句柄 */
    uart->hal_handle->Init.BaudRate     = config->baudrate;
    uart->hal_handle->Init.WordLength   = config->word_length;
    uart->hal_handle->Init.StopBits     = config->stop_bits;
    uart->hal_handle->Init.Parity       = config->parity;
    uart->hal_handle->Init.Mode         = config->mode;
    uart->hal_handle->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    uart->hal_handle->Init.OverSampling = UART_OVERSAMPLING_16;

    /* 调用 HAL 初始化 */
    if (HAL_UART_Init(uart->hal_handle) != HAL_OK) {
        return UART_OPS_ERROR;
    }

    uart->is_open = true;
    return UART_OPS_OK;
}

/**
  * @brief UART read 具体实现 (阻塞模式)
  */
static uart_ops_status_t uart_read_impl(uart_ops_t *uart, uint8_t *data, size_t len, uint32_t timeout)
{
    HAL_StatusTypeDef hal_status = HAL_UART_Receive(uart->hal_handle, data, len, timeout);

    switch (hal_status) {
        case HAL_OK:
            return UART_OPS_OK;
        case HAL_TIMEOUT:
            return UART_OPS_TIMEOUT;
        case HAL_BUSY:
            return UART_OPS_BUSY;
        default:
            return UART_OPS_ERROR;
    }
}

/**
  * @brief UART write 具体实现 (阻塞模式)
  */
static uart_ops_status_t uart_write_impl(uart_ops_t *uart, const uint8_t *data, size_t len, uint32_t timeout)
{
    HAL_StatusTypeDef hal_status = HAL_UART_Transmit(uart->hal_handle, (uint8_t *)data, len, timeout);

    switch (hal_status) {
        case HAL_OK:
            return UART_OPS_OK;
        case HAL_TIMEOUT:
            return UART_OPS_TIMEOUT;
        case HAL_BUSY:
            return UART_OPS_BUSY;
        default:
            return UART_OPS_ERROR;
    }
}

/**
  * @brief UART close 具体实现
  */
static uart_ops_status_t uart_close_impl(uart_ops_t *uart)
{
    if (HAL_UART_DeInit(uart->hal_handle) != HAL_OK) {
        return UART_OPS_ERROR;
    }
    return UART_OPS_OK;
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
