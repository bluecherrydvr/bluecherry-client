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

#ifndef VERSION_H
#define VERSION_H

#include <QString>

class Version
{

public:
    static Version fromString(const QString &string);

    explicit Version(quint16 major, quint16 minor, quint16 fix, const QString &spec);

    Version();
    Version(const Version &copyMe);

    Version operator = (const Version &copyMe);
    bool operator > (const Version &compareTo) const;

    bool isValid() const;

private:
    quint16 m_major;
    quint16 m_minor;
    quint16 m_fix;
    QString m_spec;

};

#endif // VERSION_H
