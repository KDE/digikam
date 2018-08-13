#include <twittertalker.h>

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
/*#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>*/

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "wstoolutils.h"
#include "twitterwindow.h"
#include "twitteritem.h"
#include "twittermpform.h"
#include "previewloadthread.h"
#include "o0settingsstore.h"
#include "o1requestor.h"

namespace Digikam
{

class TwTalker::Private
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
        clientId              =     QLatin1String("Kej10Xqld2SzYHpl1zPNXBkdz");
        clientSecret          =     QLatin1String("u7012XOx5Xd4t2oH10UMsffY8NseowtsfrXscoOzi4I0c039MF");
        //scope          =     QLatin1String("User.Read Files.ReadWrite");

        authUrl               =     QLatin1String("https://api.twitter.com/oauth/request_token");
        requestTokenUrl       =     QLatin1String("https://api.twitter.com/oauth/authenticate");
        accessTokenUrl        =     QLatin1String("https://api.twitter.com/oauth/access_token");

        redirectUrl    =     QLatin1String("http://127.0.0.1:8000");

        state          =     OD_USERNAME;
        netMngr        =     0;
        reply          =     0;
        accessToken    =     "";
    }

public:

    QString                clientId;
    QString                clientSecret;
    QString                authUrl;
    QString                requestTokenUrl;
    QString                accessTokenUrl;
    QString                scope;
    QString                redirectUrl;
    QString                accessToken;

    QWidget*               parent;

    QNetworkAccessManager* netMngr;

    QNetworkReply*         reply;

    State                  state;

    QByteArray             buffer;

    DMetadata              meta;

    QMap<QString,QString> urlParametersMap;

    //QWebEngineView*        view;

    QSettings*             settings;

    O1Twitter*                    o1Twitter;
};
TwTalker::TwTalker(QWidget* const parent)
    : d(new Private)
{
    d->parent  = parent;
    d->netMngr = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    d->o1Twitter = new O1Twitter(this);
    d->o1Twitter->setClientId(d->clientId);
    d->o1Twitter->setClientSecret(d->clientSecret);
    d->o1Twitter->setLocalPort(8000);

    d->settings                  = WSToolUtils::getOauthSettings(this);
    O0SettingsStore* const store = new O0SettingsStore(d->settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(QLatin1String("twitter"));
    d->o1Twitter->setStore(store);

    connect(d->o1Twitter, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(d->o1Twitter, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->o1Twitter, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));
}
TwTalker::~TwTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }
    delete d;
}
void TwTalker::link()
{
    /*emit signalBusy(true);
    QUrl url(d->requestTokenUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("OAuth oauth_callback= \"%1\"").arg(d->redirectUrl).toUtf8());
    QNetworkAccessManager requestMngr;
    QNetworkReply* reply;
    reply = requestMngr.post(netRequest);
    if (reply->error() != QNetworkReply::NoError){

    }
    QByteArray buffer;
    buffer.append(reply->readAll());
    QString response = fromLatin1(buffer);

    QMap<QString, QString> headers;

    // Discard the first line
    response = response.mid(response.indexOf('\n') + 1).trimmed();

    foreach(QString line, response.split('\n')) {
        int colon = line.indexOf(':');
        QString headerName = line.left(colon).trimmed();
        QString headerValue = line.mid(colon + 1).trimmed();

        headers.insertMulti(headerName, headerValue);
    }
    QString oauthToken = headers[oauth_token];
    QSting oauthTokenSecret = headers[oauth_token_secret];

    QUrlQuery query(url);
    query.addQueryItem(QLatin1String("client_id"), d->clientId);
    query.addQueryItem(QLatin1String("scope"), d->scope);
    query.addQueryItem(QLatin1String("redirect_uri"), d->redirectUrl);
    query.addQueryItem(QLatin1String("response_type"), "token");
    url.setQuery(query);

    d->view = new QWebEngineView(d->parent);
    d->view->setWindowFlags(Qt::Dialog);
    d->view->load(url);
    d->view->show();

    connect(d->view, SIGNAL(urlChanged(QUrl)), this, SLOT(slotCatchUrl(QUrl)));*/

    emit signalBusy(true);
    d->o1Twitter->link();
}
void TwTalker::unLink()
{
    /*d->accessToken = "";
    d->view->page()->profile()->cookieStore()->deleteAllCookies();
    Q_EMIT oneDriveLinkingSucceeded();*/

    d->o1Twitter->unlink();

    d->settings->beginGroup(QLatin1String("Dropbox"));
    d->settings->remove(QString());
    d->settings->endGroup();
}

