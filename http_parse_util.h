/*
 * HTTP解析的通用函数
 */
#ifndef _HTTP_PARSE_UTIL_H_
#define _HTTP_PARSE_UTIL_H_

#include <cstddef>//for size_t
#include <cstdint>//for uint8_t
#include "logging.h"

#define PARSE_ERR -2
#define WAIT_RECV_MORE -1

size_t count_until_chr_in_line(const uint8_t * str, char ch1, size_t max);
size_t discard_chr(uint8_t *&str, const char &ch, size_t &max);
size_t len_line(uint8_t *&str, size_t &max);
size_t discard_line(uint8_t *&str, size_t &max);

#endif
