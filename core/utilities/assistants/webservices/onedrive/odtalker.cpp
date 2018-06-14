#include <odtalker.h>

// Qt includes

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QList>
#include <QPair>
#include <QFileInfo>
#include <QWidget>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QWebEngineView>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "wstoolutils.h"
#include "odwindow.h"
#include "oditem.h"
#include "odmpform.h"
#include "previewloadthread.h"
#include "o0settingsstore.h"

namespace Digikam
{

class ODTalker::Private
{
public:

    enum State
    {
        OD_USERNAME = 0,
        OD_LISTFOLDERS,
        OD_CREATEFOLDER,
        OD_ADDPHOTO
    };

public:

    explicit Private()
    {
        clientId   = QLatin1String("83de95b7-0f35-4abf-bac1-7729ced74c01");
        clientSecret   = QLatin1String("tgjV796:)yezuRWMZSZ45+!");
        scope = QLatin1String("User.Read Files.ReadWrite");

        authUrl  = QLatin1String("https://login.live.com/oauth20_authorize.srf");
        tokenUrl = QLatin1String("https://login.live.com/oauth20_token.srf");
        redirectUrl = QLatin1String("https://login.live.com/oauth20_desktop.srf");

        state       = OD_USERNAME;
        settings    = 0;
        netMngr     = 0;
        reply       = 0;
        o2          = 0;
        accessToken = "";
    }

public:

    QString                clientId;
    QString                clientSecret;
    QString                authUrl;
    QString                tokenUrl;
    QString                scope;
    QString                redirectUrl;
    QString                accessToken;

    QWidget*               parent;
    QNetworkAccessManager* netMngr;

    QNetworkReply*         reply;

    QSettings*             settings;

    State                  state;

    QByteArray             buffer;

    DMetadata              meta;

    O2*                    o2;

    QMap<QString,QString> urlParametersMap;
};
ODTalker::ODTalker(QWidget* const parent)
    : d(new Private)
{
    d->parent  = parent;
    d->netMngr = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    d->o2      = new O2(this);

    d->o2->setClientId(d->clientId);
    d->o2->setClientSecret(d->clientSecret);
    d->o2->setRefreshTokenUrl(d->tokenUrl);
    d->o2->setRequestUrl(d->authUrl);
    d->o2->setTokenUrl(d->tokenUrl);
    d->o2->setScope(d->scope);
    d->o2->setLocalPort(5000);

    d->settings = WSToolUtils::getOauthSettings(this);
    O0SettingsStore* const store = new O0SettingsStore(d->settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(QLatin1String("Onedrive"));
    d->o2->setStore(store);

    connect(d->o2, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(this, SIGNAL(oneDriveLinkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->o2, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));
}
ODTalker::~ODTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }
    delete d;
}
void ODTalker::link()
{
    emit signalBusy(true);
    QUrl url(d->authUrl);
    QUrlQuery query(url);
    query.addQueryItem(QLatin1String("client_id"), d->clientId);
    query.addQueryItem(QLatin1String("scope"), d->scope);
    query.addQueryItem(QLatin1String("redirect_uri"), d->redirectUrl);
    query.addQueryItem(QLatin1String("response_type"), "token");
    url.setQuery(query);

    QWebEngineView *view = new QWebEngineView(d->parent);
    view->setWindowFlags(Qt::Dialog);
    view->load(url);
    view->show();

    connect(view, SIGNAL(urlChanged(QUrl)), this, SLOT(slotOpenBrowser(QUrl)));
}
void ODTalker::unLink()
{
    d->accessToken = "";
    Q_EMIT oneDriveLinkingSucceeded();
    //d->o2->unlink();
}

void ODTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Onedrive fail";
    emit signalBusy(false);
}
void ODTalker::slotLinkingSucceeded()
{
    /*if (!d->o2->linked())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Onedrive ok";
        emit signalBusy(false);
        return;
    }*/

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Onedrive ok";
    emit signalLinkingSucceeded();
}
void ODTalker::slotOpenBrowser(const QUrl& url)
{
    d->urlParametersMap = ParseUrlParameters(url.toString());
    d->accessToken = d->urlParametersMap.value("access_token");
    if(d->accessToken != ""){
      Q_EMIT oneDriveLinkingSucceeded();
    }
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Recieved URL from webview in link function: " << url ;
}
QMap<QString,QString> ODTalker::ParseUrlParameters(const QString &url)
{
  QMap<QString,QString> ret;
  if(url.indexOf('?')==-1)
  {
      return ret;
  }

  QString tmp = url.right(url.length()-url.indexOf('?')-1);
  tmp = tmp.right(tmp.length() - tmp.indexOf('#')-1);
  QStringList paramlist = tmp.split('&');

  for(int i=0;i<paramlist.count();i++)
  {
      QStringList paramarg = paramlist.at(i).split('=');
      ret.insert(paramarg.at(0),paramarg.at(1));
  }

  return ret;
}
bool ODTalker::authenticated()
{
    return d->o2->linked();
}
void ODTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(false);
}
void ODTalker::createFolder(const QString& path)
{
    //path also has name of new folder so send path parameter accordingly
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "createFolder:" << path;

    QUrl url(QLatin1String("https://graph.microsoft.com/v1.0/me/drive/root/children"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("bearer %1").arg(d->accessToken).toUtf8());

    QByteArray postData = QString::fromUtf8("{\"path\": \"%1\"}").arg(path).toUtf8();

    d->reply = d->netMngr->post(netRequest, postData);

    d->state = Private::OD_CREATEFOLDER;
    d->buffer.resize(0);
    emit signalBusy(true);
}
void ODTalker::getUserName()
{
    QUrl url(QLatin1String("https://graph.microsoft.com/v1.0/me"));

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("bearer %1").arg(d->accessToken).toUtf8());
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    d->reply = d->netMngr->get(netRequest);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "AFter Request Usar " << d->reply;
    d->state = Private::OD_USERNAME;
    d->buffer.resize(0);
    emit signalBusy(true);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "exiting";
}

