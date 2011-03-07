#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

class DVRServersView;
class LiveViewWindow;
class EventsView;
class QSplitter;
class DVRServer;
class QSslError;
class QSslConfiguration;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    LiveViewWindow *liveView() const { return m_liveView; }

    void updateTrayIcon();

public slots:
    void showOptionsDialog();
    void showEventsWindow();

    void addServer();
    void openServerConfig();
    void editCurrentServer();
    void refreshServerDevices();

    void openDocumentation();
    void openSupport();
    void openIdeas();
    void openAbout();

    void showFront();

private slots:
    void showServersMenu();
    void sslConfirmRequired(DVRServer *server, const QList<QSslError> &errors, const QSslConfiguration &config);
    void trayActivated(QSystemTrayIcon::ActivationReason);
    void queryLivePaused();
    void liveViewLayoutChanged(const QString &layout);

    void eventsContextMenu(const QPoint &pos);

signals:
    void closing();

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    DVRServersView *m_sourcesList;
    EventsView *m_eventsView;
    LiveViewWindow *m_liveView;
    QSplitter *m_centerSplit;
    QAction *menuServerName;
    QSystemTrayIcon *m_trayIcon;

    void createMenu();

    QWidget *createSourcesList();
    QWidget *createServerBox();

    QWidget *createCameraControls();
    QWidget *createRecentEvents();
};

#endif // MAINWINDOW_H
