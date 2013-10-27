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

#include "LanguageController.h"

#include <QApplication>
#include <QDir>
#include <QLocale>

LanguageController::LanguageController()
{
}

void LanguageController::setTranslationFilesPaths(const QStringList &paths)
{
    m_paths = paths;
}

QMap<QString, QString> LanguageController::supportedLanguages()
{
    if (m_languages.isEmpty())
        loadLanguages();

    return m_languages;
}

void LanguageController::loadLanguages()
{
    foreach (const QString &path, paths)
    {
        QDir tranlationsDir(path);

        QStringList languagesFilter;
        languagesFilter << "bluecherryclient_*.qm";
        QStringList languages = tranlationsDir.entryList(languagesFilter, QDir::Files);

        foreach (const QString &languageFile, languages)
        {
            QString localeCode = languageFile.remove(0, locale.indexOf('_') + 1);
            localeCode.chop(3);

            QLocale locale(localeCode);

            QString localizedLanguageName = locale.nativeLanguageName();

            m_languages.insert(localeCode, localizedLanguageName);
        }
    }
}

void LanguageController::switchTranslator(QTranslator &translator, const QString &filename)
{
    qApp->removeTranslator(&translator);

    if (translator.load(filename))
        qApp->installTranslator(&translator);
}

void LanguageController::loadLanguage(const QString &languageCode)
{
    if (m_currentLanguageCode != languageCode && m_languages.contains(languageCode))
    {
        m_currentLanguageCode = languageCode;
        QLocale locale = QLocale(m_currentLanguageCode);
        QLocale::setDefault(locale);
        switchTranslator(m_translator, QString("bluecherryclient_%1.qm").arg(m_currentLanguageCode));
        switchTranslator(m_translatorQt, QString("qt_%1.qm").arg(m_currentLanguageCode));
    }
}
