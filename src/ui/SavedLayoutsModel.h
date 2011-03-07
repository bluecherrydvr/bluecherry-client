#ifndef SAVEDLAYOUTSMODEL_H
#define SAVEDLAYOUTSMODEL_H

#include <QAbstractListModel>
#include <QVector>

class SavedLayoutsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum
    {
        LayoutDataRole = Qt::UserRole
    };

    /* The singleton instance of this class should generally be used, because it allows
     * synchronziation across multiple windows. It is parented to the BluecherryApp instance. */
    static SavedLayoutsModel *instance();

    explicit SavedLayoutsModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    bool isNewLayoutItem(int row) const { return row == items.size() - 1; }

private:
    static SavedLayoutsModel *m_instance;

    struct SavedLayoutData
    {
        QString name;
        QByteArray data;
    };

    QVector<SavedLayoutData> items;

    void populate();
};

#endif // SAVEDLAYOUTSMODEL_H
