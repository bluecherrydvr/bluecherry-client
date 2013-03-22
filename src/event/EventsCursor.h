#ifndef EVENTSCURSOR_H
#define EVENTSCURSOR_H

#include <QObject>

class EventData;

class EventsCursor : public QObject
{
    Q_OBJECT

public:
    explicit EventsCursor(QObject *parent = 0);
    virtual ~EventsCursor();

    virtual EventData * current() const = 0;
    virtual bool hasNext() = 0;
    virtual bool hasPrevious() = 0;

public slots:
    virtual void moveToNext() = 0;
    virtual void moveToPrevious() = 0;

signals:
    void eventSwitched(EventData *event);

    void nextAvailabilityChanged(bool hasNext);
    void previousAvailabilityChanged(bool hasPrevious);

protected slots:
    void emitAvailabilitySignals();
};

#endif // EVENTSCURSOR_H
