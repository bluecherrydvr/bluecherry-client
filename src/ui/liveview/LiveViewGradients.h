#ifndef LIVEVIEWGRADIENTS_H
#define LIVEVIEWGRADIENTS_H

#include <QDeclarativeImageProvider>

class LiveViewGradients : public QDeclarativeImageProvider
{
public:
    LiveViewGradients();

    virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
};

#endif // LIVEVIEWGRADIENTS_H
