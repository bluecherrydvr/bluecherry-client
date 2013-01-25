/*
 * Copyright 2010-2013 Bluecherry
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ImageDecodeTask.h"
#include <QImageReader>
#include <QBuffer>
#include <QDebug>

/* main.cpp */
extern const char *jpegFormatName;

ImageDecodeTask::ImageDecodeTask(QObject *caller, const char *callback, quint64 id)
    : ThreadTask(caller, callback), imageId(id)
{
}

void ImageDecodeTask::runTask()
{
    if (isCancelled() || m_data.isNull())
    {
        m_data.clear();
        return;
    }

    QBuffer buffer(&m_data);
    if (!buffer.open(QIODevice::ReadOnly))
    {
        qDebug() << "Image decoding buffer error:" << buffer.errorString();
        m_data.clear();
        return;
    }

    QImageReader reader(&buffer, jpegFormatName);
    /* This would be more efficient, but causes the decoding to fail with
     * Qt 4.6.2 on Ubuntu 10.04. Disabled for now as a result. Issue #473 */
    //reader.setAutoDetectImageFormat(false);

    bool ok = reader.read(&m_result);

    buffer.close();
    m_data.clear();

    if (!ok)
    {
        if (m_result.isNull())
        {
            qDebug() << "Image decoding error:" << reader.errorString();
            return;
        }
        else
            qDebug() << "Image decoding warning:" << reader.errorString();
    }

    m_scaleResults.resize(m_scaleSizes.size());
    for (int i = 0; i < m_scaleSizes.size(); ++i)
    {
        m_scaleResults[i] = m_result.scaled(m_scaleSizes[i], Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}
