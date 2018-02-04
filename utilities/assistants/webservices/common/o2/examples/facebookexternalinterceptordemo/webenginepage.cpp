
#include "webenginepage.h"
#include "webwindow.h"

WebEnginePage::~WebEnginePage()
{
    for(WebWindow *createdWindow : mCreatedWindows)
	{
		createdWindow->close();
		createdWindow->deleteLater();
	}
	
	mCreatedWindows.clear();
}

bool WebEnginePage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType /*type*/, bool /*isMainFrame*/)
{
	QString urlString = url.toString();
	if(mRedirectURLString.length() > 0 && 
		(urlString.length() >= mRedirectURLString.length()) &&
	 	(urlString.left(mRedirectURLString.length()) == mRedirectURLString))
	{
		emit callbackCatched(url.toString());
		return false;
	}

	return true;
}

QWebEnginePage *WebEnginePage::createWindow(WebWindowType type)
{
	switch(type)
	{
		case QWebEnginePage::WebBrowserTab:
		case QWebEnginePage::WebBrowserBackgroundTab:
		case QWebEnginePage::WebBrowserWindow:
		case QWebEnginePage::WebDialog:
		{
            WebWindow *webViewDialog = new WebWindow(QSize(600, 500), QUrl(), mRedirectURLString, false);
            QObject::connect(webViewDialog, SIGNAL(callbackCalled(const QString &)), this, SLOT(onAuthWindowCallbackCalled(const QString &)));
            QObject::connect(webViewDialog, SIGNAL(windowClosed()), this, SLOT(onCreatedWindowClosed()));
            QObject::connect(webViewDialog->GetWebEnginePage(), SIGNAL(windowCloseRequested()), this, SLOT(onWindowCloseRequested()));
			mCreatedWindows.push_back(webViewDialog);
			webViewDialog->open();
			return webViewDialog->GetWebEnginePage();
		}
	}
	
	return NULL;
}

void WebEnginePage::onAuthWindowCallbackCalled(const QString &inURLString)
{
	emit callbackCatched(inURLString);
}

void WebEnginePage::onCreatedWindowClosed()
{
    WebWindow *createdWindow = qobject_cast<WebWindow *>(sender());
    if (createdWindow != Q_NULLPTR)
    {
        std::vector<WebWindow*>::iterator iterator = std::find(mCreatedWindows.begin(), mCreatedWindows.end(), createdWindow);
    	if(iterator != mCreatedWindows.end())
    	{
    		mCreatedWindows.erase(iterator);
			createdWindow->close();
        	createdWindow->deleteLater();
		}
    }
}

void WebEnginePage::onWindowCloseRequested()
{
	QWebEnginePage *webEngine = qobject_cast<QWebEnginePage *>(sender());
    if (webEngine != Q_NULLPTR)
    {
        for(WebWindow *createdWindow : mCreatedWindows)
    	{
    		if(createdWindow->GetWebEnginePage() == webEngine)
    		{
                std::vector<WebWindow*>::iterator iterator = std::find(mCreatedWindows.begin(), mCreatedWindows.end(), createdWindow);
				if(iterator != mCreatedWindows.end())
				{
					mCreatedWindows.erase(iterator);
					createdWindow->close();
					createdWindow->deleteLater();
				}
    			
    			break;
			}
    	}
    }
}
