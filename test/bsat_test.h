#ifndef BSAT_TEST_H
#define BSAT_TEST_H

#include <ev.h>
#include "ymo_assert.h"


/*-------------------------------------------------------------*
 * Hacky globals:
 *-------------------------------------------------------------*/
#define NO_TEST_TIMEOUTS 5
#define IDX_TIMEOUTS_LAST (NO_TEST_TIMEOUTS-1)
#define MAX_CALLBACKS 15

static size_t no_calls = 0;
static bsat_toq_t* last_toq = NULL;
static bsat_timeout_t* last_item = NULL;

typedef struct my_data {
    char label[12];
} my_data_t;


/*-------------------------------------------------------------*
 * Hacky utility functions:
 *-------------------------------------------------------------*/
static void test_callback(bsat_toq_t* toq, bsat_timeout_t* item)
{
    no_calls++;
    last_toq = toq;
    last_item = item;
    my_data_t* data = item->data;

    ymo_assert(no_calls < MAX_CALLBACKS);
    ev_break(toq->loop, EVBREAK_ALL);
}


/** The BSAT TOQ should always contain ONLY ACTIVE items. */
static size_t bsat_valid_items(bsat_toq_t* toq)
{
    size_t no_items = 0;
    size_t no_active = 0;

    bsat_timeout_t* cur = toq->head;
    while( cur ) {
        no_items++;
        if( bsat_timeout_is_active(cur) ) {
            no_active++;
        }

        my_data_t* data = cur->data;
        cur = cur->next;
    }

    ymo_assert(no_items == no_active);
    return no_items;
}


#endif /* BSAT_TEST_H */