/** Get list of folders by parsing json sent by dropbox
 */
void ODTalker::listFolders(const QString& path)
{
    QUrl url(QLatin1String("https://graph.microsoft.com/v1.0/me/drive/root/children"));;

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("bearer %1").arg(d->accessToken).toUtf8());
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::OD_LISTFOLDERS;
    d->buffer.resize(0);
    emit signalBusy(true);
}

bool ODTalker::addPhoto(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(true);

    ODMPForm form;
    QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

    if (image.isNull())
    {
        return false;
    }

    QString path = WSToolUtils::makeTemporaryDir("onedrive").filePath(QFileInfo(imgPath)
                                                 .baseName().trimmed() + QLatin1String(".jpg"));

    if (rescale && (image.width() > maxDim || image.height() > maxDim))
    {
        image = image.scaled(maxDim,maxDim, Qt::KeepAspectRatio,Qt::SmoothTransformation);
    }

    image.save(path,"JPEG",imageQuality);

    if (d->meta.load(imgPath))
    {
        d->meta.setImageDimensions(image.size());
        d->meta.setImageOrientation(DMetadata::ORIENTATION_NORMAL);
        d->meta.setImageProgramId(QLatin1String("digiKam"), digiKamVersion());
        d->meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
        d->meta.save(path);
    }

    if (!form.addFile(path))
    {
        emit signalBusy(false);
        return false;
    }

    QString uploadPath = uploadFolder + QUrl(QUrl::fromLocalFile(imgPath)).fileName();
    QUrl url(QString::fromLatin1("https://graph.microsoft.com/v1.0/me/drive/root:/%1:/content").arg(uploadPath));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/octet-stream"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("bearer {%1}").arg(d->accessToken).toUtf8());

    d->reply = d->netMngr->put(netRequest, form.formData());

    d->state = Private::OD_ADDPHOTO;
    d->buffer.resize(0);
    emit signalBusy(true);
    return true;
}
void ODTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state != Private::OD_CREATEFOLDER)
        {
            emit signalBusy(false);
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18n("Error"), reply->errorString());

            reply->deleteLater();
            return;
        }
    }

    d->buffer.append(reply->readAll());

    switch (d->state)
    {
        case Private::OD_LISTFOLDERS:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In OD_LISTFOLDERS";
            parseResponseListFolders(d->buffer);
            break;
        case Private::OD_CREATEFOLDER:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In OD_CREATEFOLDER";
            parseResponseCreateFolder(d->buffer);
            break;
        case Private::OD_ADDPHOTO:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In OD_ADDPHOTO";
            parseResponseAddPhoto(d->buffer);
            break;
        case Private::OD_USERNAME:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In OD_USERNAME";
            parseResponseUserName(d->buffer);
            break;
        default:
            break;
    }

    reply->deleteLater();
}
void ODTalker::parseResponseAddPhoto(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool success           = jsonObject.contains(QLatin1String("size"));
    emit signalBusy(false);

    if (!success)
    {
        emit signalAddPhotoFailed(i18n("Failed to upload photo"));
    }
    else
    {
        emit signalAddPhotoSucceeded();
    }
}
void ODTalker::parseResponseUserName(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QString name  = doc.object()[QLatin1String("displayName")].toString();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseUserName: "<<name;
    emit signalBusy(false);
    emit signalSetUserName(name);
}

void ODTalker::parseResponseListFolders(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        emit signalBusy(false);
        emit signalListAlbumsFailed(i18n("Failed to list folders"));
        return;
    }

    QJsonObject jsonObject = doc.object();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Json: " << doc;
    QJsonArray jsonArray   = jsonObject[QLatin1String("value")].toArray();

    //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Json response: " << jsonArray;

    QList<QPair<QString, QString> > list;
    list.append(qMakePair(QLatin1String(""), QLatin1String("root")));

    foreach (const QJsonValue& value, jsonArray)
    {
        QString id;
        QJsonObject folder;
        QString folderName;

        QJsonObject obj = value.toObject();
        folder       = obj[QLatin1String("folder")].toObject();

        if(!folder.isEmpty()){
          id            = folder[QLatin1String("id")].toString();
          folderName    = obj[QLatin1String("name")].toString();
          qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Folder Name is" << folderName;
          list.append(qMakePair(id, folderName));

        }
    }

    emit signalBusy(false);
    emit signalListAlbumsDone(list);
}

void ODTalker::parseResponseCreateFolder(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool fail              = jsonObject.contains(QLatin1String("error"));

    emit signalBusy(false);

    if (fail)
    {
        emit signalCreateFolderFailed(jsonObject[QLatin1String("error_summary")].toString());
    }
    else
    {
        emit signalCreateFolderSucceeded();
    }
}

} // namespace Digikam
