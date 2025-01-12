#include <time.h>
#include <stdbool.h>
#include "WeatherSensor.h"
#include "radiolib.h"


// Flag to indicate that a packet was received
volatile bool receivedFlag = false;

// C-style interrupt handler
void setReceivedFlag(void) {
    receivedFlag = true;
}

