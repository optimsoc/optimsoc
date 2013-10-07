#include "traceevents.h"

void SoftwareTraceEventDistributor::emitEvents(QQueue<struct SoftwareTraceEvent*> &events)
{
    while (!events.empty()) {
        struct SoftwareTraceEvent *event;

        event = events.dequeue();

        emit softwareTraceEvent(*event);

        // Finally destroy this event
        delete(event);
    }

    // Now we can also clear the container
    events.clear();
}
