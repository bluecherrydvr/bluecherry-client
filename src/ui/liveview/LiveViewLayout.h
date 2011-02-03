#ifndef LIVEVIEWLAYOUT_H
#define LIVEVIEWLAYOUT_H

#include <QDeclarativeItem>
#include <QBasicTimer>

class LiveViewLayout : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(int rows READ rows WRITE setRows)
    Q_PROPERTY(int columns READ columns WRITE setColumns)

public:
    explicit LiveViewLayout(QDeclarativeItem *parent = 0);

    int rows() const { return m_rows; }
    int columns() const { return m_columns; }
    void setGridSize(int rows, int columns);

    QDeclarativeItem *at(int row, int col) const { return m_items[row * m_columns + col]; }
    void set(int row, int col, QDeclarativeItem *item);

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

private:
    int m_rows, m_columns;
    QList<QDeclarativeItem*> m_items;
    QDeclarativeComponent *m_itemComponent;
    QBasicTimer m_layoutTimer;

    void scheduleLayout();
    void doLayout();

    QDeclarativeItem *createNewItem();
};

#endif // LIVEVIEWLAYOUT_H
