#ifndef TRACEEVENTS_H
#define TRACEEVENTS_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QQueue>

#include <inttypes.h>

typedef uint32_t timestamp_t;

struct SoftwareTraceEvent {
    uint32_t    core_id;
    timestamp_t timestamp;
    uint16_t    id;
    uint32_t    value;
};

class SoftwareTraceEventDistributor : public QObject {
    Q_OBJECT

public:
    void emitEvents(QQueue<struct SoftwareTraceEvent*> &events);

signals:
    void softwareTraceEvent(struct SoftwareTraceEvent);
};

#endif // TRACEEVENTS_H
