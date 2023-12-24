#ifndef  _RINGBUFF_H_
#define  _RINGBUFF_H_

#include "APP.h"


#define BUFFER_SIZE 10
#define MESSAGE_SIZE 64
typedef struct {
    char messages[BUFFER_SIZE][MESSAGE_SIZE];
    int msg_len[BUFFER_SIZE];
    int read_pos;
    int write_pos;
    int count;
} CircularBuffer;
extern CircularBuffer MBS_ringbuf,Z9_ringbuf;

int cb_size(CircularBuffer* cb);
void ringbuff_init(CircularBuffer* cb);
int ringbuff_write(CircularBuffer* cb, const char* message);
int ringbuff_read(CircularBuffer* cb, char* message);
int ringbuff_write_Hex(CircularBuffer* cb, uint8_t *data,uint8_t data_len);
int ringbuff_read_Hex(CircularBuffer* cb, uint8_t *data,uint8_t *data_len);

#endif