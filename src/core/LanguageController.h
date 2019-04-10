/*
 * Copyright 2010-2019 Bluecherry, LLC
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

#ifndef LANGUAGECONTROLLER_H
#define LANGUAGECONTROLLER_H

#include <QMap>
#include <QPair>
#include <QStringList>
#include <QTranslator>

class LanguageController
{
public:
	LanguageController();

	void setTranslationFilesPaths(const QStringList &paths);

	QMap<QString, QString> supportedLanguages();
	void loadLanguage(const QString &languageCode);

	bool supportsLanguage(const QString &languageCode);

	QString currentLanguage() const { return m_currentLanguageCode; }

private:
	void loadLanguages();
	void switchTranslator(QTranslator &translator, const QString &filename, const QString &directory);

	QStringList m_paths;
	mutable QMap<QString, QString> m_languages;
	mutable QMap<QString, QString> m_languagesPaths;
	QString m_currentLanguageCode;

	QTranslator m_appTranslator;
	QTranslator m_QtTranslator;

};

#endif // LANGUAGECONTROLLER_H
