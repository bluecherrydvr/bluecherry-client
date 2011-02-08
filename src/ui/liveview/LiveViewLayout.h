#ifndef LIVEVIEWLAYOUT_H
#define LIVEVIEWLAYOUT_H

#include <QDeclarativeItem>
#include <QBasicTimer>

class LiveViewLayoutProps;

class LiveViewLayout : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(int rows READ rows WRITE setRows)
    Q_PROPERTY(int columns READ columns WRITE setColumns)
    Q_PROPERTY(QDeclarativeComponent* item READ item WRITE setItem)

public:
    explicit LiveViewLayout(QDeclarativeItem *parent = 0);

    int rows() const { return m_rows; }
    int columns() const { return m_columns; }
    void setGridSize(int rows, int columns);

    QDeclarativeItem *at(int row, int col) const { return m_items[row * m_columns + col]; }
    void set(int row, int col, QDeclarativeItem *item);

    void gridPos(const QPointF &pos, int *row, int *column);

    QDeclarativeComponent *item() const { return m_itemComponent; }
    void setItem(QDeclarativeComponent *c);

    static LiveViewLayoutProps *qmlAttachedProperties(QObject *object);

public slots:
    void setRows(int r) { setGridSize(r, m_columns); }
    void insertRow(int row);
    void removeRow(int row);

    void setColumns(int c) { setGridSize(m_rows, c); }
    void insertColumn(int column);
    void removeColumn(int column);

protected:
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
    virtual void timerEvent(QTimerEvent *event);

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dropEvent(QGraphicsSceneDragDropEvent *event);

private:
    int m_rows, m_columns;
    QList<QDeclarativeItem*> m_items;
    QDeclarativeComponent *m_itemComponent;
    QBasicTimer m_layoutTimer;

    struct
    {
        QDeclarativeItem *dragItem;
        int dropRow, dropColumn;
    } m_dragDrop;

    void scheduleLayout();
    void doLayout();

    QDeclarativeItem *createNewItem();

    void updateDrag(QGraphicsSceneDragDropEvent *event);
};

QML_DECLARE_TYPEINFO(LiveViewLayout, QML_HAS_ATTACHED_PROPERTIES)

class LiveViewLayoutProps : public QObject
{
    Q_OBJECT
    friend class LiveViewLayout;

    Q_PROPERTY(bool isDragItem READ isDragItem NOTIFY isDragItemChanged)
    Q_PROPERTY(bool isDropTarget READ isDropTarget NOTIFY isDropTargetChanged)

public:
    LiveViewLayoutProps(QObject *parent)
        : QObject(parent), m_isDragItem(false), m_isDropTarget(false)
    {
    }

    static LiveViewLayoutProps *get(QObject *object, bool create = true)
    {
        return qobject_cast<LiveViewLayoutProps*>(qmlAttachedPropertiesObject<LiveViewLayout>(object, create));
    }

    bool isDragItem() const { return m_isDragItem; }
    bool isDropTarget() const { return m_isDropTarget; }

signals:
    void isDragItemChanged(bool isDragItem);
    void isDropTargetChanged(bool isDropTarget);

private:
    bool m_isDragItem, m_isDropTarget;

    void setIsDragItem(bool v) { if (m_isDragItem != v) emit isDragItemChanged((m_isDragItem = v)); }
    void setIsDropTarget(bool v) { if (m_isDropTarget != v) emit isDropTargetChanged((m_isDropTarget = v)); }
};

inline LiveViewLayoutProps *LiveViewLayout::qmlAttachedProperties(QObject *object)
{
    return new LiveViewLayoutProps(object);
}

#endif // LIVEVIEWLAYOUT_H
