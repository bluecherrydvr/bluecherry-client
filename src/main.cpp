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

#include "bluecherry-config.h"
#include "core/BluecherryApp.h"
#include "live-stream/LiveStream.h"
#include "ui/MainWindow.h"
#include "ui/CrashReportDialog.h"
#include <QApplication>
#include <QSettings>
#include <QtPlugin>
#include <QMessageBox>
#include <QDateTime>
#include <QGLFormat>
#include <QImageReader>

#ifdef Q_OS_WIN
#include <utils/explorerstyle.h>
#endif

#ifdef QT_STATIC
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qgif)
#endif

#ifdef USE_BREAKPAD
void initBreakpad();
#endif

const char *jpegFormatName = "jpeg";

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /* These are used for the configuration file - do not change! */
    a.setOrganizationName(QLatin1String("bluecherry"));
    a.setOrganizationDomain(QLatin1String("bluecherrydvr.com"));
    a.setApplicationName(QLatin1String("bluecherry"));

    a.setApplicationVersion(QLatin1String(VERSION));

#ifdef Q_OS_WIN
    /* Use explorer style for fancier toolbars */
    if (a.style()->inherits("QWindowsXPStyle"))
        a.setStyle(new ExplorerStyle);
    /* Don't use the registry. */
    QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

    {
        QStringList args = a.arguments();
        args.takeFirst();
        if (args.size() >= 1 && args[0] == QLatin1String("--crash"))
        {
            CrashReportDialog dlg((args.size() >= 2) ? args[1] : QString());
            dlg.exec();
            if (dlg.result() != QDialog::Accepted)
                return 0;
        }
    }

#ifdef USE_BREAKPAD
    initBreakpad();
#endif

    if (!QGLFormat::hasOpenGL())
    {
        QMessageBox::critical(0, a.translate("@default", "Error"), a.translate("@default", "This application is designed to utilize OpenGL "
                                                    "acceleration, which is not supported by your system. "
                                                    "The application may not function correctly.\n\n"
                                                    "For help, contact support@bluecherrydvr.com."),
                              QMessageBox::Ok);
    }

    bcApp = new BluecherryApp;

    if (QImageReader::supportedImageFormats().contains("jpeg-turbo"))
    {
        jpegFormatName = "jpeg-turbo";
        qDebug("Using qjpeg-turbo");
    }

    LiveStream::init();

    MainWindow w(bcApp->serverRepository());
    w.show();

    return a.exec();
}
