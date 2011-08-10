/****************************************************************************
**
** Copyright (c) 2007 Trolltech ASA <info@trolltech.com>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
** explorerstyle.cpp - version 1.0
****************************************************************************/

#include "explorerstyle.h"
#include <QtGui>
#include <qt_windows.h>
#include <uxtheme.h>

typedef HANDLE (WINAPI *PtrOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (WINAPI *PtrDrawThemeBackground)(HANDLE hTheme, HDC hdc, int iPartId, int iStateId, 
                                                 const RECT *pRect, OPTIONAL const RECT *pClipRect);
typedef bool (WINAPI *PtrIsAppThemed)();

static PtrDrawThemeBackground pDrawThemeBackground = 0;
static PtrOpenThemeData pOpenThemeData = 0;
static PtrIsAppThemed pIsAppThemed = 0;
static bool vista = false;

bool isAppThemed() 
{
#ifdef Q_OS_WIN
    return pIsAppThemed && pIsAppThemed();
#else
    return false;
#endif
}

ExplorerStyle::ExplorerStyle()
    : QWindowsVistaStyle()
{
#ifdef Q_OS_WIN
    QLibrary themeLib(QLatin1String("uxtheme"));
    themeLib.load();
    if (themeLib.isLoaded()) {  //resolve uxtheme functions
        pIsAppThemed =          (PtrIsAppThemed)themeLib.resolve("IsAppThemed");
        pDrawThemeBackground =  (PtrDrawThemeBackground)themeLib.resolve("DrawThemeBackground");
        pOpenThemeData  =       (PtrOpenThemeData)themeLib.resolve("OpenThemeData");
    }
#endif
    vista = QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA && QSysInfo::WindowsVersion < QSysInfo::WV_NT_based;
}

void drawRebarBackground(const QRect &rect,  QPainter *painter) {
    if (rect.isEmpty())
        return;
    QString cacheKey = QLatin1String("q_rebar_") + QString::number(rect.size().width())
                        + QLatin1Char('x') + QString::number(rect.size().height());
    QPixmap pixmap;
    if (!QPixmapCache::find(cacheKey, pixmap)) {
        pixmap = QPixmap(rect.size());
        const RECT wRect = {0, 0, pixmap.width(), pixmap.height()};
        HBITMAP bitmap = pixmap.toWinHBITMAP();
        HDC hdc = CreateCompatibleDC(qt_win_display_dc());
        HGDIOBJ oldhdc = (HBITMAP)SelectObject(hdc, bitmap);
        HTHEME theme = pOpenThemeData(0, L"REBAR");
        pDrawThemeBackground(theme, hdc, 0, 0, &wRect, NULL);
        pixmap = pixmap.fromWinHBITMAP(bitmap);
        SelectObject(hdc, oldhdc);
        DeleteObject(bitmap);
        DeleteDC(hdc);
        QPixmapCache::insert(cacheKey, pixmap);
    }
    painter->drawPixmap(rect.topLeft(), pixmap);
}

void ExplorerStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                                    QPainter *painter, const QWidget *widget) const
{
    QRect rect = option->rect;
    switch (element) {
    case PE_Widget:
        if (isAppThemed()) {
            if (QMainWindow *window = qobject_cast<QMainWindow*>(widget->window())) { 
                QRegion topreg;
                QRegion bottomreg;
                QMenuBar* menubar = qFindChild<QMenuBar*>(window);

                //We draw the menubar as part of the top toolbar area
                if (menubar) {
                    QRect rect(menubar->mapToParent(menubar->rect().topLeft()), menubar->rect().size());
                    topreg += rect;
                }

                drawRebarBackground(topreg.boundingRect(), painter);
                topreg = QRegion();

                //We need the bounding rect for all toolbars
                QList<QToolBar*> toolbars = qFindChildren<QToolBar*>(window);
                foreach(QToolBar *tb, toolbars) {
                    if (!tb->isFloating()) {
                        QRect rect(tb->mapToParent(tb->rect().topLeft()), tb->rect().size());
                        if (window->toolBarArea(tb) == Qt::TopToolBarArea)
                            topreg += rect;
                        else if (window->toolBarArea(tb) == Qt::BottomToolBarArea)
                            bottomreg += rect;
                    }
                }

                //This is a hack to workaround missing toolbar updates since
                //we now require updates that span across the whole toolbar area:
                QRect topRect = topreg.boundingRect();
                topRect.setWidth(window->width());
                if (currentTopRect != topRect) {
                    currentTopRect = topRect;
                    window->update(topRect);
                }

                QRect bottomRect = bottomreg.boundingRect();
                bottomRect.setWidth(window->width());
                if (currentBottomRect != bottomRect) {
                    currentBottomRect = bottomRect;
                    window->update(bottomRect);
                }

                //Fill top toolbar area with gradient
                drawRebarBackground(topRect, painter);
                //Fill bottom toolbar area with gradient
                drawRebarBackground(bottomRect, painter);
            }
            break;
            
        } //fall through
    default: 
        if (vista)
            QWindowsVistaStyle::drawPrimitive(element, option, painter, widget);
        else
            QWindowsXPStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void ExplorerStyle::drawControl(ControlElement element, const QStyleOption *option,
                                  QPainter *painter, const QWidget *widget) const
{
    QColor shadow = option->palette.dark().color();
    shadow.setAlpha(120);
               
    switch (element) {
    case CE_DockWidgetTitle:
        if (isAppThemed()) {
            if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
                painter->save();
                QStyleOptionDockWidget adjustedOpt = *dwOpt;
                QRect rect = option->rect.adjusted(0, 1, -1, -2);
                adjustedOpt.palette.setBrush(QPalette::All, QPalette::Dark, Qt::transparent);
                painter->save();
                painter->setClipRect(rect);
                drawRebarBackground(rect.adjusted(0, 0, 1, 1), painter);
                painter->restore();
                painter->setPen(shadow);
                painter->drawRect(rect);
                QWindowsXPStyle::drawControl(element, &adjustedOpt, painter, widget);
                painter->restore();
            }
            break;
        } //fall through

    case CE_MenuBarItem:
        if (isAppThemed()) {
            if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
                QStyleOptionMenuItem adjustedItem = *mbi;
                adjustedItem.palette.setBrush(QPalette::All, QPalette::Dark, shadow);
                adjustedItem.palette.setBrush(QPalette::All, QPalette::Button, Qt::NoBrush);
                QWindowsXPStyle::drawControl(element, &adjustedItem, painter, widget);
            }
            break;
        } //fall through

    case CE_MenuBarEmptyArea:
        if (isAppThemed()) {
            if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
                QStyleOptionMenuItem adjustedItem = *mbi;
                adjustedItem.palette.setBrush(QPalette::All, QPalette::Dark, shadow);
                adjustedItem.palette.setBrush(QPalette::All, QPalette::Button, Qt::NoBrush);
                QWindowsXPStyle::drawControl(element, &adjustedItem, painter, widget);
            }
            break;
        } //fall through

    case CE_ToolBar:
        if (isAppThemed()) {
            if (const QStyleOptionToolBar* toolbar = qstyleoption_cast<const QStyleOptionToolBar*>(option)) {
                QStyleOptionToolBar adjustedToolBar = *toolbar;
                adjustedToolBar.palette.setBrush(QPalette::All, QPalette::Dark, shadow);
                QWindowsXPStyle::drawControl(element, &adjustedToolBar, painter, widget);
            }
        break;
        } //fall through

    default:
        if (vista)
            QWindowsVistaStyle::drawControl(element, option, painter, widget);
        else
            QWindowsXPStyle::drawControl(element, option, painter, widget);
        break;
    }
}

void ExplorerStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                         QPainter *painter, const QWidget *widget) const
{
    if (vista)
        QWindowsVistaStyle::drawComplexControl(control, option, painter, widget);
    else
        QWindowsXPStyle::drawComplexControl(control, option, painter, widget);
}

QSize ExplorerStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                        const QSize &size, const QWidget *widget) const
{
    if (vista)
        return QWindowsVistaStyle::sizeFromContents(type, option, size, widget);
    else
        return QWindowsXPStyle::sizeFromContents(type, option, size, widget);
}

QRect ExplorerStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    if (vista)
        return QWindowsVistaStyle::subElementRect(element, option, widget);
    else
        return QWindowsXPStyle::subElementRect(element, option, widget);
}

QRect ExplorerStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                                      SubControl subControl, const QWidget *widget) const
{
    if (vista)
        return QWindowsVistaStyle::subControlRect(control, option, subControl, widget);
    else
        return QWindowsXPStyle::subControlRect(control, option, subControl, widget);
}

QStyle::SubControl ExplorerStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                                          const QPoint &pos, const QWidget *widget) const
{
    if (vista)
        return QWindowsVistaStyle::hitTestComplexControl(control, option, pos, widget);
    else
        return QWindowsXPStyle::hitTestComplexControl(control, option, pos, widget);
}

int ExplorerStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    if (vista)
        return QWindowsVistaStyle::pixelMetric(metric, option, widget);
    else
        return QWindowsXPStyle::pixelMetric(metric, option, widget);
}

QPalette ExplorerStyle::standardPalette()
{
    if (vista)
        return QWindowsVistaStyle::standardPalette();
    else
        return QWindowsXPStyle::standardPalette();
}    

void ExplorerStyle::polish(QApplication *app)
{
    if (vista)
        QWindowsVistaStyle::polish(app);
    else
        QWindowsXPStyle::polish(app);
}

void ExplorerStyle::unpolish(QApplication *app)
{
    if (vista)
        QWindowsVistaStyle::unpolish(app);
    else
        QWindowsXPStyle::unpolish(app);
}

void ExplorerStyle::polish(QWidget *widget)
{
    if (QMainWindow *window = qobject_cast<QMainWindow*>(widget))
        window->setAttribute(Qt::WA_StyledBackground, true);
    if (vista)
        QWindowsVistaStyle::polish(widget);
    else
        QWindowsXPStyle::polish(widget);
}

void ExplorerStyle::unpolish(QWidget *widget)
{
    if (vista)
        QWindowsVistaStyle::unpolish(widget);
    else
        QWindowsXPStyle::unpolish(widget);
}

void ExplorerStyle::polish(QPalette pal)
{
    if (vista)
        QWindowsVistaStyle::polish(pal);
    else
        QWindowsXPStyle::polish(pal);
}

QPixmap ExplorerStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                                      const QWidget *widget) const
{
    if (vista)
        return QWindowsVistaStyle::standardPixmap(standardPixmap, opt, widget);
    else
        return QWindowsXPStyle::standardPixmap(standardPixmap, opt, widget);
}
