#include "VideoPlayerBackend_gst.h"
#include "VideoHttpBuffer.h"
#include <QUrl>
#include <QDebug>
#include <QApplication>
#include <QDir>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/video/video.h>
#include <glib.h>

VideoPlayerBackend::VideoPlayerBackend(QObject *parent)
    : QObject(parent), m_pipeline(0), m_videoLink(0), m_sink(0), m_videoBuffer(0), m_state(Stopped),
      m_playbackSpeed(1.0)
{
    m_videoBuffer = new VideoHttpBuffer(this);

    if (!initGStreamer(&m_errorMessage))
        setError(true, m_errorMessage);
}

VideoPlayerBackend::~VideoPlayerBackend()
{
    if (m_videoBuffer)
        m_videoBuffer->clearPlayback();
    clear();
}

bool VideoPlayerBackend::initGStreamer(QString *errorMessage)
{
    static bool loaded = false;
    if (loaded)
        return true;

    GError *err;
    if (gst_init_check(0, 0, &err) == FALSE)
    {
        Q_ASSERT(err);
        qWarning() << "GStreamer initialization failed:" << err->message;
        if (errorMessage)
            *errorMessage = QString::fromLatin1("initialization failed: ") + QString::fromLatin1(err->message);
        g_error_free(err);
        return false;
    }

#if defined(Q_OS_LINUX) && !defined(GSTREAMER_PLUGINS)
    /* Use the system installation of gstreamer on linux, where we don't need to
     * explicitly load our plugins. */
    return true;
#endif

#ifdef Q_OS_WIN
#define EXT ".dll"
#else
#define EXT ".so"
#endif

    const char *plugins[] =
    {
        "libgsttypefindfunctions"EXT, "libgstapp"EXT, "libgstdecodebin2"EXT, "libgstmatroska"EXT,
        "libgstffmpegcolorspace"EXT, "libgstcoreelements"EXT,
#ifndef Q_OS_WIN
        "libgstffmpeg"EXT,
#endif
#ifdef Q_OS_WIN
        "libgstffmpeg-lgpl"EXT, "libgstautodetect"EXT, "libgstdirectsound"EXT,
#elif defined(Q_OS_MAC)
        "libgstosxaudio"EXT,
#endif
        0
    };

#undef EXT
#if defined(Q_OS_MAC)
    QString pluginPath = QApplication::applicationDirPath() + QLatin1String("/../PlugIns/gstreamer/");
#else
    QString pluginPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + QLatin1String("/"));
#endif

#ifdef GSTREAMER_PLUGINS
    QString ppx = QDir::toNativeSeparators(QString::fromLatin1(GSTREAMER_PLUGINS "/"));
    if (QDir::isAbsolutePath(ppx))
        pluginPath = ppx;
    else
        pluginPath += QDir::separator() + ppx;
#endif

    if (!QFile::exists(pluginPath))
    {
        qWarning() << "gstreamer: Plugin path" << pluginPath << "does not exist";
        if (errorMessage)
            *errorMessage = QString::fromLatin1("plugin path (%1) does not exist").arg(pluginPath);
        return false;
    }

    bool success = true;
    QByteArray path = QFile::encodeName(pluginPath);
    int pathEnd = path.size();

    if (errorMessage)
        errorMessage->clear();

    for (const char **p = plugins; *p; ++p)
    {
        path.truncate(pathEnd);
        path.append(*p);

        GError *err = 0;
        GstPlugin *plugin = gst_plugin_load_file(path.constData(), &err);
        if (!plugin)
        {
            Q_ASSERT(err);
            qWarning() << "gstreamer: Failed to load plugin" << *p << ":" << err->message;
            if (errorMessage)
                errorMessage->append(QString::fromLatin1("plugin '%1' failed: %2\n").arg(QLatin1String(*p))
                                     .arg(QLatin1String(err->message)));
            g_error_free(err);
            success = false;
        }
        else
        {
            Q_ASSERT(!err);
            gst_object_unref(plugin);
        }
    }

    if (success)
        loaded = true;
    return success;
}

GstBusSyncReply VideoPlayerBackend::staticBusHandler(GstBus *bus, GstMessage *msg, gpointer data)
{
    Q_ASSERT(data);
    return ((VideoPlayerBackend*)data)->busHandler(bus, msg);
}