void TwTalker::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Open Browser...";
    QDesktopServices::openUrl(url);
}

QMap<QString,QString> TwTalker::ParseUrlParameters(const QString &url)
{
  QMap<QString,QString> urlParameters;
  if(url.indexOf('?')==-1)
  {
      return urlParameters;
  }

  QString tmp = url.right(url.length()-url.indexOf('?')-1);
  tmp = tmp.right(tmp.length() - tmp.indexOf('#')-1);
  QStringList paramlist = tmp.split('&');

  for(int i=0;i<paramlist.count();i++)
  {
      QStringList paramarg = paramlist.at(i).split('=');
      urlParameters.insert(paramarg.at(0),paramarg.at(1));
  }

  return urlParameters;
}
void TwTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Twitter fail";
    emit signalBusy(false);
}
void TwTalker::slotLinkingSucceeded()
{
    /*if (d->accessToken == "")
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Twitter ok";
        emit signalBusy(false);
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Twitter ok";
    d->view->close();
    emit signalLinkingSucceeded();*/


    if (!d->o1Twitter->linked())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Twitter ok";
        emit signalBusy(false);
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Twitter ok";
    QVariantMap extraTokens = d->o1Twitter->extraTokens();
    if (!extraTokens.isEmpty()) {
        //emit extraTokensReady(extraTokens);
        qDebug() << "Extra tokens in response:";
        foreach (QString key, extraTokens.keys()) {
            qDebug() << "\t" << key << ":" << (extraTokens.value(key).toString().left(3) + "...");
        }
    }
    emit signalLinkingSucceeded();

}

