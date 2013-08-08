#include "GstVideoHttpBuffer.h"

GstVideoHttpBuffer::GstVideoHttpBuffer(VideoHttpBuffer *buffer, QObject *parent) :
        QObject(parent), m_buffer(buffer)
{
    connect(buffer, SIGNAL(streamError(QString)), this, SIGNAL(streamError(QString)));
    connect(buffer, SIGNAL(bufferingStarted()), this, SIGNAL(bufferingStarted()));
    connect(buffer, SIGNAL(bufferingReady()), this, SIGNAL(bufferingReady()));
    connect(buffer, SIGNAL(bufferingStopped()), this, SIGNAL(bufferingStopped()));
    connect(buffer, SIGNAL(bufferingFinished()), this, SIGNAL(bufferingFinished()));
}

GstVideoHttpBuffer::~GstVideoHttpBuffer()
{
}

void GstVideoHttpBuffer::startBuffering()
{
    m_buffer.data()->startBuffering();
}

bool GstVideoHttpBuffer::isBuffering() const
{
    return m_buffer.data()->isBuffering();
}

bool GstVideoHttpBuffer::isBufferingFinished() const
{
    return m_buffer.data()->isBufferingFinished();
}

int GstVideoHttpBuffer::bufferedPercent() const
{
    return m_buffer.data()->bufferedPercent();
}

void GstVideoHttpBuffer::clearPlayback()
{
    m_buffer.data()->clearPlayback();
}

GstElement * GstVideoHttpBuffer::setupSrcElement(GstElement *pipeline)
{
    return m_buffer.data()->setupSrcElement(pipeline);
}
