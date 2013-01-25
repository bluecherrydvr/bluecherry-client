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
