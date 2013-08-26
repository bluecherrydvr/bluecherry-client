#include "LoggableUrl.h"

#include <QDebug>

LoggableUrl::LoggableUrl(const QUrl &url) :
        m_url(url)
{
}

LoggableUrl::LoggableUrl(const LoggableUrl &copyMe)
{
    *this = copyMe;
}

LoggableUrl & LoggableUrl::operator = (const LoggableUrl &copyMe)
{
    m_url = copyMe.m_url;

    return *this;
}

void LoggableUrl::setUrl(const QUrl &url)
{
    m_url = url;
}

QUrl LoggableUrl::url() const
{
    return m_url;
}

QDebug & operator << (QDebug &debug, const LoggableUrl &loggableUrl)
{
    QUrl urlWithHiddenPassword = loggableUrl.url();
    urlWithHiddenPassword.setPassword(QLatin1String("***"));
    debug << urlWithHiddenPassword.toString();

    return debug;
}
