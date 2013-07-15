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

#include "GstWrapper.h"
#include "video/gst/GstPluginLoader.h"

#include <gst/gst.h>
#include <QDebug>

GstWrapper::GstWrapper() :
        m_pluginLoader(0), m_initialized(false)
{
}

void GstWrapper::setPluginLoader(GstPluginLoader *pluginLoader)
{
    m_pluginLoader = pluginLoader;
}

void GstWrapper::setPlugins(const QStringList &plugins)
{
    m_plugins = plugins;
}

bool GstWrapper::ensureInitialized()
{
    if (m_initialized)
        return true;

    GError *err;
    if (!gst_init_check(0, 0, &err))
    {
        Q_ASSERT(err);
        qWarning() << "GStreamer initialization failed:" << err->message;
        m_errorMessage = QString::fromLatin1("initialization failed: ") + QString::fromLatin1(err->message);
        g_error_free(err);
        return false;
    }

    if (m_pluginLoader)
        foreach (const QString &plugin, m_plugins)
            m_pluginLoader->loadGstPlugin(plugin);

    m_initialized = true;
    return true;
}

QString GstWrapper::errorMessage() const
{
    return m_errorMessage;
}
