/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hhttp_messagecreator_p.h"
#include "hhttp_messaginginfo_p.h"
#include "hhttp_header_p.h"
#include "hhttp_utils_p.h"

#include "hevent_messages_p.h"
#include "hactioninfo.h"
#include "hlogger_p.h"

#include "hupnp_global_p.h"

#include "qtsoap.h"

namespace Herqq
{

namespace Upnp
{

namespace
{
void checkForActionError(
    qint32 actionRetVal, QtSoapMessage::FaultCode* soapFault, qint32* httpStatusCode,
    QString* httpReasonPhrase)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(httpStatusCode);
    Q_ASSERT(httpReasonPhrase);
    Q_ASSERT(soapFault);

    if (actionRetVal == UpnpInvalidArgs)
    {
        *httpStatusCode   = 402;
        *httpReasonPhrase = QLatin1String("Invalid Args");
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == UpnpActionFailed)
    {
        *httpStatusCode   = 501;
        *httpReasonPhrase = QLatin1String("Action Failed");
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == UpnpArgumentValueInvalid)
    {
        *httpStatusCode   = 600;
        *httpReasonPhrase = QLatin1String("Argument Value Invalid");
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == UpnpArgumentValueOutOfRange)
    {
        *httpStatusCode   = 601;
        *httpReasonPhrase = QLatin1String("Argument Value Out of Range");
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == UpnpOptionalActionNotImplemented)
    {
        *httpStatusCode   = 602;
        *httpReasonPhrase = QLatin1String("Optional Action Not Implemented");
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == UpnpOutOfMemory)
    {
        *httpStatusCode   = 603;
        *httpReasonPhrase = QLatin1String("Out of Memory");
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == UpnpHumanInterventionRequired)
    {
        *httpStatusCode   = 604;
        *httpReasonPhrase = QLatin1String("Human Intervention Required");
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == UpnpStringArgumentTooLong)
    {
        *httpStatusCode   = 605;
        *httpReasonPhrase = QLatin1String("String Argument Too Long");
        *soapFault        = QtSoapMessage::Client;
    }
    else
    {
        *httpStatusCode   = actionRetVal;
        *httpReasonPhrase = QString::number(actionRetVal);
        *soapFault        = QtSoapMessage::Client;
    }
}

QString contentTypeToString(ContentType ct)
{
    QString retVal;
    switch(ct)
    {
    case ContentType_TextXml:
        retVal = QLatin1String("text/xml; charset=\"utf-8\"");
        break;
    case ContentType_OctetStream:
        retVal = QLatin1String("application/octet-stream");
        break;
    default:
        ;
    }
    return retVal;
}

void getStatusInfo(StatusCode sc, qint32* statusCode, QString* reasonPhrase)
{
    switch(sc)
    {
    case Ok:
        *statusCode = 200;
        *reasonPhrase = QLatin1String("OK");
        break;

    case BadRequest:
        *statusCode = 400;
        *reasonPhrase = QLatin1String("Bad Request");
        break;

    case IncompatibleHeaderFields:
        *statusCode = 400;
        *reasonPhrase = QLatin1String("Incompatible header fields");
        break;

    case Unauthorized:
        *statusCode = 401;
        *reasonPhrase = QLatin1String("Unauthorized");
        break;

    case Forbidden:
        *statusCode = 403;
        *reasonPhrase = QLatin1String("Forbidden");
        break;

    case NotFound:
        *statusCode = 404;
        *reasonPhrase = QLatin1String("Not Found");
        break;

    case MethotNotAllowed:
        *statusCode = 405;
        *reasonPhrase = QLatin1String("Method Not Allowed");
        break;

    case PreconditionFailed:
        *statusCode = 412;
        *reasonPhrase = QLatin1String("Precondition Failed");
        break;

    case InternalServerError:
        *statusCode = 500;
        *reasonPhrase = QLatin1String("Internal Server Error");
        break;

    case ServiceUnavailable:
        *statusCode = 503;
        *reasonPhrase = QLatin1String("Service Unavailable");
        break;

    default:
        Q_ASSERT(false);
    }
}

}

HHttpMessageCreator::HHttpMessageCreator()
{
}

HHttpMessageCreator::~HHttpMessageCreator()
{
}

QByteArray HHttpMessageCreator::setupData(
    HHttpHeader& hdr, const HMessagingInfo& mi)
{
    return setupData(hdr, QByteArray(), mi, ContentType_Undefined);
}

QByteArray HHttpMessageCreator::setupData(
    HHttpHeader& reqHdr, qint64 bodySizeInBytes, const HMessagingInfo& mi,
    ContentType ct)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(reqHdr.isValid());

