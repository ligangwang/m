/*
 * array.c
 * 
 * Copyright (C) 2020 Ligang Wang <ligangwangs@gmail.com>
 *
 * dynamic array in C
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "clib/array.h"
#include "clib/string.h"
#include "clib/queue.h"

void queue_init(queue *q, size_t element_size)
{
    array_init(&q->items, element_size);
    q->front = q->back = 0;
}

void queue_push(queue *q, void* element)
{
    array_push(&q->items, element);
    q->back++;
}

void queue_pop(queue *q)
{
    if (q->front < q->back)
        q->front++;
}

void* queue_front(queue *q)
{
    return q->front < q->back ? array_get(&q->items, q->front) : NULL;
}

void* queue_back(queue *q)
{
    return  (q->back > 0) ? array_get(&q->items, q->back - 1) : NULL;
}

void queue_deinit(queue *q)
{
    array_deinit(&q->items);
}
