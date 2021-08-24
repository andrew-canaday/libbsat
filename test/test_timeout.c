#include "bsat.h"
#include "bsat_test.h"


/*-------------------------------------------------------------*
 * Tests:
 *-------------------------------------------------------------*/
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
    test_bsat_timeout();
    return 0;
}