    reqHdr.setValue(
        QLatin1String("DATE"),
        QDateTime::currentDateTime().toString(HHttpUtils::rfc1123DateFormat()));

    QString contentType = contentTypeToString(ct);
    if (!contentType.isEmpty())
    {
        reqHdr.setContentType(contentType);
    }

    reqHdr.setValue(QLatin1String("EXT"), QLatin1String(""));

    HProductTokens serverTokens = mi.serverInfo();
    if (!serverTokens.isEmpty())
    {
        reqHdr.setValue(QLatin1String("Server"), serverTokens.toString());
    }

    if (!mi.keepAlive() && reqHdr.minorVersion() == 1)
    {
        reqHdr.setValue(QLatin1String("Connection"), QLatin1String("close"));
    }

    reqHdr.setValue(QLatin1String("HOST"), mi.hostInfo());

    if (mi.chunkedInfo().max() > 0 &&
        bodySizeInBytes > mi.chunkedInfo().max())
    {
        reqHdr.setValue(QLatin1String("Transfer-Encoding"), QLatin1String("chunked"));
    }
    else
    {
        reqHdr.setContentLength(bodySizeInBytes);
    }

    QByteArray msg(reqHdr.toString().toUtf8());
    return msg;
}

QByteArray HHttpMessageCreator::setupData(
    HHttpHeader& reqHdr, const QByteArray& body, const HMessagingInfo& mi,
    ContentType ct)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(reqHdr.isValid());

    QByteArray msg = setupData(reqHdr, body.size(), mi, ct);
    msg.append(body);

    return msg;
}

QByteArray HHttpMessageCreator::createResponse(
    StatusCode sc, const HMessagingInfo& mi)
{
    return createResponse(sc, mi, QByteArray(), ContentType_Undefined);
}

QByteArray HHttpMessageCreator::createHeaderData(
    StatusCode sc, const HMessagingInfo& mi, qint64 bodySizeInBytes,
    ContentType ct)
{
    qint32 statusCode = 0;
    QString reasonPhrase = QLatin1String("");

    getStatusInfo(sc, &statusCode, &reasonPhrase);

    HHttpResponseHeader responseHdr(statusCode, reasonPhrase);
    return setupData(responseHdr, bodySizeInBytes, mi, ct);
}

QByteArray HHttpMessageCreator::createResponse(
    StatusCode sc, const HMessagingInfo& mi, const QByteArray& body, ContentType ct)
{
    qint32 statusCode = 0;
    QString reasonPhrase;

    getStatusInfo(sc, &statusCode, &reasonPhrase);

    HHttpResponseHeader responseHdr(statusCode, reasonPhrase);
    return setupData(responseHdr, body, mi, ct);
}

QByteArray HHttpMessageCreator::setupData(
    const HMessagingInfo& mi, qint32 statusCode, const QString& reasonPhrase,
    const QString& body, ContentType ct)
{
    HHttpResponseHeader responseHdr(statusCode, reasonPhrase);
    return setupData(responseHdr, body.toUtf8(), mi, ct);
}

QByteArray HHttpMessageCreator::createResponse(
    const HMessagingInfo& mi, qint32 actionErrCode, const QString& description)
{
    QtSoapMessage::FaultCode soapFault = QtSoapMessage::Other;
    qint32 httpStatusCode = 0;
    QString httpReasonPhrase;

    checkForActionError(
        actionErrCode, &soapFault, &httpStatusCode, &httpReasonPhrase);

    QtSoapMessage soapFaultResponse;
    soapFaultResponse.setFaultCode(soapFault);
    soapFaultResponse.setFaultString(QLatin1String("UPnPError"));

    QtSoapStruct* detail = new QtSoapStruct(QtSoapQName(QLatin1String("UPnPError")));
    detail->insert(new QtSoapSimpleType(QtSoapQName(QLatin1String("errorCode")), actionErrCode));
    detail->insert(new QtSoapSimpleType(QtSoapQName(QLatin1String("errorDescription")), description));
    soapFaultResponse.addFaultDetail(detail);

    return setupData(
        mi,
        httpStatusCode,
        httpReasonPhrase,
        soapFaultResponse.toXmlString(),
        ContentType_TextXml);
}

