#include "bsat.h"
#include "bsat_test.h"


/*-------------------------------------------------------------*
 * Tests:
 *-------------------------------------------------------------*/
void test_bsat_toq(void)
{
    struct ev_loop* loop = ev_default_loop(0);

    /* Create a TOQ and ensure it's valid: */
    bsat_toq_t toq;
    bsat_toq_init(EV_A_ &toq, test_callback, 0.01);
    ymo_assert(bsat_valid_items(&toq) == 0);

    /* Add a timeout, verify still valid: */
    bsat_timeout_t timeout;
    bsat_timeout_init(&timeout);
    bsat_timeout_start(&toq, &timeout);
    ymo_assert(bsat_valid_items(&toq) == 1);

    /* Reset the timeout, verify we still just have one: */
    bsat_timeout_reset(&toq, &timeout);
    ymo_assert(bsat_valid_items(&toq) == 1);

    /* Stop it, verify we're back to zero items: */
    bsat_timeout_stop(&toq, &timeout);
    ymo_assert(bsat_valid_items(&toq) == 0);

    bsat_toq_stop(&toq);
    /* Cool! */
    return;
}


void test_bsat_order(void)
{
    struct ev_loop* loop = ev_default_loop(0);

    /* Create a TOQ and ensure it's valid: */
    bsat_toq_t toq;
    bsat_toq_init(EV_A_ &toq, test_callback, 0.01);
    ymo_assert(bsat_valid_items(&toq) == 0);

    /* Add some timeouts. */
    my_data_t data[NO_TEST_TIMEOUTS];
    bsat_timeout_t timeouts[NO_TEST_TIMEOUTS];

    for( size_t i=0; i<NO_TEST_TIMEOUTS; i++ ) {
        sprintf(data[i].label, "timeouts[%zu]", i);

        bsat_timeout_init(&timeouts[i]);
        timeouts[i].data = &data[i];
        bsat_timeout_start(&toq, &timeouts[i]);
    }
    ymo_assert(bsat_valid_items(&toq) == NO_TEST_TIMEOUTS);

    /* Check that the order reflects the order added: */
    ymo_assert(toq.head == &timeouts[0]);
    ymo_assert(toq.tail == &timeouts[IDX_TIMEOUTS_LAST]);

    /* Reset the item in the front and confirm it moves to the back: */
    bsat_timeout_reset(&toq, &timeouts[0]);
    ymo_assert(toq.head == &timeouts[1]);
    ymo_assert(toq.tail == &timeouts[0]);

    /* Stop all but one and confirm we only have the last one remaining: */
    for( size_t i=0; i<IDX_TIMEOUTS_LAST; i++ ) {
        bsat_timeout_stop(&toq, &timeouts[i]);
    }
    ymo_assert(bsat_valid_items(&toq) == 1);
    ymo_assert(toq.head == &timeouts[IDX_TIMEOUTS_LAST]);
    ymo_assert(toq.tail == &timeouts[IDX_TIMEOUTS_LAST]);

    bsat_toq_stop(&toq);
    /* Cool! */
    return;
}


void test_bsat_timeout(void)
{
    EV_P = ev_default_loop(0);

    /* Create a TOQ and ensure it's valid: */
    bsat_toq_t toq;
    bsat_toq_init(EV_A_ &toq, test_callback, 0.1);

    /* Add some timeouts. */
    my_data_t data[NO_TEST_TIMEOUTS];
    bsat_timeout_t timeouts[NO_TEST_TIMEOUTS];

    for( size_t i=0; i<NO_TEST_TIMEOUTS; i++ ) {
        sprintf(data[i].label, "timeouts[%zu]", i);

        bsat_timeout_init(&timeouts[i]);
        timeouts[i].data = &data[i];
        bsat_timeout_start(&toq, &timeouts[i]);
    }
    ymo_assert(bsat_valid_items(&toq) == NO_TEST_TIMEOUTS);

    /* Run the loop, verify we got called back: */
    ev_run(loop, 0);
    ymo_assert(last_toq == &toq);
    ymo_assert(last_item == &(timeouts[IDX_TIMEOUTS_LAST]));
    ymo_assert(bsat_valid_items(&toq) == 0);
    ymo_assert(no_calls == NO_TEST_TIMEOUTS);

    /* Run once more and confirm only a single item: */
    bsat_timeout_reset(&toq, &timeouts[0]);
    ymo_assert(bsat_valid_items(&toq) == 1);
    ev_run(loop, 0);
    ymo_assert(bsat_valid_items(&toq) == 0);
    ymo_assert(no_calls == NO_TEST_TIMEOUTS+1);
    bsat_toq_stop(&toq);

    /* Cool! */
    return;
}


/*-------------------------------------------------------------*
 * Main:
 *-------------------------------------------------------------*/
int main(int argc, char** argv)
{
    test_bsat_toq();
    test_bsat_order();
    test_bsat_timeout();
    return 0;
}
