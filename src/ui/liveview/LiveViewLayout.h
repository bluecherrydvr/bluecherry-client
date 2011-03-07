#ifndef LIVEVIEWLAYOUT_H
#define LIVEVIEWLAYOUT_H

#include <QDeclarativeItem>
#include <QBasicTimer>

class LiveViewLayoutProps;

class LiveViewLayout : public QDeclarativeItem
{
    Q_OBJECT
    Q_ENUMS(DragDropMode)

    Q_PROPERTY(int rows READ rows WRITE setRows)
    Q_PROPERTY(int columns READ columns WRITE setColumns)
    Q_PROPERTY(QDeclarativeComponent* item READ item WRITE setItem)
    Q_PROPERTY(QDeclarativeItem* dropTarget READ dropTarget NOTIFY dropTargetChanged)
    Q_PROPERTY(QDeclarativeItem* dragItem READ dragItem NOTIFY dragItemChanged)

public:
    enum DragDropMode {
        DragReplace,
        DragSwap
    };

    explicit LiveViewLayout(QDeclarativeItem *parent = 0);
    virtual ~LiveViewLayout();

    int rows() const { return m_rows; }
    int columns() const { return m_columns; }
    void setGridSize(int rows, int columns);
    void gridPos(const QPointF &pos, int *row, int *column);
    bool gridPos(QDeclarativeItem *item, int *row, int *column);

    QSize idealSize() const;

    QDeclarativeItem *at(int row, int col) const { return m_items[row * m_columns + col]; }
    Q_INVOKABLE void set(int row, int col, QDeclarativeItem *item);

    /* Add a new item, automatically placing it in the best available position */
    QDeclarativeItem *addItemAuto();
    QDeclarativeItem *addItem(int row, int column);

    QDeclarativeItem *takeItem(int row, int column);
    Q_INVOKABLE QDeclarativeItem *takeItem(QDeclarativeItem *item);

    /* The item created to fill spaces in the layout */
    QDeclarativeComponent *item() const { return m_itemComponent; }
    void setItem(QDeclarativeComponent *c);

    QDeclarativeItem *dropTarget() const;
    QDeclarativeItem *dragItem() const;

    /* Called at the start of a drag movement operation for the item */
    Q_INVOKABLE void startDrag(QDeclarativeItem *item, DragDropMode mode = DragSwap);
    Q_INVOKABLE void updateDrag();
    Q_INVOKABLE bool drop();
    Q_INVOKABLE void endDrag(bool dropped);

    Q_INVOKABLE QPointF cursorItemPos() const;

    /* Use LiveViewLayoutProps::get() to get the properties attached to
     * an object; this is for use by QML. */
    static LiveViewLayoutProps *qmlAttachedProperties(QObject *object);

public slots:
    void setRows(int r) { setGridSize(r, m_columns); }
    void insertRow(int row);
    void appendRow() { insertRow(rows()); }
    void removeRow(int row);
    void removeRow() { setGridSize(m_rows-1, m_columns); }

    void setColumns(int c) { setGridSize(m_rows, c); }
    void insertColumn(int column);
    void appendColumn() { insertColumn(columns()); }
    void removeColumn(int column);
    void removeColumn() { setGridSize(m_rows, m_columns-1); }

    void setGridSize(int size) { setGridSize(size, size); }

    void removeItem(QDeclarativeItem *item);

signals:
    void dropTargetChanged(QDeclarativeItem *item);
    void dragItemChanged(QDeclarativeItem *item);
    void idealSizeChanged(const QSize &idealSize);

protected:
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
    virtual void timerEvent(QTimerEvent *event);

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dropEvent(QGraphicsSceneDragDropEvent *event);

private slots:
    void scheduleLayout();
    void updateIdealSize();

private:
    int m_rows, m_columns;
    QList<QDeclarativeItem*> m_items;
    QDeclarativeComponent *m_itemComponent;
    QBasicTimer m_layoutTimer;

    struct DragDropData;
    DragDropData *drag;

    void doLayout();

    QDeclarativeItem *createNewItem();
};

QML_DECLARE_TYPEINFO(LiveViewLayout, QML_HAS_ATTACHED_PROPERTIES)

class LiveViewLayoutProps : public QObject
{
    Q_OBJECT
    friend class LiveViewLayout;

    Q_PROPERTY(bool isDragItem READ isDragItem NOTIFY isDragItemChanged)
    Q_PROPERTY(bool isDropTarget READ isDropTarget NOTIFY isDropTargetChanged)
    Q_PROPERTY(QSizeF sizeHint READ sizeHint WRITE setSizeHint NOTIFY sizeHintChanged)
    Q_PROPERTY(QSizeF sizePadding READ sizePadding WRITE setSizePadding)
    Q_PROPERTY(bool fixedAspectRatio READ fixedAspectRatio WRITE setFixedAspectRatio)

public:
    LiveViewLayoutProps(QObject *parent)
        : QObject(parent), m_sizePadding(0, 0), m_isDragItem(false), m_isDropTarget(false), m_fixedAspectRatio(false)
    {
    }

    static LiveViewLayoutProps *get(QObject *object, bool create = true)
    {
        return static_cast<LiveViewLayoutProps*>(qmlAttachedPropertiesObject<LiveViewLayout>(object, create));
    }

    bool isDragItem() const { return m_isDragItem; }
    bool isDropTarget() const { return m_isDropTarget; }
    QSizeF sizeHint() const { return m_sizeHint; }
    bool fixedAspectRatio() const { return m_fixedAspectRatio; }
    QSizeF sizePadding() const { return m_sizePadding; }

public slots:
    void setSizeHint(const QSizeF &sizeHint);
    void setFixedAspectRatio(bool fixedAspectRatio);
    void setSizePadding(const QSizeF &sizePadding);

signals:
    void isDragItemChanged(bool isDragItem);
    void isDropTargetChanged(bool isDropTarget);
    void sizeHintChanged(const QSizeF &sizeHint);
    void layoutNeeded();

private:
    QSizeF m_sizeHint, m_sizePadding;
    bool m_isDragItem, m_isDropTarget, m_fixedAspectRatio;

    void setIsDragItem(bool v) { if (m_isDragItem != v) emit isDragItemChanged((m_isDragItem = v)); }
    void setIsDropTarget(bool v) { if (m_isDropTarget != v) emit isDropTargetChanged((m_isDropTarget = v)); }
};

inline LiveViewLayoutProps *LiveViewLayout::qmlAttachedProperties(QObject *object)
{
    return new LiveViewLayoutProps(object);
}

#endif // LIVEVIEWLAYOUT_H