bool TwTalker::authenticated()
{
    return d->o1Twitter->linked();
}
void TwTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = 0;
    }

    emit signalBusy(false);
}
/*void TwTalker::upload(QByteArray& data){
  QByteArray encoded = data.toBase64(QByteArray::Base64UrlEncoding);

  QUrl url = QUrl("https://upload.twitter.com/1.1/media/upload.json");
}*/
bool TwTalker::addPhoto(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality)
{

    //qDebug() << "Status update message:" << message.toLatin1().constData();
    emit signalBusy(true);
    TwMPForm form;
    QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();
    qint64 imageSize = QFileInfo(imgPath).size();
    qDebug() << "SIZE of image using qfileinfo:   " << imageSize;
    qDebug() << " " ;
    if (image.isNull())
    {
        return false;
    }

    QString path = WSToolUtils::makeTemporaryDir("twitter").filePath(QFileInfo(imgPath)
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
    if(form.formData().isEmpty()){
      qDebug() << "Form DATA Empty:";
    }
    if(form.formData().isNull()){
      qDebug() << "Form DATA null:";
    }
    QUrl url = QUrl("https://upload.twitter.com/1.1/media/upload.json");

    O1Requestor* requestor = new O1Requestor(d->netMngr, d->o1Twitter, this);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    //These are the parameters passed for the first step in chuncked media upload.
    //reqParams << O0RequestParameter(QByteArray("command"), QByteArray("INIT"));
    //reqParams << O0RequestParameter(QByteArray("media_type"), QByteArray("image/jpeg"));
    //reqParams << O0RequestParameter(QByteArray("total_bytes"), QString::fromLatin1("%1").arg(imageSize).toUtf8());

    reqParams << O0RequestParameter(QByteArray("media"), form.formData());

    QByteArray postData = O1::createQueryParameters(reqParams);


    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");
    QNetworkReply *reply2 = requestor->post(request, reqParams, postData);
    qDebug() << "reply size:              " << reply2->readBufferSize();  //always returns 0
    QByteArray replyData = reply2->readAll();
    qDebug() << "Media reply:" << replyData; //always Empty


    //uncomment this to try to post a tweet
    QUrl url2 = QUrl("https://api.twitter.com/1.1/statuses/update.json");
    reqParams = QList<O0RequestParameter>();
    reqParams << O0RequestParameter(QByteArray("status"), "Hello");
    //reqParams << O0RequestParameter(QByteArray("media_ids"), mediaId.toUtf8());

    postData = O1::createQueryParameters(reqParams);

    request.setUrl(url2);
    request.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);

    QNetworkReply *reply = requestor->post(request, reqParams, postData);
    connect(reply, SIGNAL(finished()), this, SLOT(slotTweetDone()));


    connect(reply2, SIGNAL(finished()), this, SLOT(reply2finish()));
    emit signalBusy(true);
    return true;
}
void TwTalker::reply2finish(){
  qDebug() << "In Reply2:";
  QNetworkReply *reply2 = qobject_cast<QNetworkReply *>(sender());
  QString mediaId;
  if (reply2->error() != QNetworkReply::NoError) {

      qDebug() << "ERROR:" << reply2->errorString();
      qDebug() << "Content:" << reply2->readAll();
      emit signalAddPhotoFailed(i18n("Failed to upload photo"));
  } else {
    QByteArray replyData = reply2->readAll();
    QJsonDocument doc      = QJsonDocument::fromJson(replyData);
    qDebug() << "Media reply:" << replyData;
    QJsonObject jsonObject = doc.object();
    mediaId = jsonObject[QLatin1String("media_id")].toString();
  }

  qDebug() << "Media ID:" << mediaId;
}
void TwTalker::getUserName()
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
}
void TwTalker::slotTweetDone()
{
    qDebug() << "In slotTweetDone:";
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "ERROR:" << reply->errorString();
        qDebug() << "Content:" << reply->readAll();
        emit signalAddPhotoFailed(i18n("Failed to upload photo"));
    } else {
        qDebug() << "Tweet posted successfully!";
        emit signalAddPhotoSucceeded();
    }

}
/*bool TwTalker::addPhoto(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "PATH " << imgPath;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Folder " << uploadFolder;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Others: " << rescale << " " << maxDim << " " <<imageQuality;
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

    QString path = WSToolUtils::makeTemporaryDir("twitter").filePath(QFileInfo(imgPath)
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
}*/
void TwTalker::slotFinished(QNetworkReply* reply)
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
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_LISTFOLDERS";
            parseResponseListFolders(d->buffer);
            break;
        case Private::OD_CREATEFOLDER:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_CREATEFOLDER";
            parseResponseCreateFolder(d->buffer);
            break;
        case Private::OD_ADDPHOTO:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_ADDPHOTO";
            parseResponseAddPhoto(d->buffer);
            break;
        case Private::OD_USERNAME:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_USERNAME";
            parseResponseUserName(d->buffer);
            break;
        default:
            break;
    }

    reply->deleteLater();
}
void TwTalker::parseResponseAddPhoto(const QByteArray& data)
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
void TwTalker::parseResponseUserName(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseUserName: "<<doc;
    QString name  = doc.object()[QLatin1String("displayName")].toString();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseUserName: "<<name;
    emit signalBusy(false);
    emit signalSetUserName(name);
}

void TwTalker::parseResponseListFolders(const QByteArray& data)
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
    //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Json: " << doc;
    QJsonArray jsonArray   = jsonObject[QLatin1String("value")].toArray();

    //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Json response: " << jsonArray;

    QList<QPair<QString, QString> > list;
    list.append(qMakePair(QLatin1String(""), QLatin1String("root")));

    foreach (const QJsonValue& value, jsonArray)
    {
        QString path;
        QString folderName;
        QJsonObject folder;

        QJsonObject obj = value.toObject();
        folder       = obj[QLatin1String("folder")].toObject();

        if(!folder.isEmpty()){
          folderName    = obj[QLatin1String("name")].toString();
          path          =    "/" + folderName;
          qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Folder Name is" << folderName;
          list.append(qMakePair(path, folderName));

        }
    }

    emit signalBusy(false);
    emit signalListAlbumsDone(list);
}

void TwTalker::parseResponseCreateFolder(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool fail              = jsonObject.contains(QLatin1String("error"));

    emit signalBusy(false);

    if (fail)
    {
      QJsonParseError err;
      QJsonDocument doc = QJsonDocument::fromJson(data, &err);
      qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseCreateFolder ERROR: " << doc;
      emit signalCreateFolderFailed(jsonObject[QLatin1String("error_summary")].toString());
    }
    else
    {
        emit signalCreateFolderSucceeded();
    }
}

} // namespace Digikam
