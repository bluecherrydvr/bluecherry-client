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

#include "ui/model/EventsModel.h"
#include "ModelEventsCursor.h"
#include <QDebug>

ModelEventsCursor::ModelEventsCursor(QObject *parent)
    : EventsCursor(parent), m_model(0), m_index(0), m_cachedNextIndex(-1), m_cachedPreviousIndex(-1)
{
}

ModelEventsCursor::~ModelEventsCursor()
{
}

EventData * ModelEventsCursor::current() const
{
    if (!m_model)
        return 0;

    QModelIndex currentIndex = m_model->index(m_index, 0, QModelIndex());
    return currentIndex.data(EventsModel::EventDataPtr).value<EventData*>();
}

int ModelEventsCursor::nextIndex(int currentIndex) const
{
    return currentIndex - 1; // we are going backwards, so next is smaller
}

int ModelEventsCursor::previousIndex(int currentIndex) const
{
    return currentIndex + 1; // we are going backwards, so previous is greter
}

bool ModelEventsCursor::isValidIndex(int index) const
{
    if (!m_model)
        return false;

    if (index < 0)
        return false;

    return index < m_model->rowCount();
}

bool ModelEventsCursor::acceptIndex(int index) const
{
    if (!isValidIndex(index))
        return false;

    QModelIndex modelIndex = m_model->index(index, 0, QModelIndex());
    EventData *event = modelIndex.data(EventsModel::EventDataPtr).value<EventData*>();
    if (!event)
        return false;

    if (!m_cameraFilter)
        return true;

    return event->locationCamera() == m_cameraFilter.data();
}

void ModelEventsCursor::invalidateIndexCache()
{
    m_cachedNextIndex = -1;
    m_cachedPreviousIndex = -1;
}

void ModelEventsCursor::computeNextIndex()
{
    if (m_cachedNextIndex >= 0)
        return;

    int index = nextIndex(m_index);
    while (isValidIndex(index) && !acceptIndex(index))
        index = nextIndex(index);

    m_cachedNextIndex = index;
}

void ModelEventsCursor::computePreviousIndex()
{
    if (m_cachedPreviousIndex >= 0)
        return;

    int index = previousIndex(m_index);
    while (isValidIndex(index) && !acceptIndex(index))
        index = previousIndex(index);

    m_cachedPreviousIndex = index;
}

bool ModelEventsCursor::hasNext()
{
    computeNextIndex();
    return isValidIndex(m_cachedNextIndex);
}

bool ModelEventsCursor::hasPrevious()
{
    computePreviousIndex();
    return isValidIndex(m_cachedPreviousIndex);
}

void ModelEventsCursor::moveToNext()
{
    if (!hasNext()) // it computes m_cachedNextIndex for us
        return;

    m_index = m_cachedNextIndex;
    invalidateIndexCache();
    emit indexUpdated();
    emit eventSwitched(current());
}

void ModelEventsCursor::moveToPrevious()
{
    if (!hasPrevious()) // it computes m_cachedPreviousIndex for us
        return;

    m_index = m_cachedPreviousIndex;
    invalidateIndexCache();
    emit indexUpdated();
    emit eventSwitched(current());
}

void ModelEventsCursor::modelReset()
{
    // TODO: this is not a perfect solution as we loose read index value here
    // possibly we should not use model reset at all
    // it will work a lot better after http://improve.bluecherrydvr.com/issues/1186 is done
    invalidateIndexCache();
    emitAvailabilitySignals();
}

void ModelEventsCursor::rowsInserted(const QModelIndex &parent, int start, int end)
{
    if (parent.isValid())
        return;

    bool updated = false;
    if (start <= m_index)
    {
        m_index += (end - start + 1);
        updated = true;
    }

    emitAvailabilitySignals();
    invalidateIndexCache();

    if (updated)
        emit indexUpdated();
}

