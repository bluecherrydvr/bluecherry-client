#include "FileUtils.h"
#include <QFileDialog>
#include <QSettings>

QString getSaveFileNameExt(QWidget *parent, const QString &caption, const QString &defaultDir,
        const QString &dirCacheKey, const QString &defaultFilename, const QString &filter, bool autoSelectFilter)
{
    QSettings settings;
    QString dir = dirCacheKey.isNull() ? defaultDir : settings.value(dirCacheKey, defaultDir).toString();

    if (!defaultFilename.isEmpty())
    {
        if (!dir.isEmpty())
            dir.append(QLatin1Char('/'));
        dir.append(sanitizeFilename(defaultFilename));
    }

    QString selFilter;
    if (autoSelectFilter)
        selFilter = filter;

    QString result = QFileDialog::getSaveFileName(parent, caption, dir, filter, &selFilter);

    if (!result.isEmpty() && !dirCacheKey.isNull())
        settings.setValue(dirCacheKey, QFileInfo(result).absolutePath());

    return result;
}

QString sanitizeFilename(const QString &filename)
{
    QChar forbidden[] = { QLatin1Char('/'), QLatin1Char('\\'), QLatin1Char('?'),
                          QLatin1Char('"'), QLatin1Char(':'), QLatin1Char('*'),
                          QLatin1Char('<'), QLatin1Char('>'), QLatin1Char('|') };

    /* This does not prevent all invalid filenames; notably, it does not check for control
     * characters, Windows reserved filenames (e.g. NUL), and several other obscure situations.
     * The purpose is to clear *common* but invalid characters from filenames.

     * Could be implemented more efficiently. */

    QString re = filename;

    for (int i = 0; i < re.size(); ++i)
    {
        for (int j = 0; j < (sizeof(forbidden) / sizeof(QChar)); ++j)
        {
            if (re[i] == forbidden[j])
            {
                re[i] = QLatin1Char('_');
                break;
            }
        }
    }

    if (re == QLatin1String(".") || re == QLatin1String(".."))
        return QString();

    return re;
}