QByteArray HHttpMessageCreator::create(
    const HNotifyRequest& req, HMessagingInfo* mi)
{
    Q_ASSERT(req.isValid(true));

    HHttpRequestHeader reqHdr;
    reqHdr.setContentType(QLatin1String("Content-type: text/xml; charset=\"utf-8\""));

    reqHdr.setRequest(
        QLatin1String("NOTIFY"), extractRequestPart(QUrl(req.callback().toString())));

    mi->setHostInfo(req.callback());

    reqHdr.setValue(QLatin1String("SID"), req.sid().toString());
    reqHdr.setValue(QLatin1String("SEQ"), QString::number(req.seq()));
    reqHdr.setValue(QLatin1String("NT") , QLatin1String("upnp:event"));
    reqHdr.setValue(QLatin1String("NTS"), QLatin1String("upnp:propchange"));

    return setupData(reqHdr, req.data(), *mi, ContentType_TextXml);
}

QByteArray HHttpMessageCreator::create(
    const HSubscribeRequest& req, const HMessagingInfo& mi)
{
    Q_ASSERT(req.isValid(false));

    HHttpRequestHeader requestHdr(QLatin1String("SUBSCRIBE"), extractRequestPart(req.eventUrl()));
    requestHdr.setValue(QLatin1String("TIMEOUT"), req.timeout().toString());

    if (!req.isRenewal())
    {
        if (req.hasUserAgent())
        {
            requestHdr.setValue(QLatin1String("USER-AGENT"), req.userAgent().toString());
        }
        requestHdr.setValue(QLatin1String("CALLBACK"), HHttpUtils::callbackAsStr(req.callbacks()));
        requestHdr.setValue(QLatin1String("NT"), req.nt().typeToString());
    }
    else
    {
        requestHdr.setValue(QLatin1String("SID"), req.sid().toString());
    }

    return setupData(requestHdr, mi);
}

QByteArray HHttpMessageCreator::create(
    const HUnsubscribeRequest& req, HMessagingInfo* mi)
{
    Q_ASSERT(req.isValid(false));

    HHttpRequestHeader requestHdr(
        QLatin1String("UNSUBSCRIBE"), extractRequestPart(req.eventUrl()));

    mi->setHostInfo(req.eventUrl());

    requestHdr.setValue(QLatin1String("SID"), req.sid().toString());

    return setupData(requestHdr, *mi);
}

QByteArray HHttpMessageCreator::create(
    const HSubscribeResponse& response, const HMessagingInfo& mi)
{
    Q_ASSERT(response.isValid(true));

    HHttpResponseHeader responseHdr(200, QLatin1String("OK"));
    responseHdr.setContentLength(0);

    responseHdr.setValue(QLatin1String("SID")    , response.sid().toString());
    responseHdr.setValue(QLatin1String("TIMEOUT"), response.timeout().toString());
    responseHdr.setValue(QLatin1String("SERVER") , response.server().toString());

    return setupData(responseHdr, mi);
}

