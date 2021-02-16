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

#ifndef LIVEVIEWAREA_H
#define LIVEVIEWAREA_H

#include <QQuickView>

class LiveViewLayout;
class DVRCamera;
class DVRServerRepository;

class LiveViewArea : public QQuickView
{
    Q_OBJECT

public:
    explicit LiveViewArea(DVRServerRepository *serverRepository, QWindow *parent = 0);
    virtual ~LiveViewArea();

    LiveViewLayout *layout() const { return m_layout; }

    QSize sizeHint() const;

    //bool isHardwareAccelerated() const;

public slots:
    void addCamera(DVRCamera *camera);
    void updateGeometry() { m_sizeHint = QSize(); QQuickView::updateGeometry(); }

    //void settingsChanged();

signals:
    void forwardKey(QKeyEvent *event);

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

private slots:
    //void setViewportHack();

private:
    LiveViewLayout *m_layout;
    mutable QSize m_sizeHint;
};

#endif // LIVEVIEWAREA_H
