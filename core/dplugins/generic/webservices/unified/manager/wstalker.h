/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-08
 * Description : Base class for web service talkers.
 *
 * Copyright (C) 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_WS_TALKER_H
#define DIGIKAM_WS_TALKER_H

// Qt includes

#include <QtGlobal>
#include <QList>
#include <QPair>
#include <QString>
#include <QUrl>
#include <QWidget>
#include <QSettings>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QMap>

// Local includes

#include "o0settingsstore.h"
#include "wsitem.h"
#include "wswizard.h"

namespace Digikam
{

class WSTalker : public QObject
{
    Q_OBJECT

public:

    enum State
    {
        DEFAULT = 0,
        GETUSER,
        LISTALBUMS,
        CREATEALBUM,
        ADDPHOTO
    };

public:

    explicit WSTalker(QWidget* const parent);
    ~WSTalker();

    /*
     * Get ID of an existent user account saved when he logged in before, 
     * knowing user name.
     */
    QString getUserID(const QString& userName);

    /*
     * Link user account (login).
     */
    virtual void link();

    /*
     * Unlink user account (logout).
     */
    virtual void unlink();

    /*
     * Return true if account is linked.
     */
    virtual bool linked() const;

    /*
     * This method load account that user chooses to login. If it exists and doesn't expire yet,
     * then obtain the saved token and pass the authentication process. Otherwise, relogin.
     */
    virtual void authenticate();

    /*
     * Force user to login on web service login page.
     */
    void reauthenticate();

    /*
     * Abort any network request realizing at the moment.
     */
    void cancel();

protected:

    /*
     * Return a map of all information stored in the previous login of userName account.
     */
    QMap<QString, QVariant> getUserAccountInfo(const QString& userName);

    /*
     * Save all necessary information of user account to disk. That information will be retrieved
     * by getUserAccountInfo(userName) when needed.
     */
    void saveUserAccount(const QString& userName,
                         const QString& userID,
                         long long int expire,
                         const QString& accessToken,
                         const QString& refreshToken = QString());

    /*
     * Remove all information of user account that was stored by saveUserAccount(...)
     * TODO: this method should be called when user uninstalls digiKam.
     */
    void removeUserAccount(const QString& userName);

    /*
     * Save as removeUserAccount(userName), but for all accounts.
     */
    void removeAllAccounts();

    /*
     * A wrapper method of getUserAccountInfo(userName), but perform further verification
     * on account's validation and further operation in case that account is expired.
     */
    bool loadUserAccount(const QString& userName);

    /*
     * This method can be (and must be) reimplemented in derived class. Indeed, it will hard code
     * (at runtime) O2's settings (i.e accessToken, refreshToken, expired date and value of linked state).
     * It forces O2 to link to another account according to user's selection. Otherwise, O2 will
     * "remember" account from previous login and always link to that account, if an obligated reauthenticate
     * (unlink and then link) is not realized.
     */
    virtual void resetTalker(const QString& expire, const QString& accessToken, const QString& refreshToken);

    /*
     * Sort list of albums by ascending order of titles.
     */
    virtual void sortAlbumsList(QList<WSAlbum>& albumsList);

    /*
     * These methods are reimplemented in derived class and used to parse response of network requests
     * for user's information or APIs of web service. They will be called asynchronously when responses
     * for net request are received.
     */
    virtual void parseResponseGetLoggedInUser(const QByteArray& data);
    virtual void parseResponseListAlbums(const QByteArray& data);
    virtual void parseResponseCreateAlbum(const QByteArray& data);
    virtual void parseResponseAddPhoto(const QByteArray& data);

    /*
     * This method is called when authentication is complete. It should be reimplemented in derived class
     * and call saveUserAccount(...) inside. Here, we implement a minimised version so that derived class
     * can call it if needed.
     */
    virtual void authenticationDone(int errCode, const QString& errMsg);

public:

    /*
     * These methods are reimplemented in derived class, and will be used to make network requests
     * for user's information or APIs of web service.
     */
    virtual void getLoggedInUser();
    virtual void listAlbums(long long userID = 0);
    virtual void createNewAlbum();
    virtual void addPhoto(const QString& imgPath, const QString& albumID, const QString& caption);

Q_SIGNALS:

    void signalBusy(bool val);
    void signalOpenBrowser(const QUrl& url);
    void signalCloseBrowser();
    void signalAuthenticationComplete(bool);
    void signalCreateAlbumDone(int errCode, const QString& errMsg, const QString& newAlbumId);
    void signalListAlbumsDone(int errCode, const QString& errMsg, const QList <WSAlbum>& albumsList);
    void signalAddPhotoDone(int errCode, const QString& errMsg);

protected Q_SLOTS:

    /*
     * Slots for signals from O2 authentication flow
     */
    void slotFinished(QNetworkReply* reply);
    void slotOpenBrowser(const QUrl& url);
    void slotCloseBrowser();
    virtual void slotLinkingFailed();
    virtual void slotLinkingSucceeded();

    /*
     * This is a particular slot, only used in case that digiKam will intercept O2 authentication flow,
     * catch all navigation from web service, and the final url whose fragment contains accessToken 
     * and other necessary information. digiKam then parses the response to get accessToken and join back to 
     * O2's authentication flow by calling this method.
     *
     * Facebook is a web service where this approach is used, because the callback url is not http://127.0.0.1/
     */
    virtual void slotResponseTokenReceived(const QMap<QString, QString>& rep);

protected:

    QNetworkAccessManager*  m_netMngr;
    QNetworkReply*          m_reply;

    State                   m_state;

    QSettings*              m_settings;
    O0SettingsStore*        m_store;

    QString                 m_userName;

    WSWizard*               m_wizard;
};

} // namespace Digikam

#endif // DIGIKAM_WS_TALKER_H
