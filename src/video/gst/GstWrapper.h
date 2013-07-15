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

#ifndef GST_WRAPPER_H
#define GST_WRAPPER_H

#include <QStringList>

class GstPluginLoader;

class GstWrapper
{

public:
    GstWrapper();

    void setPluginLoader(GstPluginLoader *pluginLoader);
    void setPlugins(const QStringList &plugins);

    bool ensureInitialized();
    QString errorMessage() const;

private:
    GstPluginLoader *m_pluginLoader;
    QStringList m_plugins;
    bool m_initialized;
    QString m_errorMessage;

};

#endif // GST_WRAPPER_H
