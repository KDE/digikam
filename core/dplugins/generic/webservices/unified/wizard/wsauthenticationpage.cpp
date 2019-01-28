/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-02
 * Description : embedded web browser for web service authentication
 *
 * Copyright (C) 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "wsauthenticationpage.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QWidget>
#include <QApplication>
#include <QStandardPaths>
#include <QString>
#include <QLabel>
#include <QStringList>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "dlayoutbox.h"
#include "wswizard.h"
#include "wssettings.h"

namespace DigikamGenericUnifiedPlugin
{

#ifdef HAVE_QWEBENGINE

WSAuthenticationPage::WSAuthenticationPage(QObject* const parent, QWebEngineProfile* profile, const QString& callbackUrl)
    : QWebEnginePage(profile, parent),
      m_callbackUrl(callbackUrl)
{
}

#else

WSAuthenticationPage::WSAuthenticationPage(QObject* const parent, const QString& callbackUrl)
    : QWebPage(parent),
      m_callbackUrl(callbackUrl)
{
    connect(mainFrame(), SIGNAL(urlChanged(QUrl)),
            this, SLOT(slotUrlChanged(QUrl)));
}

#endif // #ifdef HAVE_QWEBENGINE

WSAuthenticationPage::~WSAuthenticationPage()
{
}

void WSAuthenticationPage::setCallbackUrl(const QString& url)
{
    m_callbackUrl = url;
}

#ifdef HAVE_QWEBENGINE

bool WSAuthenticationPage::acceptNavigationRequest(const QUrl& url, QWebEnginePage::NavigationType /*type*/, bool /*isMainFrame*/)

#else

bool WSAuthenticationPage::slotUrlChanged(const QUrl& url)

#endif // #ifdef HAVE_QWEBENGINE

{
    QString urlString = url.toString();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "urlString: " << urlString;

    /*
     * Condition to verify that the url loaded on page is the one containing access token
     */
    if (m_callbackUrl.length() > 0                   &&
        urlString.length() >= m_callbackUrl.length() &&
        urlString.left(m_callbackUrl.length()) == m_callbackUrl)
    {
        emit callbackCatched(urlString);
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------

#ifdef HAVE_QWEBENGINE

WSAuthenticationPageView::WSAuthenticationPageView(QWidget* const parent,
                                                   WSAuthentication* const wsAuth,
                                                   const QString& callbackUrl)
    : QWebEngineView(parent),
      m_WSAuthentication(wsAuth)

#else

WSAuthenticationPageView::WSAuthenticationPageView(QWidget* const parent,
                                                   WSAuthentication* const wsAuth,
                                                   const QString& callbackUrl)
    : QWebView(parent),
      m_WSAuthentication(wsAuth)

#endif // #ifdef HAVE_QWEBENGINE

{
    adjustSize();
    setMinimumSize(QSize(850,800));

#ifdef HAVE_QWEBENGINE

    WSAuthenticationPage* const wpage = new WSAuthenticationPage(this, new QWebEngineProfile, callbackUrl);

#else

    WSAuthenticationPage* const wpage = new WSAuthenticationPage(this, callbackUrl);

#endif // #ifdef HAVE_QWEBENGINE

    setPage(wpage);

    connect(wpage, SIGNAL(callbackCatched(QString)),
            this, SLOT(slotCallbackCatched(QString)));

    connect(m_WSAuthentication, SIGNAL(signalOpenBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));

    connect(m_WSAuthentication, SIGNAL(signalCloseBrowser()),
            this, SLOT(slotCloseBrowser()));

    /*
     * Here we hide the web browser immediately after creation.
     * If user has to login, we will show the browser again. Otherwise,
     * we will keep it hiding to improve page's looking.
     */
    hide();
}

WSAuthenticationPageView::~WSAuthenticationPageView()
{
}

bool WSAuthenticationPageView::authenticationComplete() const
{
    return m_WSAuthentication->authenticated();
}

QMap<QString, QString> WSAuthenticationPageView::parseUrlFragment(const QString& urlFragment)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseUrlFragment: " << urlFragment;

    QMap<QString, QString> result;
    QStringList listArgs = urlFragment.split(QLatin1Char('&'));

    foreach (const QString& arg, listArgs)
    {
        QStringList pair = arg.split(QLatin1Char('='));
        result.insert(pair.first(), pair.last());
    }

    return result;
}

void WSAuthenticationPageView::slotOpenBrowser(const QUrl& url)
{
    WSAuthenticationPage* const page = dynamic_cast<WSAuthenticationPage*>(this->page());

#ifdef HAVE_QWEBENGINE

    page->setUrl(url);

#else

    page->mainFrame()->setUrl(url);

#endif

    /*
     * Here we show the web browser again after creation, because when this slot is triggered,
     * user has to login. Therefore the login page has to be shown.
     */
    show();
}

void WSAuthenticationPageView::slotCloseBrowser()
{
    close();
}

void WSAuthenticationPageView::slotCallbackCatched(const QString& callbackUrl)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotCallbackCatched url: " << callbackUrl;

    QUrl url(callbackUrl);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url fragment: " << url.fragment();

    QMap<QString, QString> res = parseUrlFragment(url.fragment());
    emit m_WSAuthentication->signalResponseTokenReceived(res);
}

// ----------------------------------------------------------------------------

class Q_DECL_HIDDEN WSAuthenticationWizard::Private
{
public:

