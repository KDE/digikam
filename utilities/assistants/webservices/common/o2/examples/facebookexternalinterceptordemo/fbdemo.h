#ifndef FBDEMO_H
#define FBDEMO_H

#include <QObject>

#include "o2facebook.h"
#include "webwindow.h"

class FBDemo : public QObject
{
    Q_OBJECT

public:
    explicit FBDemo(QObject *parent = 0);

signals:
    void extraTokensReady(const QVariantMap &extraTokens);
    void linkingFailed();
    void linkingSucceeded();

public slots:
    void doOAuth(O2::GrantFlow grantFlowType);

private slots:
    void onLinkedChanged();
    void onLinkingSucceeded();
    void onOpenBrowser(const QUrl &url);
    void onAuthWindowCallbackCalled(const QString &inURLString);
    void onAuthWindowClosed();
    void onCloseBrowser();
    void onFinished();

private:
    O2Facebook *o2Facebook_;
    WebWindow* authDialog;
};

#endif // FBDEMO_H
