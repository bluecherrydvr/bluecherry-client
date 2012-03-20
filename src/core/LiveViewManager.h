#ifndef LIVEVIEWMANAGER_H
#define LIVEVIEWMANAGER_H

#include <QObject>

class LiveStream;
class QAction;

class LiveViewManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(BandwidthMode)

    Q_PROPERTY(int bandwidthMode READ bandwidthMode WRITE setBandwidthMode NOTIFY bandwidthModeChanged)

public:
    enum BandwidthMode
    {
        FullBandwidth,
        LowBandwidth
    };

    explicit LiveViewManager(QObject *parent = 0);

    QList<LiveStream*> streams() const;

    BandwidthMode bandwidthMode() const { return m_bandwidthMode; }

    QList<QAction*> bandwidthActions(int currentMode, QObject *target, const char *slot) const;

public slots:
    void setBandwidthMode(int value);
    void setBandwidthModeFromAction();

signals:
    void bandwidthModeChanged(int value);

private:
    QList<LiveStream*> m_streams;
    BandwidthMode m_bandwidthMode;

    friend class LiveStream;
    void addStream(LiveStream *stream);
    void removeStream(LiveStream *stream);
};

#endif // LIVEVIEWMANAGER_H