    explicit Private(QWizard* const dialog, const QString& callback)
      : wizard(0),
        iface(0),
        wsAuth(0),
        wsAuthView(0),
        vbox(0),
        text(0),
        callbackUrl(callback)
    {
        wizard = dynamic_cast<WSWizard*>(dialog);

        if (wizard)
        {
            iface = wizard->iface();

            if (wizard->settings()->webService == WSSettings::WebService::FACEBOOK)
            {
                callbackUrl = QLatin1String("https://www.facebook.com/connect/login_success.html");
            }
        }
    }

    WSWizard*                   wizard;
    DInfoInterface*             iface;
    WSAuthentication*           wsAuth;
    WSAuthenticationPageView*   wsAuthView;
    DVBox*                      vbox;
    QLabel*                     text;
    QString                     callbackUrl;
};

WSAuthenticationWizard::WSAuthenticationWizard(QWizard* const dialog,
                                               const QString& title,
                                               const QString& callback)
    : DWizardPage(dialog, title),
      d(new Private(dialog, callback))
{
    d->wsAuth  = d->wizard->wsAuth();

    connect(d->wsAuth, SIGNAL(signalAuthenticationComplete(bool)),
            this, SLOT(slotAuthenticationComplete(bool)));

    d->vbox    = new DVBox(this);

    d->text    = new QLabel(d->vbox);
    d->text->setWordWrap(true);
    d->text->setOpenExternalLinks(true);

    d->vbox->setStretchFactor(d->text, 1);
    d->vbox->setStretchFactor(d->wsAuthView, 4);

    setPageWidget(d->vbox);
}

WSAuthenticationWizard::~WSAuthenticationWizard()
{
    delete d;
}

bool WSAuthenticationWizard::isComplete() const
{
    return d->wsAuthView->authenticationComplete();
}

void WSAuthenticationWizard::initializePage()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "initPage WSAuthenticationWizard";

    /*
     * Init WebView of WSAuthenticationWizard every time initializePage is called.
     *
     * This guarantees an appropriate authentication page, even when user goes back to
     * intro page and choose a different account or different web service.
     */
    d->text->hide();
    d->wsAuthView                                 = new WSAuthenticationPageView(d->vbox, d->wsAuth, d->callbackUrl);
    QMap<WSSettings::WebService, QString> wsNames = WSSettings::webServiceNames();
    WSSettings::WebService ws                     = d->wizard->settings()->webService;
    d->wsAuth->createTalker(ws, wsNames[ws]);

    d->wsAuth->authenticate();
}

bool WSAuthenticationWizard::validatePage()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "validatePage";
    return true;
}

void WSAuthenticationWizard::cleanupPage()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "cleanupPage WSAuthenticationWizard";
    d->wsAuth->cancelTalker();

    delete d->wsAuthView;
    d->wsAuthView = 0;
}

void WSAuthenticationWizard::slotAuthenticationComplete(bool isLinked)
{
    d->text->show();

    if (isLinked)
    {
        d->text->setText(i18n("<qt>"
                              "<p><h1><b>Authentication done!</b></h1></p>"
                              "<p><h3>Account linking succeeded!</h3></p>"
                              "</qt>"));
    }
    else
    {
        d->text->setText(i18n("<qt>"
                              "<p><h1><b>Authentication done!</b></h1></p>"
                              "<p><h3>Account linking failed!</h3></p>"
                              "</qt>"));
    }

    emit completeChanged();
}

} // namespace DigikamGenericUnifiedPlugin
