/*============================================================================*
 * libbsat: timeout management utilities for projects that use libev.
 * Copyright (C) 2021  Andrew T. Canaday

 * This file is part of libbsat.

 * libbsat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * libbsat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with libbsat.  If not, see <https://www.gnu.org/licenses/>.
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

