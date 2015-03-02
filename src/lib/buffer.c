#include "buffer.h"

void bind_buffer(struct buffer *b, void *buff, size_t len)
{
    b->start = buff;
    b->end = b->start + len;
    b->pos = b->last = b->start;
}

