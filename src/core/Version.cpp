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

#include "Version.h"
#include <QStringList>

Version Version::fromString(const QString &string)
{
    QStringList parts = string.split(QLatin1String("."));
    if (parts.size() < 3 || parts.size() > 5)
        return Version();

    bool ok;
    quint16 major = parts.at(0).toUShort(&ok);
    if (!ok)
        return Version();
    quint16 minor = parts.at(1).toUShort(&ok);
    if (!ok)
        return Version();
    quint16 fix = parts.at(2).toUShort(&ok);
    if (!ok)
        return Version();

    if (parts.size() == 4)
        return Version(major, minor, fix, parts.at(3));
    else
        return Version(major, minor, fix, QString());
}

Version::Version(quint16 major, quint16 minor, quint16 fix, const QString &spec)
    : m_major(major), m_minor(minor), m_fix(fix), m_spec(spec)
{
}

Version::Version()
    : m_major(0), m_minor(0), m_fix(0)
{
}

Version::Version(const Version &copyMe)
    : m_major(), m_minor(), m_fix(), m_spec()
{
    m_major = copyMe.m_major;
    m_minor = copyMe.m_minor;
    m_fix = copyMe.m_fix;
    m_spec = copyMe.m_spec;
}

Version Version::operator = (const Version &copyMe)
{
    m_major = copyMe.m_major;
    m_minor = copyMe.m_minor;
    m_fix = copyMe.m_fix;
    m_spec = copyMe.m_spec;

    return *this;
}

bool Version::operator > (const Version &compareTo) const
{
    Q_ASSERT(isValid());
    Q_ASSERT(compareTo.isValid());

    if (m_major > compareTo.m_major)
        return true;
    if (m_major < compareTo.m_major)
        return false;
    if (m_minor > compareTo.m_minor)
        return true;
    if (m_minor < compareTo.m_minor)
        return false;
    return m_fix > compareTo.m_fix;
}

bool Version::isValid() const
{
    return m_major != 0 || m_minor != 0 || m_fix != 0;
}
