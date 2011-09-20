#ifndef LIVEVIEWMANAGER_H
#define LIVEVIEWMANAGER_H

#include <QObject>

class MJpegStream;
class QAction;

class LiveViewManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int globalInterval READ globalInterval WRITE setGlobalInterval NOTIFY globalIntervalChanged)

public:
    explicit LiveViewManager(QObject *parent = 0);

    QList<MJpegStream*> mjpegStreams() const;

    /* Global interval has the same semantics as MJpegStream intervals;
     * we receive only every Nth frame from the source. It may also be the
     * special value '0', which is a static 1 FPS stream. */
    int globalInterval() const { return m_globalInterval; }

    /* Create the standard FPS actions, */
    QList<QAction*> fpsActions(int currentInterval, QObject *target, const char *slot) const;

public slots:
    void setGlobalInterval(int interval);
    void setGlobalIntervalFromAction();

signals:
    void globalIntervalChanged(int interval);

private:
    QList<MJpegStream*> m_mjpegStreams;
    int m_globalInterval;

    friend class MJpegStream;
    void addStream(MJpegStream *stream);
    void removeStream(MJpegStream *stream);
};

#endif // LIVEVIEWMANAGER_H
