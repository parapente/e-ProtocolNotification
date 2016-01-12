#ifndef EPN_DIALOG_H
#define EPN_DIALOG_H

#include <QDialog>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>
#include <QResource>
#include "popup.h"
#include "version.h"

namespace Ui {
class EPN_Dialog;
}

class EPN_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit EPN_Dialog(QWidget *parent = 0);
    ~EPN_Dialog();

public slots:
    void getUpdate(void);

private slots:
    void replyFinished(QNetworkReply*);
    void saveSettings(void);
    void quit(void);

private:
    Ui::EPN_Dialog *ui;
    QNetworkAccessManager *networkManager;
    Popup *popup;
    int timeout;
    QUrl url;
    QSettings *settings;
    QString username, version;
    QTimer *timer;
    QAction *openAction, *exitAction;
    QResource resource;
    bool dontshowagain;
    int lowPriorityMsg;
    QSystemTrayIcon *trayIcon;
};

#endif // EPN_DIALOG_H
