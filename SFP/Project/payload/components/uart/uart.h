#pragma once

#include <stdint.h>

/*
 * @brief Initialize UART
 */
void uart_init(void);


/*
 * Read a character from UART
 */
void get_char(bool echo);

/*
 * Read a string from UART
 */
void get_string(char *data,bool echo);