void VideoPlayerBackend::staticDecodePadReady(GstDecodeBin *bin, GstPad *pad, gboolean islast, gpointer user_data)
{
    Q_ASSERT(user_data);
    static_cast<VideoPlayerBackend*>(user_data)->decodePadReady(bin, pad, islast);
}

void VideoPlayerBackend::setSink(GstElement *sink)
{
    Q_ASSERT(!m_pipeline);
    Q_ASSERT(!m_sink);
    Q_ASSERT(sink);

    if (!m_sink && !m_pipeline)
    {
        m_sink = sink;
        g_object_ref(m_sink);
    }
}

bool VideoPlayerBackend::start(const QUrl &url)
{
    Q_ASSERT(!m_pipeline);
    if (state() == PermanentError || m_pipeline)
        return false;

    if (!m_sink)
    {
        setError(true, QLatin1String("Internal error: improper usage"));
        return false;
    }

    /* Pipeline */
    m_pipeline = gst_pipeline_new("stream");
    if (!m_pipeline)
    {
        setError(true, tr("Failed to create video pipeline (%1)").arg(QLatin1String("stream")));
        return false;
    }

    /* Buffered HTTP source */
    Q_ASSERT(m_videoBuffer);
    GstElement *source = m_videoBuffer->setupSrcElement(m_pipeline);
    if (!source)
    {
        setError(true, tr("Failed to create video pipeline (%1)").arg(QLatin1String("source")));
        delete m_videoBuffer;
        m_videoBuffer = 0;
        return false;
    }

    connect(m_videoBuffer, SIGNAL(streamError(QString)), SLOT(streamError(QString)));
    m_videoBuffer->start(url);

    /* Decoder */
    GstElement *decoder = gst_element_factory_make("decodebin2", "decoder");
    if (!decoder)
    {
        setError(true, tr("Failed to create video pipeline (%1)").arg(QLatin1String("decoder")));
        return false;
    }

    g_object_set(G_OBJECT(decoder),
                 "use-buffering", TRUE,
                 "max-size-time", 10 * GST_SECOND,
                 NULL);

    g_signal_connect(decoder, "new-decoded-pad", G_CALLBACK(staticDecodePadReady), this);

    /* Colorspace conversion (no-op if unnecessary) */
    GstElement *colorspace = gst_element_factory_make("ffmpegcolorspace", "colorspace");
    if (!colorspace)
    {
        setError(true, tr("Failed to create video pipeline (%1)").arg(QLatin1String("colorspace")));
        return false;
    }

    gst_bin_add_many(GST_BIN(m_pipeline), decoder, colorspace, m_sink, NULL);
    if (!gst_element_link(source, decoder))
    {
        setError(true, tr("Failed to create video pipeline (%1)").arg(QLatin1String("link decoder")));
        return false;
    }

    if (!gst_element_link(colorspace, m_sink))
    {
        setError(true, tr("Failed to create video pipeline (%1)").arg(QLatin1String("link sink")));
        return false;
    }

    /* This is the element that is linked to the decoder for video output; it will be linked when decodePadReady
     * gives us the video pad. */
    m_videoLink = colorspace;

    m_playbackSpeed = 1.0;

    /* We handle all messages in the sync handler, because we can't run a glib event loop.
     * Although linux does use glib's loop (and we could take advantage of that), it's better
     * to handle everything this way for windows and mac support. */
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    Q_ASSERT(bus);
    gst_bus_enable_sync_message_emission(bus);
    gst_bus_set_sync_handler(bus, staticBusHandler, this);
    gst_object_unref(bus);

    /* Move the pipeline into the PLAYING state. This call may block for a very long time
     * (up to several seconds), because it will block until the pipeline has completed that move. */
    gst_element_set_state(m_pipeline, GST_STATE_READY);
    connect(m_videoBuffer, SIGNAL(bufferingReady()), SLOT(playIfReady()));
    return true;
}

