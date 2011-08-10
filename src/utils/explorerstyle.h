/****************************************************************************
**
** Copyright (c) 2007 Trolltech ASA <info@trolltech.com>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
** explorerstyle.h - version 1.0
****************************************************************************/

#ifndef EXPLORERSTYLE_H
#define EXPLORERSTYLE_H

#include <QtGui/qwindowsvistastyle.h>
#include <QtGui/qproxystyle.h>

class ExplorerStyle : public QWindowsVistaStyle
{
public:
    ExplorerStyle();
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget = 0) const;
    void drawControl(ControlElement element, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget) const;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                            QPainter *painter, const QWidget *widget) const;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const;
    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                         SubControl sc, const QWidget *widget) const;
    SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
				     const QPoint &pos, const QWidget *widget = 0) const;
    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                           const QWidget *widget = 0) const;
    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
    void polish(QWidget *widget);
    void unpolish(QWidget *widget);
    void polish(QPalette pal);
    void polish(QApplication *app);
    void unpolish(QApplication *app);
    QPalette standardPalette();

private:
    mutable QRect currentTopRect; //current toolbar top area size 
    mutable QRect currentBottomRect; //current toolbar top area size 
};
#endif //EXPLORERSTYLE_H
