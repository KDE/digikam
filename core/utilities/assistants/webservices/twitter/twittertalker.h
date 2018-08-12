#ifndef TW_TALKER_H
#define TW_TALKER_H

// Qt includes

#include <QList>
#include <QPair>
#include <QString>
#include <QSettings>
#include <QNetworkReply>
#include <QNetworkAccessManager>

// Local includes

#include "twitteritem.h"
#include "o2.h"
#include "o0globals.h"
#include "dmetadata.h"
#include "o1twitter.h"

namespace Digikam
{

class TwTalker : public QObject
{
    Q_OBJECT

public:

    explicit TwTalker(QWidget* const parent);
    ~TwTalker();

public:

    void link();
    void unLink();
    void getUserName();
    bool authenticated();
    void cancel();
    bool addPhoto(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality);
    void listFolders(const QString& path = QString());
    void createFolder(QString& path);
    void setAccessToken(const QString& token);
    QMap<QString,QString> ParseUrlParameters(const QString& url);

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLinkingSucceeded();
    void signalLinkingFailed();
    void signalSetUserName(const QString& msg);
    void signalListAlbumsFailed(const QString& msg);
    void signalListAlbumsDone(const QList<QPair<QString, QString> >& list);
    void signalCreateFolderFailed(const QString& msg);
    void signalCreateFolderSucceeded();
    void signalAddPhotoFailed(const QString& msg);
    void signalAddPhotoSucceeded();
    void twitterLinkingSucceeded();
    void twitterLinkingFailed();

private Q_SLOTS:

    void slotLinkingFailed();
    void slotLinkingSucceeded();
    void slotOpenBrowser(const QUrl& url);
    void slotFinished(QNetworkReply* reply);
    void slotTweetDone();
    //void slotGetMediaId(QNetworkReply* reply);
    void reply2finish();

private:

    void parseResponseUserName(const QByteArray& data);
    void parseResponseListFolders(const QByteArray& data);
    void parseResponseCreateFolder(const QByteArray& data);
    void parseResponseAddPhoto(const QByteArray& data);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // OD_TALKER_H
