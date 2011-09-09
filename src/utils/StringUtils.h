#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <QString>

enum ByteSizeFormat {
    Bytes,
    BytesPerSecond
};

QString byteSizeString(quint64 bytes, ByteSizeFormat format);

#endif // STRINGUTILS_H
