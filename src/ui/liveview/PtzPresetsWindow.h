#ifndef PTZPRESETSWINDOW_H
#define PTZPRESETSWINDOW_H

#include <QWidget>

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
    void moveToPreset(const QModelIndex &index);
    void presetsViewContextMenu(const QPoint &pos);

private:
    CameraPtzControl *m_ptz;
    PtzPresetsModel *m_presetsModel;
    QTreeView *m_presetsView;
};

#endif // PTZPRESETSWINDOW_H
