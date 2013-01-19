#ifndef MODELEVENTSCURSOR_H
#define MODELEVENTSCURSOR_H

#include "events/EventsCursor.h"
#include "core/DVRCamera.h"

class QAbstractItemModel;

class ModelEventsCursor : public EventsCursor
{
    Q_OBJECT

public:
    explicit ModelEventsCursor(QObject *parent = 0);
    virtual ~ModelEventsCursor();

    virtual EventData * current() const;
    virtual bool hasNext();
    virtual bool hasPrevious();
    virtual void moveToNext();
    virtual void moveToPrevious();

    void setModel(QAbstractItemModel *model);
    QAbstractItemModel * model() const { return m_model; }

    void setCameraFilter(DVRCamera cameraFilter);
    DVRCamera cameraFilter() const { return m_cameraFilter; }


    void setIndex(int index);
    int index() const { return m_index; }

signals:
    void indexUpdated();

private slots:
    void modelDestroyed();

private:
    DVRCamera m_cameraFilter;
    QAbstractItemModel *m_model;
    int m_index;
    int m_cachedNextIndex;
    int m_cachedPreviousIndex;

    bool invert() const;
    int nextIndex(int currentIndex) const;
    int previousIndex(int currentIndex) const;

    bool isValidIndex(int index) const;
    bool acceptIndex(int index) const;
    void invalidateIndexCache();
    void computeNextIndex();
    void computePreviousIndex();
};

#endif // MODELEVENTSCURSOR_H
