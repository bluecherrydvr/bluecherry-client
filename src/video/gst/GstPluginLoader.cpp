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

#include "GstPluginLoader.h"

#include <gst/gst.h>
#include <QDebug>
#include <QDir>

void GstPluginLoader::setPaths(const QStringList &paths)
{
    m_paths = paths;
}

void GstPluginLoader::setPrefixes(const QStringList &prefixes)
{
    m_prefixes = prefixes;
}

void GstPluginLoader::setSuffixes(const QStringList &suffixes)
{
    m_suffixes = suffixes;
}

bool GstPluginLoader::loadGstPlugin(const QString &plugin)
{
    qDebug() << "gstreamer: Loading plugin" << plugin;

    foreach (const QString &path, m_paths)
        foreach (const QString &prefix, m_prefixes)
            foreach (const QString &suffix, m_suffixes)
                if (loadGstPluginFullPath(QString::fromAscii("%1/%2%3%4").arg(path).arg(prefix).arg(plugin).arg(suffix)))
                    return true;

    qDebug() << "gstreamer: Plugin not found" << plugin;

    return false;
}

bool GstPluginLoader::loadGstPluginFullPath(const QString &pluginFullPath)
{
    qDebug() << "gstreamer: Loading plugin" << pluginFullPath;

    if (!QFile::exists(pluginFullPath))
        return false;

    GError *error = 0;
    GstPlugin *plugin = gst_plugin_load_file(QFile::encodeName(pluginFullPath).constData(), &error);

    if (plugin)
    {
        Q_ASSERT(!error);
        gst_object_unref(plugin);
        return true;
    }

    Q_ASSERT(error);
    qWarning() << "gstreamer: Failed to load plugin" << pluginFullPath << ":" << error->message;
    g_error_free(error);
    return false;
}
