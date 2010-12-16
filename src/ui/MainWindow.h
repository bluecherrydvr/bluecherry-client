#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

class DVRServersView;
class CameraAreaWidget;
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

    CameraAreaWidget *cameraArea() const { return m_cameraArea; }

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

signals:
    void closing();

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    DVRServersView *m_sourcesList;
    CameraAreaWidget *m_cameraArea;
    EventsView *m_eventsView;
    QSplitter *m_centerSplit;
    QAction *menuServerName;
    QSystemTrayIcon *m_trayIcon;

    void createMenu();

    QWidget *createSourcesList();
    QWidget *createServerBox();

    QWidget *createCameraArea();
    QWidget *createCameraControls();
    QWidget *createRecentEvents();
};

#endif // MAINWINDOW_H
