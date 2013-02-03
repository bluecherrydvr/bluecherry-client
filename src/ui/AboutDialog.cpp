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

#include "AboutDialog.h"
#include <QGridLayout>
#include <QTextBrowser>
#include <QLabel>
#include <QFile>
#include <QApplication>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Bluecherry Client - About"));
    setFixedSize(500, 400);
    setModal(true);

    QGridLayout *layout = new QGridLayout(this);

    QLabel *logo = new QLabel;
    logo->setPixmap(QPixmap(QLatin1String(":/images/logo.png"))
                    .scaled(130, 130, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(logo, 0, 0);

    QLabel *text = new QLabel;
    text->setText(tr("Bluecherry DVR Client<br>Version %1").arg(QApplication::applicationVersion()));
    text->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QFont font = text->font();
    font.setPixelSize(15);
    text->setFont(font);

    layout->addWidget(text, 0, 1);
    layout->setColumnStretch(1, 1);

    QTextBrowser *license = new QTextBrowser;
    license->setHtml(getLicenseText());
    license->setStyleSheet(QLatin1String("font-size: 12px"));
    license->setReadOnly(true);
    license->setOpenExternalLinks(true);
    license->setTabChangesFocus(true);
    layout->addWidget(license, 1, 0, 1, 2);

    font = QFont();
    font.setStyleHint(QFont::SansSerif);
    font.setPixelSize(13);
    license->setFont(font);
}

QString AboutDialog::getLicenseText() const
{
    QFile file(QLatin1String(":/license.txt"));
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    return QString::fromUtf8(file.readAll());
}
