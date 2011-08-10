#include "core/BluecherryApp.h"
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

    a.setApplicationVersion(QLatin1String("2.0.0-beta6"));

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
        QMessageBox::critical(0, a.tr("Error"), a.tr("This application is designed to utilize OpenGL "
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

    MainWindow w;
    w.show();

#ifdef BC_CUTOFF_DATE
    if (QDateTime::currentDateTime().addDays(3) >= QDateTime::fromTime_t(BC_CUTOFF_DATE))
    {
        int days = QDateTime::currentDateTime().daysTo(QDateTime::fromTime_t(BC_CUTOFF_DATE));
        QMessageBox::critical(&w, a.tr("Bluecherry - Beta Expiration"),
                              (days > 0) ? a.tr("This beta release of the Bluecherry DVR software will expire in %1 days.\n"
                                                "After this date, the software will function fully.\n\nPlease download the latest "
                                                "update from www.bluecherrydvr.com").arg(days)
                                         :
                                           a.tr("This beta release of the Bluecherry DVR software has expired.\n\nPlease download "
                                                "the latest update from www.bluecherrydvr.com")
                              );
    }
#endif

    return a.exec();
}
