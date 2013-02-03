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
