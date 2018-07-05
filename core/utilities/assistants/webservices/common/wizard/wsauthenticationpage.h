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

#ifndef DIGIKAM_WS_AUTHENTICATION_PAGE_H
#define DIGIKAM_WS_AUTHENTICATION_PAGE_H

#include "digikam_config.h"

// Qt includes

#include <QByteArray>
#include <QString>
#include <QUrl>
#include <QMap>

#ifdef HAVE_QWEBENGINE
#   include <QWebEngineView>
#   include <QWebEnginePage>
#   include <QWebEngineProfile>
#   include <QWebEngineCookieStore>
#else
#   include <QWebView>
#endif

// Local includes

#include "digikam_export.h"
#include "dwizardpage.h"
#include "dinfointerface.h"
#include "wsauthentication.h"

namespace Digikam
{
    
#ifdef HAVE_QWEBENGINE
class WSAuthenticationPage : public QWebEnginePage
{
    Q_OBJECT

public:

    explicit WSAuthenticationPage(QObject* const parent, QWebEngineProfile* profile, QString callbackUrl);
    virtual ~WSAuthenticationPage();

    bool acceptNavigationRequest(const QUrl&, QWebEnginePage::NavigationType, bool);

    void setCallbackUrl(const QString& url);
    
Q_SIGNALS:

    void callbackCatched(const QString&);
    
private:
    
    QString m_callbackUrl;
};

// -------------------------------------------------------------------

class WSAuthenticationPageView : public QWebEngineView
#else
class WSAuthenticationPageView : public QWebView
#endif
{
    Q_OBJECT

public:

    explicit WSAuthenticationPageView(QWidget* const parent, 
                                      WSAuthentication* const wsAuth,
                                      QString callbackUrl);
    ~WSAuthenticationPageView();
    
    bool authenticationComplete() const;
    
private:
    
    QMap<QString, QString> parseResponseToken(const QString& rep);

Q_SIGNALS:
    
    void signalAuthenticationComplete();
    
private Q_SLOTS:
    
    void slotCallbackCatched(const QString& callbackUrl); 
    void slotOpenBrowser(const QUrl& url);
    void slotCloseBrowser();
    
private:
    
    WSAuthentication*   m_WSAuthentication;
    bool                m_authenticationCompleted;
}; 

// -------------------------------------------------------------------

class WSAuthenticationWizard : public DWizardPage
{
    Q_OBJECT
    
public:
    
    explicit WSAuthenticationWizard(QWizard* const dialog, const QString& title,
                                    QString callbackUrl = QString("http://127.1.1.0:8000/"));
    ~WSAuthenticationWizard();
    
    bool isComplete() const;
    
    void initializePage();
    bool validatePage();

public Q_SLOTS:
    
    void slotAuthenticationComplete();

private:
    
    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_WS_AUTHENTICATION_PAGE_H
