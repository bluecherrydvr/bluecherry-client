#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>

class QWidget;

QString sanitizeFilename(const QString &filename);
QString getSaveFileNameExt(QWidget *parent, const QString &caption, const QString &defaultDir,
                           const QString &dirCacheKey = QString(), const QString &filename = QString(),
                           const QString &filter = QString(), bool autoSelectFilter = true);

#endif // FILEUTILS_H
