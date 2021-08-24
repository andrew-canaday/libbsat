#include "bsat.h"
#include "bsat_test.h"


/*-------------------------------------------------------------*
 * Tests:
 *-------------------------------------------------------------*/
void test_bsat_invoke_pending(void)
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

    /* Invoke all pending timeouts NOW: */
    bsat_toq_invoke_pending(&toq);

    /* Confirm that all of the scheduled timeouts fired: */
    ymo_assert(no_calls == NO_TEST_TIMEOUTS);

    /* Confirm that none are pending: */
    ymo_assert(bsat_valid_items(&toq) == 0);

    /* Confirm that all of the previously scheduled items are now inactive: */
    for( size_t i=0; i<NO_TEST_TIMEOUTS; i++ ) {
        ymo_assert(bsat_timeout_is_active(&timeouts[i]) == 0);
    }

    bsat_toq_stop(&toq);
    /* Cool! */
    return;
}


/*-------------------------------------------------------------*
 * Main:
 *-------------------------------------------------------------*/
int main(int argc, char** argv)
{
    test_bsat_invoke_pending();
    return 0;
}
