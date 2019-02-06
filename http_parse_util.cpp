/*
 * HTTP解析的通用函数
 */
#include "http_parse_util.h"

/* 从max大小的缓存str中，找到ch1，返回下标(>=0)
 * 返回值：小于0时，-1代表到达缓存结尾，
 *                  -2代表到达行尾没有找到ch1
 */
size_t count_until_chr_in_line(const uint8_t * str, 
        char ch1,
        size_t max) {
    size_t i;
    for (i = 0; i < max; ++i) {
        if (str[i] == ch1)
            return i;
        if (str[i] == '\r'
                || str[i] == '\n') {
            return PARSE_ERR;
        }
    }
    return WAIT_RECV_MORE;
}

size_t discard_chr(uint8_t *&str, const char &ch, size_t &max) {
    if (max < 1)
        return WAIT_RECV_MORE;
    if (str[0] != ch) {
        return PARSE_ERR;
    }
    ++str;
    --max;
    return 0;
}

size_t len_line(uint8_t *&str, size_t &max) {
    size_t i;
    if (max < 2)
        return WAIT_RECV_MORE;
    for (i = 1; i < max; ++i) {
        if (str[i] == '\n' && str[i-1] == '\r') {
            return i - 1;
        }
        if (str[i-1] == '\n' && str[i-1] == '\r') {
            return PARSE_ERR;
        }
    }
    return WAIT_RECV_MORE;
}

size_t discard_line(uint8_t *&str, size_t &max) {
    size_t len = len_line(str, max);
    LOGP(DEBUG, "discard_line %d\n", (int)len);
    if(len < 0 )
        return len;
    str += len + 2;
    max -= len - 2;
    return len;
}
