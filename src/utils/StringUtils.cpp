#include "StringUtils.h"
#include <QApplication>

QString byteSizeString(quint64 bytes, ByteSizeFormat format)
{
    double v = bytes / double(1024);
    int n = 1;

    for (; v >= 1024; n++)
        v /= 1024;

    QString suffix;
    if (format == Bytes)
    {
        switch (n)
        {
        case 1: suffix = QApplication::translate("byteSizeString", "KB"); break;
        case 2: suffix = QApplication::translate("byteSizeString", "MB"); break;
        case 3: suffix = QApplication::translate("byteSizeString", "GB"); break;
        }
    }
    else if (format == BytesPerSecond)
    {
        switch (n)
        {
        case 1: suffix = QApplication::translate("byteSizeString", "KB/s"); break;
        case 2: suffix = QApplication::translate("byteSizeString", "MB/s"); break;
        case 3: suffix = QApplication::translate("byteSizeString", "GB/s"); break;
        }
    }

    return QString::number(v, 'f', 0) + QLatin1Char(' ') + suffix;
}
