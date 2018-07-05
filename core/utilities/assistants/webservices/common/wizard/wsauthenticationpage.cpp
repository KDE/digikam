/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-02
 * Description : a widget to display authentication dialog with web 
 *               service server.
 *
 * Copyright (C) 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

namespace Digikam
{

#ifdef HAVE_QWEBENGINE
    

WSAuthenticationPage::WSAuthenticationPage(QObject* const parent, QWebEngineProfile* profile, QString callbackUrl)
  : QWebEnginePage(profile, parent),
    m_callbackUrl(callbackUrl)
{
}

WSAuthenticationPage::~WSAuthenticationPage()
{
}

bool WSAuthenticationPage::acceptNavigationRequest(const QUrl& url, QWebEnginePage::NavigationType type, bool)
{
    QString urlString = url.toString();
    if(m_callbackUrl.length() > 0 &&
       urlString.length() >= m_callbackUrl.length() &&
       urlString.left(m_callbackUrl.length()) == m_callbackUrl)
    {
        emit callbackCatched(urlString);
        return false;
    }

    return true;
}

void WSAuthenticationPage::setCallbackUrl(const QString& url)
{
    m_callbackUrl = url;
}

// ----------------------------------------------------------------------------

WSAuthenticationPageView::WSAuthenticationPageView(QWidget* const parent,
                                                   WSAuthentication* const wsAuth,
                                                   QString callbackUrl)
    : QWebEngineView(parent),
      m_WSAuthentication(wsAuth),
      m_authenticationCompleted(false)
      
#else // #ifdef HAVE_QWEBENGINE
      
WSAuthenticationPageView::WSAuthenticationPageView(QWidget* const parent, 
                                                   WSAuthentication* const wsAuth,
                                                   QString callbackUrl)
    : QWebView(parent),
      m_WSAuthentication(wsAuth),
      m_authenticationCompleted(false)
      
#endif // #ifdef HAVE_QWEBENGINE
      
{
    
#ifdef HAVE_QWEBENGINE
    
    adjustSize();
    setMinimumSize(QSize(850,800));
    
    WSAuthenticationPage* const wpage = new WSAuthenticationPage(this, new QWebEngineProfile, callbackUrl);
    setPage(wpage);
    
    connect(wpage, SIGNAL(callbackCatched(QString)),
            this, SLOT(slotCallbackCatched(QString)));

#else
    
    /*
     * (Trung) TODO: backport with QWebkit for setting page()
     *               backport with QWebkit for signal callbackCatched(const QUrl&)
     */ 
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    setRenderHint(QPainter::TextAntialiasing);
    
#endif
    
    
    connect(m_WSAuthentication, SIGNAL(signalOpenBrowser(const QUrl&)),
            this, SLOT(slotOpenBrowser(const QUrl&)));
    connect(m_WSAuthentication, SIGNAL(signalCloseBrowser()),
            this, SLOT(slotCloseBrowser()));
    
    hide();
}

WSAuthenticationPageView::~WSAuthenticationPageView()
{
}

bool WSAuthenticationPageView::authenticationComplete() const
{
    return m_authenticationCompleted;
}

QMap<QString, QString> WSAuthenticationPageView::parseResponseToken(const QString& rep)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseToken: " << rep;
    
    QMap<QString, QString> result;
    
    QStringList listArgs = rep.split(QLatin1Char('&'));
    foreach(const QString& arg, listArgs)
    {
        QStringList pair = arg.split(QLatin1Char('='));
        result.insert(pair.takeFirst(), pair.takeLast());
    }
    
    return result;
}

void WSAuthenticationPageView::slotOpenBrowser(const QUrl& url)
{
#ifdef HAVE_QWEBENGINE
    
    WSAuthenticationPage* page = dynamic_cast<WSAuthenticationPage*>(this->page());
    page->profile()->cookieStore()->deleteAllCookies();
    page->setUrl(url);
    show();

#else

    //(Trung) TODO: backport with QWebkit 

#endif
}

//(Trung) Here we hide browser, instead of closing it completely
void WSAuthenticationPageView::slotCloseBrowser()
{
#ifdef HAVE_QWEBENGINE

    close();

#else

    //(Trung) TODO: backport with QWebkit 

#endif 
    
    m_authenticationCompleted = true;
    
    emit signalAuthenticationComplete();
}

void WSAuthenticationPageView::slotCallbackCatched(const QString& callbackUrl)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotCallbackCatched url: " << callbackUrl;
    
    QUrl url(callbackUrl);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url fragment: " << url.fragment();
    
    QMap<QString, QString> rep = parseResponseToken(url.fragment());
    emit m_WSAuthentication->signalResponseTokenReceived(rep);

#ifdef HAVE_QWEBENGINE
    
#else

    //(Trung) TODO: backport with QWebkit 

#endif
}

// ----------------------------------------------------------------------------

class WSAuthenticationWizard::Private 
{
public:
    
    explicit Private(QWizard* const dialog)
      : wizard(0),
        iface(0),
        wsAuth(0),
        wsAuthView(0),
        vbox(0),
        text(0)
    {
        wizard = dynamic_cast<WSWizard*>(dialog);
        
        if(wizard)
        {
            iface = wizard->iface();
        }
    }
    
    WSWizard*                   wizard;
    DInfoInterface*             iface;
    WSAuthentication*           wsAuth;
    WSAuthenticationPageView*   wsAuthView;
    DVBox*                      vbox;
    QLabel*                     text;
};

WSAuthenticationWizard::WSAuthenticationWizard(QWizard* const dialog, const QString& title, 
                                               QString callbackUrl)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    d->vbox    = new DVBox(this);
    
    d->text    = new QLabel(d->vbox);
    d->text->setWordWrap(true);
    d->text->setOpenExternalLinks(true);
    
    d->wsAuth       = new WSAuthentication(d->vbox, d->iface); 
    d->wsAuthView   = new WSAuthenticationPageView(d->vbox, d->wsAuth, callbackUrl);
    connect(d->wsAuthView, SIGNAL(signalAuthenticationComplete()),
            this, SLOT(slotAuthenticationComplete()));
    
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
    d->wsAuth->createTalker(d->wizard->settings()->webService);
    
    if(d->wizard->settings()->webService == WSSettings::WebService::FACEBOOK)
    {
        WSAuthenticationPage* wsAuthPage = dynamic_cast<WSAuthenticationPage*>(d->wsAuthView->page());
        wsAuthPage->setCallbackUrl(QLatin1String("https://www.facebook.com/connect/login_success.html"));
    }
    
    d->wsAuth->reauthenticate();
}

bool WSAuthenticationWizard::validatePage()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "validatePage";
    
    return true;
}

void WSAuthenticationWizard::slotAuthenticationComplete()
{
    d->text->setText(i18n("<qt>"
                          "<p><h2><b>Authentication done!</b></h2></p>"
                          "</qt>"));
    emit completeChanged();
}

} // namespace Digikam
