#include "GstVideoBuffer.h"

GstVideoBuffer::GstVideoBuffer(VideoHttpBuffer *buffer, QObject *parent) :
        QObject(parent), m_buffer(buffer)
{
    connect(buffer, SIGNAL(streamError(QString)), this, SIGNAL(streamError(QString)));
    connect(buffer, SIGNAL(bufferingStarted()), this, SIGNAL(bufferingStarted()));
    connect(buffer, SIGNAL(bufferingReady()), this, SIGNAL(bufferingReady()));
    connect(buffer, SIGNAL(bufferingStopped()), this, SIGNAL(bufferingStopped()));
    connect(buffer, SIGNAL(bufferingFinished()), this, SIGNAL(bufferingFinished()));
}

GstVideoBuffer::~GstVideoBuffer()
{
}

void GstVideoBuffer::startBuffering()
{
    m_buffer.data()->startBuffering();
}

bool GstVideoBuffer::isBuffering() const
{
    return m_buffer.data()->isBuffering();
}

bool GstVideoBuffer::isBufferingFinished() const
{
    return m_buffer.data()->isBufferingFinished();
}

int GstVideoBuffer::bufferedPercent() const
{
    return m_buffer.data()->bufferedPercent();
}

void GstVideoBuffer::clearPlayback()
{
    m_buffer.data()->clearPlayback();
}

GstElement * GstVideoBuffer::setupSrcElement(GstElement *pipeline)
{
    return m_buffer.data()->setupSrcElement(pipeline);
}
