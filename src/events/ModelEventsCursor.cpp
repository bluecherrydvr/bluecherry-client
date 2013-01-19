#include "ui/EventsModel.h"
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

    return event->locationCamera() == m_cameraFilter;
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
        connect(m_model, SIGNAL(destroyed()), this, SLOT(modelDestroyed()));

    m_index = 0;

    emitAvailabilitySignals();
    invalidateIndexCache();
    emit indexUpdated();
    emit eventSwitched(current());
}

void ModelEventsCursor::setCameraFilter(DVRCamera cameraFilter)
{
    if (m_cameraFilter == cameraFilter)
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
