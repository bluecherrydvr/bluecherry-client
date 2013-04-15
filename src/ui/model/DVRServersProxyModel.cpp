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

#include "DVRServersModel.h"
#include "DVRServersProxyModel.h"

DVRServersProxyModel::DVRServersProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_hideDisabledCameras(false)
{
}

DVRServersProxyModel::~DVRServersProxyModel()
{
}

bool DVRServersProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_ASSERT(sourceModel());

    const QModelIndex &index = sourceModel()->index(sourceRow, 0, sourceParent);
    DVRCamera *camera = index.data(DVRServersModel::DVRCameraRole).value<DVRCamera *>();

    if (camera && m_hideDisabledCameras && camera->data().disabled())
        return false;

    return true;
}

void DVRServersProxyModel::setHideDisabledCameras(bool hideDisabledCameras)
{
    if (m_hideDisabledCameras == hideDisabledCameras)
        return;

    m_hideDisabledCameras = hideDisabledCameras;
    invalidateFilter();
}
