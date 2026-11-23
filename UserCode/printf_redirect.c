#include <stdio.h>

#include "usart.h"

int fputc(int ch, FILE *stream)
{
  uint8_t byte = (uint8_t)ch;

  (void)stream;
  HAL_UART_Transmit(&huart2 ,&byte, 1, HAL_MAX_DELAY);
  return ch;
}
