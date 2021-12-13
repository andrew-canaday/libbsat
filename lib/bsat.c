/*============================================================================*
 * libbsat: timeout management utilities for projects that use libev.
 * Copyright (c) 2021 Andrew T. Canaday
 *
 * This file is part of libbsat, which is licensed under the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *----------------------------------------------------------------------------*/

#include <stdlib.h>

#include "bsat_config.h"
#include "bsat.h"


/*--------------------------------------------------
 * Macros and utils:
 *--------------------------------------------------*/
#if EV_MULTIPLICITY
# define TOQ_LOOP toq->loop
# define TOQ_LOOP_ toq->loop,
#else
# define TOQ_LOOP
# define TOQ_LOOP_
#endif /* EV_MULTIPLICITY */


/*--------------------------------------------------
 * Globals:
 *--------------------------------------------------*/
const char* BSAT_VERSION_STR = PACKAGE_VERSION;


/*--------------------------------------------------
 * Prototypes:
 *--------------------------------------------------*/
static void bsat_toq_dispatch(EV_P_ ev_timer* w, int revents);
static void bsat_toq_schedule_next(bsat_toq_t* toq);


/*--------------------------------------------------
 * BSAT Timeout Queue Functions:
 *--------------------------------------------------*/
void bsat_toq_init(
        EV_P_
        bsat_toq_t* toq,
        bsat_callback_t cb,
        ev_tstamp after)
{
    toq->cb = cb;
    toq->head = toq->tail = NULL;
    toq->data = NULL;

#if EV_MULTIPLICITY
    toq->loop = EV_A;
#endif /* EV_MULTIPLICITY */

    ev_timer_init(
        &(toq->timer), bsat_toq_dispatch, after, 0.0 );
    toq->timer.data = toq;
    toq->after = after;
}


static void bsat_toq_dispatch(EV_P_ ev_timer* w, int revents)
{
    bsat_toq_t* toq = w->data;
    ev_tstamp threshold = ev_now(EV_A) - toq->after;

    while( toq->head ) {
        bsat_timeout_t* current = toq->head;
        if( current->tstamp <= threshold ) {
            bsat_timeout_stop(toq, current);
            toq->cb(toq, current);
        } else {
            break;
        }
    }

    bsat_toq_schedule_next(toq);
}


static void bsat_toq_schedule_next(bsat_toq_t* toq)
{
    ev_timer_stop(TOQ_LOOP_ &(toq->timer));
    bsat_timeout_t* next_item = toq->head;
    if( next_item ) {
        ev_tstamp delta_next =
            (next_item->tstamp + toq->after) - ev_now(TOQ_LOOP);
        ev_timer_set(&(toq->timer), delta_next, 0.0);
        ev_timer_start(TOQ_LOOP_ &(toq->timer));
    }
}

void bsat_toq_stop(bsat_toq_t* toq)
{
    ev_timer_stop(TOQ_LOOP_ &(toq->timer));
}


void bsat_toq_clear(bsat_toq_t* toq)
{
    while( toq->head ) {
        bsat_timeout_t* current = toq->head;
        bsat_timeout_stop(toq, current);
    }

    ev_timer_stop(TOQ_LOOP_ &(toq->timer));
}


void bsat_toq_invoke_pending(bsat_toq_t* toq)
{
    while( toq->head ) {
        bsat_timeout_t* current = toq->head;
        bsat_timeout_stop(toq, current);
        toq->cb(toq, current);
    }

    bsat_toq_clear(toq);
}


/*--------------------------------------------------
 * BSAT Timeout Functions:
 *--------------------------------------------------*/
void bsat_timeout_init(bsat_timeout_t* timeout)
{
    timeout->tstamp = (ev_tstamp)-1.0;
    timeout->prev = timeout->next = NULL;
    timeout->data = NULL;
}


void bsat_timeout_start(bsat_toq_t* toq, bsat_timeout_t* item)
{
    /* Don't do anything if it's already started: */
    if( item->tstamp > 0.0 ) {
        return;
    }

    item->tstamp = ev_now(TOQ_LOOP);
    if( toq->tail ) {
        item->prev = toq->tail;
        toq->tail->next = item;
        toq->tail = item;
    } else {
        toq->head = toq->tail = item;
        bsat_toq_schedule_next(toq);
    }
    return;
}


void bsat_timeout_reset(bsat_toq_t* toq, bsat_timeout_t* item)
{
    bsat_timeout_stop(toq, item);
    bsat_timeout_start(toq, item);
    return;
}


void bsat_timeout_stop(bsat_toq_t* toq, bsat_timeout_t* item)
{
    if( item->tstamp < 0.0 ) {
        return;
    }

    item->tstamp = (ev_tstamp)-1.0;
    bsat_timeout_t* next = item->next;
    bsat_timeout_t* prev = item->prev;

    if( prev ) {
        prev->next = next;
    }
    if( next ) {
        next->prev = prev;
    }

    if( toq->head == item ) {
        toq->head = next;
    }
    if( toq->tail == item ) {
        toq->tail = prev;
    }

    item->next = item->prev = NULL;
    return;
}


int bsat_timeout_is_active(bsat_timeout_t* item)
{
    return item->tstamp > 0.0;
}

