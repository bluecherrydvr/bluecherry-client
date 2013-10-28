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

#ifndef CRASHREPORTDIALOG_H
#define CRASHREPORTDIALOG_H

#include <QDialog>
#include <QFile>

class QCheckBox;
class QCommandLinkButton;
class QLabel;
class QLineEdit;
class QNetworkReply;
class QProgressBar;
class QStackedLayout;

class CrashReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CrashReportDialog(const QString &dumpFile, QWidget *parent = 0);
    virtual ~CrashReportDialog();

private slots:
    void uploadAndRestart();
    void uploadAndExit();

    void setUploadProgress(qint64 done);
    void uploadFinished();

    void reportContextMenu(const QPoint &point);
    void saveCrashReport();

protected:
	virtual void changeEvent(QEvent *event);
    virtual void showEvent(QShowEvent *event);

private:
    QFile m_dumpFile;
    QNetworkReply *m_uploadReply;

    QLineEdit *m_emailInput;
    QCheckBox *m_allowReport;
    QProgressBar *m_uploadProgress;
    QStackedLayout *m_stackLayout;
    int m_finalResult;

	QLabel *m_emailLabel;
	QLabel *m_emailInformationLabel;
	QLabel *m_titleLabel;
	QLabel *m_progressLabel;

	QCommandLinkButton *m_restartBtn;
	QCommandLinkButton *m_exitBtn;


    void sendReport();
    void deleteDump();

	void retranslateUI();
};

#endif // CRASHREPORTDIALOG_H