void VideoPlayerBackend::clear()
{
    if (m_sink)
        g_object_unref(m_sink);

    if (m_pipeline)
    {
        qDebug("gstreamer: Destroying pipeline");

        /* Disable the message handlers to avoid anything calling back into this instance */
        GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
        Q_ASSERT(bus);
        gst_bus_disable_sync_message_emission(bus);

        gst_object_unref(bus);

        gst_element_set_state(m_pipeline, GST_STATE_NULL);

        /* Ensure the transition to NULL completes */
        gst_element_get_state(m_pipeline, 0, 0, GST_CLOCK_TIME_NONE);
        gst_object_unref(GST_OBJECT(m_pipeline));
    }

    m_pipeline = m_videoLink = m_sink = 0;

    if (m_videoBuffer)
    {
        disconnect(m_videoBuffer, 0, this, 0);
        if (m_videoBuffer->parent() == this)
            m_videoBuffer->deleteLater();
        m_videoBuffer = new VideoHttpBuffer(this);
    }

    m_state = Stopped;
    m_errorMessage.clear();
}

void VideoPlayerBackend::setError(bool permanent, const QString &message)
{
    VideoState old = m_state;
    m_state = permanent ? PermanentError : Error;
    m_errorMessage = message;
    emit stateChanged(m_state, old);
}

void VideoPlayerBackend::streamError(const QString &message)
{
    qDebug() << "VideoPlayerBackend: stopping stream due to error:" << message;
    if (m_pipeline)
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
    setError(true, message);
}

void VideoPlayerBackend::playIfReady()
{
    if (!m_pipeline)
        return;
    GstState c, p;
    gst_element_get_state(m_pipeline, &c, &p, 0);
    if (c == GST_STATE_READY)
        play();
}

void VideoPlayerBackend::play()
{
    if (!m_pipeline)
        return;
    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    emit playbackSpeedChanged(m_playbackSpeed);
}

void VideoPlayerBackend::pause()
{
    if (!m_pipeline)
        return;
    gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
}

void VideoPlayerBackend::restart()
{
    if (!m_pipeline)
        return;
    gst_element_set_state(m_pipeline, GST_STATE_READY);

    VideoState old = m_state;
    m_state = Stopped;
    emit stateChanged(m_state, old);
}

qint64 VideoPlayerBackend::duration() const
{
    if (m_pipeline)
    {
        GstFormat fmt = GST_FORMAT_TIME;
        gint64 re = 0;
        if (gst_element_query_duration(m_pipeline, &fmt, &re))
            return re;
    }

    return -1;
}

qint64 VideoPlayerBackend::position() const
{
    if (!m_pipeline)
        return -1;
    else if (m_state == Stopped)
        return 0;
    else if (m_state == Done)
        return duration();

    GstFormat fmt = GST_FORMAT_TIME;
    gint64 re = 0;
    if (!gst_element_query_position(m_pipeline, &fmt, &re))
        re = -1;
    return re;
}

bool VideoPlayerBackend::isSeekable() const
{
    if (!m_pipeline)
        return false;

    GstQuery *query = gst_query_new_seeking(GST_FORMAT_TIME);
    gboolean re = gst_element_query(m_pipeline, query);
    if (re)
    {
        gboolean seekable;
        gst_query_parse_seeking(query, 0, &seekable, 0, 0);
        re = seekable;
    }
    else
        qDebug() << "gstreamer: Failed to query seeking properties of the stream";

    gst_query_unref(query);
    return re;
}

bool VideoPlayerBackend::seek(qint64 position)
{
    if (!m_pipeline)
        return false;

    if (state() != Playing && state() != Paused)
    {
        qDebug() << "gstreamer: Stream is not playing or paused, ignoring seek";
        return false;
    }

    gboolean re = gst_element_seek(m_pipeline, m_playbackSpeed, GST_FORMAT_TIME,
                            (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SKIP |
                                           GST_SEEK_FLAG_KEY_UNIT /* removing this will seek between
                                                                   * keyframes, but is much slower */
                                           ),
                            GST_SEEK_TYPE_SET, position, GST_SEEK_TYPE_SET, GST_CLOCK_TIME_NONE);

    if (!re)
    {
        qDebug() << "gstreamer: seek to position" << position << "failed";
        emit nonFatalError(tr("Seeking failed"));
    }

    return re ? true : false;
}

