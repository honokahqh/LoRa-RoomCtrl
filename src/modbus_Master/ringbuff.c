#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ringbuff.h"

/* 数据为ascii类型的用r/w,为HEX的用r_hex/w_hex,不能混用 */
CircularBuffer MBS_ringbuf,Z9_ringbuf;

void ringbuff_init(CircularBuffer* cb) {
    memset(cb->messages, 0, sizeof(cb->messages));
    cb->read_pos = 0;
    cb->write_pos = 0;
    cb->count = 0;
}

int cb_is_empty(CircularBuffer* cb) {
    return cb->count == 0;
}

int cb_is_full(CircularBuffer* cb) {
    return cb->count == BUFFER_SIZE;
}

int cb_size(CircularBuffer* cb) {
    return cb->count;
}
int ringbuff_write(CircularBuffer* cb, const char* message) {
    if (cb_is_full(cb)) {
        return 0; // 返回0表示写入失败
    }

    strncpy(cb->messages[cb->write_pos], message, MESSAGE_SIZE - 1);
    cb->write_pos = (cb->write_pos + 1) % BUFFER_SIZE;
    cb->count++;
    return 1; // 返回1表示写入成功
}

int ringbuff_read(CircularBuffer* cb, char* message) {
    if (cb_is_empty(cb)) {
        return 0; // 返回0表示读取失败
    }

    strncpy(message, cb->messages[cb->read_pos], MESSAGE_SIZE - 1);
    cb->read_pos = (cb->read_pos + 1) % BUFFER_SIZE;
    cb->count--;
    return 1; // 返回1表示读取成功
}

int ringbuff_write_Hex(CircularBuffer* cb, uint8_t *data,uint8_t data_len) {
    if (cb_is_full(cb)) {
        return 0; // 返回0表示写入失败
    }
    memcpy(cb->messages[cb->write_pos], data, data_len);
    cb->msg_len[cb->write_pos] = data_len;
    cb->write_pos = (cb->write_pos + 1) % BUFFER_SIZE;
    cb->count++;
    return 1; // 返回1表示写入成功
}

int ringbuff_read_Hex(CircularBuffer* cb, uint8_t *data,uint8_t *data_len) {
    if (cb_is_empty(cb)) {
        return 0; // 返回0表示读取失败
    }
    memcpy(data, cb->messages[cb->read_pos], cb->msg_len[cb->read_pos]);
    *data_len = cb->msg_len[cb->read_pos];
    cb->read_pos = (cb->read_pos + 1) % BUFFER_SIZE;
    cb->count--;
    return 1; // 返回1表示读取成功
}
