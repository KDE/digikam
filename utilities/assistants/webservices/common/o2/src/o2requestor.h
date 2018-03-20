#ifndef O2REQUESTOR_H
#define O2REQUESTOR_H

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QByteArray>

#include "o0export.h"
#include "o2reply.h"

class O2;

/// Makes authenticated requests.
class O0_EXPORT O2Requestor: public QObject {
    Q_OBJECT

public:
    explicit O2Requestor(QNetworkAccessManager *manager, O2 *authenticator, QObject *parent = 0);
    ~O2Requestor();
    
    /// Some services require the access token to be sent as a Authentication HTTP header.
    /// This is the case for Twitch and Mixer.
    /// When the access token expires and is refreshed, O2Requestor::retry() needs to update the Authentication HTTP header.
    /// In order to do so, O2Requestor needs to know the format of the Authentication HTTP header.
    void setAccessTokenInAuthenticationHTTPHeaderFormat(const QString &value);

public Q_SLOTS:
    /// Make a GET request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int get(const QNetworkRequest &req);

    /// Make a POST request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int post(const QNetworkRequest &req, const QByteArray &data);

    /// Make a PUT request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int put(const QNetworkRequest &req, const QByteArray &data);

Q_SIGNALS:
    /// Emitted when a request has been completed or failed.
    void finished(int id, QNetworkReply::NetworkError error, QByteArray data);

    /// Emitted when an upload has progressed.
    void uploadProgress(int id, qint64 bytesSent, qint64 bytesTotal);

protected Q_SLOTS:
    /// Handle refresh completion.
    void onRefreshFinished(QNetworkReply::NetworkError error);

    /// Handle request finished.
    void onRequestFinished();

    /// Handle request error.
    void onRequestError(QNetworkReply::NetworkError error);

    /// Re-try request (after successful token refresh).
    void retry();

    /// Finish the request, Q_EMIT finished() signal.
    void finish();

    /// Handle upload progress.
    void onUploadProgress(qint64 uploaded, qint64 total);

protected:
    int setup(const QNetworkRequest &request, QNetworkAccessManager::Operation operation);

    enum Status {
        Idle, Requesting, ReRequesting
    };

    QNetworkAccessManager *manager_;
    O2 *authenticator_;
    QNetworkRequest request_;
    QByteArray data_;
    QNetworkReply *reply_;
    Status status_;
    int id_;
    QNetworkAccessManager::Operation operation_;
    QUrl url_;
    O2ReplyList timedReplies_;
    QNetworkReply::NetworkError error_;
    QString accessTokenInAuthenticationHTTPHeaderFormat_;
};

#endif // O2REQUESTOR_H