bool VideoPlayerBackend::setSpeed(double speed)
{
    if (!m_pipeline)
        return false;

    if (speed == m_playbackSpeed)
        return true;

    if (speed == 0)
        return false;

    gboolean re = gst_element_seek(m_pipeline, speed, GST_FORMAT_TIME,
                                   GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SKIP),
                                   GST_SEEK_TYPE_SET, position(), GST_SEEK_TYPE_SET, GST_CLOCK_TIME_NONE);

    if (!re)
    {
        qDebug() << "gstreamer: Setting playback speed failed";
        emit nonFatalError(tr("Playback speed failed"));
    }
    else
    {
        m_playbackSpeed = speed;
        qDebug() << "gstreamer: set playback speed to" << m_playbackSpeed;
        emit playbackSpeedChanged(m_playbackSpeed);
    }

    return re ? true : false;
}

void VideoPlayerBackend::decodePadReady(GstDecodeBin *bin, GstPad *pad, gboolean islast)
{
    Q_UNUSED(islast);

    /* TODO: Better cap detection */
    GstCaps *caps = gst_pad_get_caps_reffed(pad);
    Q_ASSERT(caps);
    gchar *capsstr = gst_caps_to_string(caps);
    gst_caps_unref(caps);

    if (QByteArray(capsstr).contains("video"))
    {
        qDebug("gstreamer: linking video decoder to pipeline");
        if (!gst_element_link(GST_ELEMENT(bin), m_videoLink))
        {
            setError(false, tr("Building video pipeline failed"));
            return;
        }
    }

    /* TODO: Audio */

    //free(capsstr);
}

/* Caution: This function is executed on all sorts of strange threads, which should
 * not be excessively delayed, deadlocked, or used for anything GUI-related. Primarily,
 * we want to emit signals (which will result in queued slot calls) or do queued method
 * invocation to handle GUI updates. */
GstBusSyncReply VideoPlayerBackend::busHandler(GstBus *bus, GstMessage *msg)
{
    Q_UNUSED(bus);

    switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_BUFFERING:
        {
            gint percent = 0;
            gst_message_parse_buffering(msg, &percent);
            qDebug() << "gstreamer: buffering" << percent << "%";
            emit bufferingStatus(percent);
        }
        break;

    case GST_MESSAGE_STATE_CHANGED:
        {
            if (m_state == PermanentError)
                break;

            GstState oldState, newState;
            gst_message_parse_state_changed(msg, &oldState, &newState, 0);
            VideoState vpState = m_state;

            switch (newState)
            {
            case GST_STATE_VOID_PENDING:
            case GST_STATE_NULL:
                if (m_state == Error)
                    break;
            case GST_STATE_READY:
                vpState = Stopped;
                break;
            case GST_STATE_PAUSED:
                vpState = Paused;
                emit durationChanged(duration());
                break;
            case GST_STATE_PLAYING:
                vpState = Playing;
                emit durationChanged(duration());
                break;
            }

            if (vpState != m_state)
            {
                VideoState old = m_state;
                m_state = vpState;
                emit stateChanged(m_state, old);
            }
        }
        break;

    case GST_MESSAGE_DURATION:
        emit durationChanged(duration());
        break;

    case GST_MESSAGE_EOS:
        {
            qDebug("gstreamer: end of stream");
            VideoState old = m_state;
            m_state = Done;
            emit stateChanged(m_state, old);
            emit endOfStream();
        }
        break;

    case GST_MESSAGE_ERROR:
        {
            gchar *debug;
            GError *error;

            gst_message_parse_error(msg, &error, &debug);
            qDebug() << "gstreamer: Error:" << error->message;
            qDebug() << "gstreamer: Debug:" << debug;

            /* Set the error message, but don't move to the error state, because that will stop playback,
             * possibly incorrectly. */
            m_errorMessage = QString::fromLatin1(error->message);

            g_free(debug);
            g_error_free(error);
        }
        break;

    case GST_MESSAGE_WARNING:
        {
            gchar *debug;
            GError *error;

            gst_message_parse_warning(msg, &error, &debug);
            qDebug() << "gstreamer: Warning:" << error->message;
            qDebug() << "gstreamer:   Debug:" << debug;

            g_free(debug);
            g_error_free(error);
        }
        break;

    default:
        break;
    }

    return GST_BUS_PASS;
}
