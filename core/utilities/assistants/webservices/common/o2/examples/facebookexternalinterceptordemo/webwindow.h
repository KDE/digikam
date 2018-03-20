
#pragma once

#include <QDialog>
#include <QWebEngineView>

namespace Ui {
class WebWindow;
}

class WebWindow : public QDialog
{
    Q_OBJECT

public:
    explicit WebWindow(QSize inWindowSize, QUrl inLoginURL, QString inRedirectURLString, bool inAutoclose);
    ~WebWindow();

    QString resultStr;
    
    QWebEnginePage *GetWebEnginePage() { return mWebEnginePage; }

protected:
    void closeEvent(QCloseEvent *);

signals:
    void windowClosed();
    void callbackCalled(const QString &inURLString);

private slots:
    void onCallbackCatched(const QString &inURLString);
    void onCallbackCatchedSafe();

private:
    Ui::WebWindow *ui;
	
	// Profile to not store cookies and cache data to the disk
    QWebEngineProfile *mWebEngineProfile;
	
    // Webengine page using the Off-the-record profile
    QWebEnginePage *mWebEnginePage;
	
    // Webview
    QWebEngineView* mWebView;
    
    QString mCatchedOAuthURL;
    bool mAutoclose;
};

