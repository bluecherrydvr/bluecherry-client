#ifndef IMAGEDECODETASK_H
#define IMAGEDECODETASK_H

#include "ThreadTask.h"
#include <QImage>
#include <QVector>

class ImageDecodeTask : public ThreadTask
{
public:
    const quint64 imageId;

    ImageDecodeTask(QObject *caller, const char *callback, quint64 imageId = 0);

    void setData(const QByteArray &data) { m_data = data; }
    void setScaleSizes(const QVector<QSize> &sizes) { m_scaleSizes = sizes; }

    QImage result() const { return m_result; }
    const QVector<QImage> &scaleResults() const { return m_scaleResults; }

protected:
    virtual void runTask();

private:
    QByteArray m_data;
    QVector<QSize> m_scaleSizes;
    QImage m_result;
    QVector<QImage> m_scaleResults;
};

#endif // IMAGEDECODETASK_H
