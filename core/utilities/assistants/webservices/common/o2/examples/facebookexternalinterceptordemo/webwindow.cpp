#include "webwindow.h"
#include "ui_webwindow.h"
#include "webenginepage.h"

#include <QSizePolicy>
#include <QThread>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QWebEngineSettings>
#include <QUrlQuery>
#include <QTimer>

WebWindow::WebWindow(QSize inWindowSize, QUrl inLoginURL, QString inRedirectURLString, bool inAutoclose) :
    QDialog(Q_NULLPTR),
    ui(new Ui::WebWindow),
    mAutoclose(inAutoclose)
{
    ui->setupUi(this);
    setFocus();
    setFixedSize(inWindowSize);

    mWebView = new QWebEngineView(this);
    mWebView->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
	
    mWebEngineProfile = new QWebEngineProfile();
	
    mWebEnginePage = new WebEnginePage(mWebEngineProfile, inRedirectURLString);
    QObject::connect(mWebEnginePage, SIGNAL(callbackCatched(const QString &)), this, SLOT(onCallbackCatched(const QString &)));
	
	mWebView->setPage(mWebEnginePage);
	
    QRect rect_map = QRect(1, 1, width() - 2, height() - 2);
    mWebView->setGeometry(rect_map);
    mWebView->setObjectName("webView");
	
	mWebView->page()->profile()->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
	if(!inLoginURL.isEmpty())
	{
    	mWebView->page()->profile()->cookieStore()->deleteAllCookies();
    }
	
	// Load the URL
    mWebView->setUrl(inLoginURL);
}

WebWindow::~WebWindow()
{
    mWebView->setPage(Q_NULLPTR);
    delete mWebEnginePage;

    delete mWebEngineProfile;

    mWebView->close();
    delete mWebView;
	
    delete ui;
}

void WebWindow::closeEvent(QCloseEvent *)
{
	emit (windowClosed());
}

void WebWindow::onCallbackCatched(const QString &inURLString)
{
	mCatchedOAuthURL = inURLString;
	QTimer::singleShot(100, this, SLOT(onCallbackCatchedSafe()));
}

void WebWindow::onCallbackCatchedSafe()
{
	if(mAutoclose)
	{
		QUrl getTokenUrl(mCatchedOAuthURL);
		QUrlQuery query(getTokenUrl);
		QList< QPair<QString, QString> > tokens = query.queryItems();
		
		QMultiMap<QString, QString> queryParams;
		QPair<QString, QString> tokenPair;
		foreach (tokenPair, tokens) {
			QString key = QUrl::fromPercentEncoding(QByteArray().append(tokenPair.first.trimmed().toLatin1()));
			QString value = QUrl::fromPercentEncoding(QByteArray().append(tokenPair.second.trimmed().toLatin1()));
			queryParams.insert(key, value);
		}
		
		resultStr = queryParams.value("code", "");
		close();
	}
	else
	{
		emit callbackCalled(mCatchedOAuthURL);
	}
}


