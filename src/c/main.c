#include <pebble.h>
#include <math.h>

#include "setup.h"

int main()
{
    setup_watchface();
    app_event_loop();
    tear_down_watchface();
}