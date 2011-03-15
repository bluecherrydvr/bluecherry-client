#include "PtzPresetsModel.h"
#include "CameraPtzControl.h"
#include <QFont>

PtzPresetsModel::PtzPresetsModel(QObject *parent)
    : QAbstractListModel(parent), m_ptz(0)
{
}

void PtzPresetsModel::setPtzController(CameraPtzControl *ptzController)
{
    if (m_ptz == ptzController)
        return;

    if (m_ptz)
        m_ptz->disconnect(this);
    m_ptz = ptzController;
    if (m_ptz)
    {
        connect(m_ptz, SIGNAL(destroyed()), SLOT(clearPtzController()));
        connect(m_ptz, SIGNAL(infoUpdated()), SLOT(updatePresets()));
        connect(m_ptz, SIGNAL(currentPresetChanged(int)), SLOT(updateCurrentPreset()));
    }

    beginResetModel();
    m_presets.clear();
    endResetModel();

    updatePresets();
}

void PtzPresetsModel::updatePresets()
{
    if (!m_ptz)
    {
        if (!m_presets.isEmpty())
        {
            beginRemoveRows(QModelIndex(), 0, m_presets.size()-1);
            m_presets.clear();
            endRemoveRows();
        }
        return;
    }

    QMap<int,QString> updated = m_ptz->presets();

    int cRow = 0;
    for (QMap<int,QString>::Iterator it = updated.begin(); it != updated.end();)
    {
        if (cRow >= m_presets.size() || it.key() < m_presets[cRow].first)
        {
            /* New row */
            beginInsertRows(QModelIndex(), cRow, cRow);
            m_presets.insert(cRow, qMakePair(it.key(), it.value()));
            endInsertRows();
        }
        else if (it.key() > m_presets[cRow].first)
        {
            /* Remove row(s) */
            int last = cRow + 1;
            while (last < m_presets.size() && it.key() > m_presets[last].first)
                last++;

            /* last is one more than the last row we wish to remove */

            beginRemoveRows(QModelIndex(), cRow, last-1);
            m_presets.erase(m_presets.begin()+cRow, m_presets.begin()+last);
            endRemoveRows();

            /* Do not increment the iterator or change cRow's value */
            continue;
        }
        else if (m_presets[cRow].second != it.value())
        {
            m_presets[cRow].second = it.value();
            QModelIndex idx = index(cRow, 0);
            emit dataChanged(idx, idx);
        }

        ++it;
        ++cRow;
    }

    if (cRow < m_presets.size())
    {
        beginRemoveRows(QModelIndex(), cRow, m_presets.size()-1);
        m_presets.erase(m_presets.begin()+cRow, m_presets.end());
        endRemoveRows();
    }
}

void PtzPresetsModel::updateCurrentPreset()
{
    emit dataChanged(index(0, 0), index(m_presets.size()-1, 0));
}

int PtzPresetsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_presets.size();
}

Qt::ItemFlags PtzPresetsModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant PtzPresetsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() > m_presets.size())
        return QVariant();

    const QPair<int,QString> &data = m_presets[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return data.second;
    case Qt::FontRole:
    {
        QFont f;
        if (data.first == m_ptz->currentPreset())
            f.setBold(true);
        return f;
    }
    case PresetIdRole:
        return data.first;
    }

    return QVariant();
}

QVariant PtzPresetsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section != 0)
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole: return tr("Presets");
    }

    return QVariant();
}

bool PtzPresetsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_presets.size() || role != Qt::EditRole)
        return false;

    QString name = value.toString();
    if (name.isEmpty())
        return false;

    m_ptz->renamePreset(m_presets[index.row()].first, name);
    return true;
}