int HHttpMessageCreator::create(
    const HHttpRequestHeader& reqHdr, const QByteArray& body, HNotifyRequest& req)
{
    HLOG(H_AT, H_FUN);

    QString nt     = reqHdr.value(QLatin1String("NT") );
    QString nts    = reqHdr.value(QLatin1String("NTS"));
    QString sid    = reqHdr.value(QLatin1String("SID"));
    QString seqStr = reqHdr.value(QLatin1String("SEQ"));
    QString host   = reqHdr.value(QLatin1String("HOST")).trimmed();

    QString deliveryPath = reqHdr.path().trimmed();
    if (!deliveryPath.startsWith(QLatin1Char('/')))
    {
        deliveryPath.insert(0, QLatin1Char('/'));
    }

    QUrl callbackUrl(QString(QLatin1String("http://%1%2")).arg(host, deliveryPath));

    HNotifyRequest nreq;
    HNotifyRequest::RetVal retVal =
        nreq.setContents(callbackUrl, nt, nts, sid, seqStr, QLatin1String(body.data()));

    switch(retVal)
    {
    case HNotifyRequest::Success:
        break;

    case HNotifyRequest::PreConditionFailed:
        break;

    case HNotifyRequest::InvalidContents:
    case HNotifyRequest::InvalidSequenceNr:
        break;

    default:
        Q_ASSERT(false);

        retVal = HNotifyRequest::BadRequest;
    }

    req = nreq;
    return retVal;
}

int HHttpMessageCreator::create(
    const HHttpRequestHeader& reqHdr, HSubscribeRequest& req)
{
    HLOG(H_AT, H_FUN);

    QString nt         = reqHdr.value(QLatin1String("NT"));
    QString callback   = reqHdr.value(QLatin1String("CALLBACK")).trimmed();
    QString timeoutStr = reqHdr.value(QLatin1String("TIMEOUT"));
    QString sid        = reqHdr.value(QLatin1String("SID"));
    QString userAgent  = reqHdr.value(QLatin1String("USER-AGENT"));
    QString host       = reqHdr.value(QLatin1String("HOST"));
    QUrl servicePath   = QUrl(reqHdr.path().trimmed());

    HSubscribeRequest sreq;
    HSubscribeRequest::RetVal retVal =
        sreq.setContents(
            nt, appendUrls(QUrl(QString(QLatin1String("http://"))+host), servicePath),
            sid, callback, timeoutStr, userAgent);

    switch(retVal)
    {
    case HSubscribeRequest::Success:
        break;

    case HSubscribeRequest::PreConditionFailed:
        break;

    case HSubscribeRequest::IncompatibleHeaders:
        break;

    case HSubscribeRequest::BadRequest:
        break;

    default:
        Q_ASSERT(false);

        retVal = HSubscribeRequest::BadRequest;
    }

    req = sreq;
    return retVal;
}

int HHttpMessageCreator::create(
    const HHttpRequestHeader& reqHdr, HUnsubscribeRequest& req)
{
    HLOG(H_AT, H_FUN);

    QString sid     = reqHdr.value(QLatin1String("SID"));
    QUrl callback   = QUrl(reqHdr.value(QLatin1String("CALLBACK")).trimmed());
    QString hostStr = reqHdr.value(QLatin1String("HOST")).trimmed();

    if (!callback.isEmpty())
    {
        return HUnsubscribeRequest::IncompatibleHeaders;
    }

    HUnsubscribeRequest usreq;
    HUnsubscribeRequest::RetVal retVal =
        usreq.setContents(
            appendUrls(QUrl(QString(QLatin1String("http://"))+hostStr), QUrl(reqHdr.path().trimmed())), sid);

    switch(retVal)
    {
    case HUnsubscribeRequest::Success:
        break;

    case HUnsubscribeRequest::PreConditionFailed:
        break;

    default:
        Q_ASSERT(false);

        retVal = HUnsubscribeRequest::BadRequest;
    }

    req = usreq;
    return retVal;
}

bool HHttpMessageCreator::create(
    const HHttpResponseHeader& respHdr, HSubscribeResponse& resp)
{
    HLOG(H_AT, H_FUN);

    if (!respHdr.isValid() || respHdr.statusCode() != 200)
    {
        return false;
    }

    HSid      sid     = HSid(respHdr.value(QLatin1String("SID")));
    HTimeout  timeout = HTimeout(respHdr.value(QLatin1String("TIMEOUT")));
    QString   server  = respHdr.value(QLatin1String("SERVER"));
    QDateTime date    =
        QDateTime::fromString(respHdr.value(QLatin1String("DATE")), HHttpUtils::rfc1123DateFormat());

    resp = HSubscribeResponse(sid, HProductTokens(server), timeout, date);
    return resp.isValid(false);
}

}
}
