#ifndef PTZPRESETSWINDOW_H
#define PTZPRESETSWINDOW_H

#include <QWidget>
#include <QModelIndex>

class CameraPtzControl;
class QModelIndex;
class QTreeView;
class PtzPresetsModel;

class PtzPresetsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PtzPresetsWindow(CameraPtzControl *ptzControl, QWidget *parent = 0);

private slots:
    void newPreset();
    void moveToPreset(const QModelIndex &index = QModelIndex());
    void updatePreset(const QModelIndex &index = QModelIndex());
    void renamePreset(const QModelIndex &index = QModelIndex());
    void deletePreset(const QModelIndex &index = QModelIndex());
    void presetsViewContextMenu(const QPoint &pos);

private:
    CameraPtzControl *m_ptz;
    PtzPresetsModel *m_presetsModel;
    QTreeView *m_presetsView;
};

#endif // PTZPRESETSWINDOW_H
