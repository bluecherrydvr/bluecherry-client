#ifndef PTZPRESETSMODEL_H
#define PTZPRESETSMODEL_H

#include <QAbstractListModel>

class CameraPtzControl;

class PtzPresetsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum
    {
        PresetIdRole = Qt::UserRole
    };

    explicit PtzPresetsModel(QObject *parent = 0);

    void setPtzController(CameraPtzControl *ptzController);

    virtual int rowCount(const QModelIndex &parent) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

private slots:
    void updatePresets();
    void updateCurrentPreset();
    void clearPtzController() { setPtzController(0); }

private:
    CameraPtzControl *m_ptz;
    QList<QPair<int,QString> > m_presets;
};

#endif // PTZPRESETSMODEL_H
