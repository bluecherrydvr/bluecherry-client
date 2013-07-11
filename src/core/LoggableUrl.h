#ifndef LOGGABLE_URL_H
#define LOGGABLE_URL_H

#include <QUrl>

class QDebug;

class LoggableUrl
{

public:
    explicit LoggableUrl(const QUrl &url);
    LoggableUrl(const LoggableUrl &copyMe);

    LoggableUrl & operator = (const LoggableUrl &copyMe);

    void setUrl(const QUrl &url);
    QUrl url() const;

private:
    QUrl m_url;

};

QDebug & operator << (QDebug &debug, const LoggableUrl &loggableUrl);

#endif // LOGGABLE_URL_H