bool ModelEventsCursor::indexNotMoved(int index, int sourceFirst, int sourceLast, int destinationChild) const
{
    if (destinationChild < sourceFirst)
        return (index < destinationChild + sourceFirst - sourceLast - 1) || (index > sourceLast);
    else
        return (index < sourceFirst) || (index >= destinationChild);
}

bool ModelEventsCursor::indexDirectlyMoved(int index, int sourceFirst, int sourceLast) const
{
    return index >= sourceFirst && index <= sourceLast;
}

int ModelEventsCursor::indexAfterDirectMove(int index, int sourceLast, int destinationChild) const
{
    return index + destinationChild - sourceLast - 1;
}

void ModelEventsCursor::rowsMoved(const QModelIndex &sourceParent, int sourceFirst, int sourceLast, const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceParent.isValid() && destinationParent.isValid())
        return;

    if (sourceParent.isValid())
    {
        rowsRemoved(sourceParent, sourceFirst, sourceLast);
        return;
    }

    if (destinationParent.isValid())
    {
        rowsInserted(destinationParent, destinationChild, destinationChild + sourceLast - sourceFirst);
        return;
    }

    // from the Qt docs: destinationChild is not within the range of sourceFirst and sourceLast + 1
    Q_ASSERT(destinationChild < sourceFirst || destinationChild > sourceLast + 1);

    if (indexNotMoved(m_index, sourceFirst, sourceLast, destinationChild))
    {
        emitAvailabilitySignals();
        invalidateIndexCache();
        return;
    }

    if (indexDirectlyMoved(m_index, sourceFirst, sourceFirst))
    {
        m_index = indexAfterDirectMove(m_index, sourceLast, destinationChild);
        emitAvailabilitySignals();
        invalidateIndexCache();
        emit indexUpdated();
        return;
    }

    // indirectly moved
    if (destinationChild < sourceFirst)
        m_index += (sourceLast - sourceFirst + 1);
    else
        m_index -= (sourceLast - sourceFirst + 1);

    emitAvailabilitySignals();
    invalidateIndexCache();
    emit indexUpdated();
}

void ModelEventsCursor::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    qDebug() << "rowsRemoved";
    qDebug() << m_cameraFilter.data();

    if (parent.isValid())
        return;

    bool updated = false;
    if (start <= m_index)
    {
        if (end >= m_index)
            m_index = 0;
        else
            m_index -= (end - start + 1);
        updated = true;
    }

    emitAvailabilitySignals();
    invalidateIndexCache();

    if (updated)
        emit indexUpdated();
}

void ModelEventsCursor::modelDestroyed()
{
    m_model = 0;

    emitAvailabilitySignals();
    invalidateIndexCache();
    emit eventSwitched(current());
}

void ModelEventsCursor::setModel(QAbstractItemModel *model)
{
    if (m_model == model)
        return;

    if (m_model)
        disconnect(m_model, 0, this, 0);

    m_model = model;

    if (m_model)
    {
        connect(m_model, SIGNAL(modelReset()), this, SLOT(modelReset()));
        connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));
        connect(m_model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(rowsMoved(QModelIndex,int,int,QModelIndex,int)));
        connect(m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rowsRemoved(QModelIndex,int,int)));

        connect(m_model, SIGNAL(destroyed()), this, SLOT(modelDestroyed()));
    }

    m_index = 0;

    emitAvailabilitySignals();
    invalidateIndexCache();
    emit indexUpdated();
    emit eventSwitched(current());
}

void ModelEventsCursor::setCameraFilter(DVRCamera *cameraFilter)
{
    if (m_cameraFilter.data() == cameraFilter)
        return;

    m_cameraFilter = cameraFilter;
    emitAvailabilitySignals();
    invalidateIndexCache();
}

void ModelEventsCursor::setIndex(int index)
{
    if (!m_model)
        return;

    if (m_index == index)
        return;

    m_index = index;
    emitAvailabilitySignals();
    invalidateIndexCache();
    emit indexUpdated();
    emit eventSwitched(current());
}
