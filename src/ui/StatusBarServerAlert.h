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

#ifndef STATUSBARSERVERALERT_H
#define STATUSBARSERVERALERT_H

#include <QWidget>

class QLabel;
class DVRServerRepository;

class StatusBarServerAlert : public QWidget
{
    Q_OBJECT

public:
    explicit StatusBarServerAlert(DVRServerRepository *serverRepository, QWidget *parent = 0);

private slots:
    void updateAlert();

protected:
    virtual void mousePressEvent(QMouseEvent *);

private:
    DVRServerRepository *m_serverRepository;
    QLabel *alertText;

};

#endif // STATUSBARSERVERALERT_H
