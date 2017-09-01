/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#include "habstract_avtransport_service.h"
#include "habstract_avtransport_service_p.h"

#include "hduration.h"
#include "hplaymode.h"
#include "hseekinfo.h"
#include "hmediainfo.h"
#include "hpositioninfo.h"
#include "htransportinfo.h"
#include "htransportstate.h"
#include "htransportaction.h"
#include "htransportsettings.h"
#include "hrecordqualitymode.h"
#include "hdevicecapabilities.h"
#include "hrecordmediumwritestatus.h"

#include "../common/hstoragemedium.h"
#include "../mediarenderer/hrendererconnection_info.h"

#include <../../HUpnpCore/private/hlogger_p.h>

#include <../../HUpnpCore/HUdn>
#include <../../HUpnpCore/HServiceId>
#include <../../HUpnpCore/HResourceType>
#include <../../HUpnpCore/HActionArguments>

#include <QtCore/QSet>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAbstractTransportServicePrivate
 ******************************************************************************/
HAbstractTransportServicePrivate::HAbstractTransportServicePrivate()
{
}

HAbstractTransportServicePrivate::~HAbstractTransportServicePrivate()
{
}

qint32 HAbstractTransportServicePrivate::setAVTransportURI(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();
    QString currentUri = inArgs.value(QLatin1String("CurrentURI")).toString();
    QString metadata = inArgs.value(QLatin1String("CurrentURIMetaData")).toString();

    return q->setAVTransportURI(instanceId, (QUrl)currentUri, metadata);
}

qint32 HAbstractTransportServicePrivate::setNextAVTransportURI(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();
    QString currentUri = inArgs.value(QLatin1String("NextURI")).toString();
    QString metadata = inArgs.value(QLatin1String("NextURIMetaData")).toString();

    return q->setNextAVTransportURI(instanceId, (QUrl)currentUri, metadata);
}

qint32 HAbstractTransportServicePrivate::getMediaInfo(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    HMediaInfo arg;
    qint32 retVal = q->getMediaInfo(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("NrTracks"), arg.numberOfTracks());
        outArgs->setValue(QLatin1String("MediaDuration"), arg.mediaDuration().toString());
        outArgs->setValue(QLatin1String("CurrentURI"), arg.currentUri().toString());
        outArgs->setValue(QLatin1String("CurrentURIMetaData"), arg.currentUriMetadata());
        outArgs->setValue(QLatin1String("NextURI"), arg.nextUri().toString());
        outArgs->setValue(QLatin1String("NextURIMetaData"), arg.nextUriMetadata());
        outArgs->setValue(QLatin1String("PlayMedium"), arg.playMedium().toString());
        outArgs->setValue(QLatin1String("RecordMedium"), arg.recordMedium().toString());
        outArgs->setValue(QLatin1String("WriteStatus"), arg.writeStatus().toString());
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::getMediaInfo_ext(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    HMediaInfo arg;
    qint32 retVal = q->getMediaInfo_ext(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("CurrentType"), HMediaInfo::toString(arg.mediaCategory()));
        outArgs->setValue(QLatin1String("NrTracks"), arg.numberOfTracks());
        outArgs->setValue(QLatin1String("MediaDuration"), arg.mediaDuration().toString());
        outArgs->setValue(QLatin1String("CurrentURI"), arg.currentUri().toString());
        outArgs->setValue(QLatin1String("CurrentURIMetaData"), arg.currentUriMetadata());
        outArgs->setValue(QLatin1String("NextURI"), arg.nextUri().toString());
        outArgs->setValue(QLatin1String("NextURIMetaData"), arg.nextUriMetadata());
        outArgs->setValue(QLatin1String("PlayMedium"), arg.playMedium().toString());
        outArgs->setValue(QLatin1String("RecordMedium"), arg.recordMedium().toString());
        outArgs->setValue(QLatin1String("WriteStatus"), arg.writeStatus().toString());
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::getTransportInfo(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    HTransportInfo arg;
    qint32 retVal = q->getTransportInfo(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("CurrentTransportState"), arg.state().toString());
        outArgs->setValue(QLatin1String("CurrentTransportStatus"), arg.status().toString());
        outArgs->setValue(QLatin1String("CurrentSpeed"), arg.speed());
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::getPositionInfo(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    HPositionInfo arg;
    qint32 retVal = q->getPositionInfo(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("Track"), arg.track());
        outArgs->setValue(QLatin1String("TrackDuration"), arg.trackDuration().toString());
        outArgs->setValue(QLatin1String("TrackMetaData"), arg.trackMetadata());
        outArgs->setValue(QLatin1String("TrackURI"), arg.trackUri().toString());
        outArgs->setValue(QLatin1String("RelTime"), arg.relativeTimePosition().toString());
        outArgs->setValue(QLatin1String("AbsTime"), arg.absoluteTimePosition().toString());
        outArgs->setValue(QLatin1String("RelCount"), arg.relativeCounterPosition());
        outArgs->setValue(QLatin1String("AbsCount"), arg.absoluteCounterPosition());
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::getDeviceCapabilities(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    HDeviceCapabilities arg;
    qint32 retVal = q->getDeviceCapabilities(instanceId, &arg);
    if (retVal != UpnpSuccess)
    {
        return retVal;
    }

    QStringList tmp;
    foreach(const HStorageMedium& medium, arg.playMedia())
    {
        tmp.append(medium.toString());
    }

    outArgs->setValue(QLatin1String("PlayMedia"), tmp.join(QLatin1String(",")));

    tmp.clear();
    foreach(const HStorageMedium& medium, arg.recordMedia())
    {
        tmp.append(medium.toString());
    }

    outArgs->setValue(QLatin1String("RecMedia"), tmp.join(QLatin1String(",")));

    tmp.clear();
    foreach(const HRecordQualityMode& mode, arg.recordQualityModes())
    {
        tmp.append(mode.toString());
    }

    outArgs->setValue(QLatin1String("RecQualityModes"), tmp.join(QLatin1String(",")));

    return UpnpSuccess;
}

qint32 HAbstractTransportServicePrivate::getTransportSettings(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    HTransportSettings settings;
    qint32 retVal = q->getTransportSettings(instanceId, &settings);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("PlayMode"), settings.playMode().toString());
        outArgs->setValue(QLatin1String("RecQualityMode"), settings.recordQualityMode().toString());
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::stop(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    return q->stop(instanceId);
}

qint32 HAbstractTransportServicePrivate::play(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();
    QString playSpeed = inArgs.value(QLatin1String("Speed")).toString();

    return q->play(instanceId, playSpeed);
}

qint32 HAbstractTransportServicePrivate::pause(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    return q->pause(instanceId);
}

qint32 HAbstractTransportServicePrivate::record(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    return q->record(instanceId);
}

qint32 HAbstractTransportServicePrivate::seek(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();
    QString unitAsStr = inArgs.value(QLatin1String("Unit")).toString();
    QString target = inArgs.value(QLatin1String("Target")).toString();

    HSeekInfo arg(unitAsStr, target);
    return q->seek(instanceId, arg);
}

qint32 HAbstractTransportServicePrivate::next(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    return q->next(instanceId);
}

qint32 HAbstractTransportServicePrivate::previous(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    return q->previous(instanceId);
}

qint32 HAbstractTransportServicePrivate::setPlayMode(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();
    HPlayMode playMode = inArgs.value(QLatin1String("NewPlayMode")).toString();

    return q->setPlayMode(instanceId, playMode);
}

qint32 HAbstractTransportServicePrivate::setRecordQualityMode(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();
    QString recQualityMode =
        inArgs.value(QLatin1String("NewRecordQualityMode")).toString();

    return q->setRecordQualityMode(instanceId, HRecordQualityMode(recQualityMode));
}

qint32 HAbstractTransportServicePrivate::getCurrentTransportActions(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    QSet<HTransportAction> arg;
    qint32 retVal = q->getCurrentTransportActions(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        QStringList tmp;
        foreach(const HTransportAction& action, arg)
        {
            tmp.append(action.toString());
        }

        outArgs->setValue(QLatin1String("Actions"), tmp.join(QLatin1String(",")));
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::getDRMState(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();

    HAvTransportInfo::DrmState arg;
    qint32 retVal = q->getDrmState(instanceId, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("CurrentDRMState"), HAvTransportInfo::drmStateToString(arg));
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::getStateVariables(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();
    QSet<QString> svNames = inArgs.value(QLatin1String("StateVariableList")).toString().split(QLatin1String(",")).toSet();

    QString arg;
    qint32 retVal = q->getStateVariables(instanceId, svNames, &arg);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("StateVariableValuePairs"), arg);
    }

    return retVal;
}

qint32 HAbstractTransportServicePrivate::setStateVariables(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractTransportService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 instanceId = inArgs.value(QLatin1String("InstanceID")).toUInt();
    HUdn udn = inArgs.value(QLatin1String("RenderingControlUDN")).toString();
    HResourceType rt = HResourceType(inArgs.value(QLatin1String("ServiceType")).toString());
    HServiceId sid = inArgs.value(QLatin1String("ServiceId")).toString();
    QString svValuePairs = inArgs.value(QLatin1String("StateVariableValuePairs")).toString();

    QStringList arg;
    qint32 retVal = q->setStateVariables(
        instanceId, udn, rt, sid, svValuePairs, &arg);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("StateVariableList"), arg.join(QLatin1String(",")));
    }

    return retVal;
}

/*******************************************************************************
 * HAbstractTransportService
 ******************************************************************************/
HAbstractTransportService::HAbstractTransportService(
    HAbstractTransportServicePrivate& dd) :
        HServerService(dd)
{
}

HAbstractTransportService::HAbstractTransportService() :
    HServerService(*new HAbstractTransportServicePrivate())
{
}

HAbstractTransportService::~HAbstractTransportService()
{
}

HAbstractTransportService::HActionInvokes
    HAbstractTransportService::createActionInvokes()
{
    H_D(HAbstractTransportService);

    HActionInvokes retVal;

    retVal.insert(QLatin1String(
        "SetAVTransportURI"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::setAVTransportURI));

    retVal.insert(QLatin1String(
        "SetNextAVTransportURI"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::setNextAVTransportURI));

    retVal.insert(QLatin1String(
        "GetMediaInfo"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::getMediaInfo));

    retVal.insert(QLatin1String(
        "GetMediaInfo_Ext"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::getMediaInfo_ext));

    retVal.insert(QLatin1String(
        "GetTransportInfo"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::getTransportInfo));

    retVal.insert(QLatin1String(
        "GetPositionInfo"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::getPositionInfo));

    retVal.insert(QLatin1String(
        "GetDeviceCapabilities"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::getDeviceCapabilities));

    retVal.insert(QLatin1String(
        "GetTransportSettings"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::getTransportSettings));

    retVal.insert(QLatin1String("Stop"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::stop));

    retVal.insert(QLatin1String("Play"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::play));

    retVal.insert(QLatin1String(
        "Pause"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::pause));

    retVal.insert(QLatin1String(
        "Record"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::record));

    retVal.insert(QLatin1String(
        "Seek"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::seek));

    retVal.insert(QLatin1String(
        "Next"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::next));

    retVal.insert(QLatin1String(
        "Previous"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::previous));

    retVal.insert(QLatin1String(
        "SetPlayMode"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::setPlayMode));

    retVal.insert(QLatin1String(
        "SetRecordQualityMode"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::setRecordQualityMode));

    retVal.insert(QLatin1String(
        "GetCurrentTransportActions"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::getCurrentTransportActions));

    retVal.insert(QLatin1String(
        "GetDRMState"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::getDRMState));

    retVal.insert(QLatin1String(
        "GetStateVariables"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::getStateVariables));

    retVal.insert(QLatin1String(
        "SetStateVariables"),
        HActionInvoke(h, &HAbstractTransportServicePrivate::setStateVariables));

    return retVal;
}

qint32 HAbstractTransportService::setNextAVTransportURI(
    quint32 /*instanceId*/, const QUrl& /*nextUri*/,
    const QString& /*nextUriMetaData*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractTransportService::pause(quint32 /*instanceId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractTransportService::record(quint32 /*instanceId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractTransportService::getCurrentTransportActions(
    quint32 /*instanceId*/, QSet<HTransportAction>* /*retVal*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractTransportService::getDrmState(
    quint32 /*instanceId*/, HAvTransportInfo::DrmState* /*retVal*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractTransportService::getStateVariables(
    quint32 /*instanceId*/,
    const QSet<QString>& /*stateVariableNames*/,
    QString* /*stateVariableValuePairs*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractTransportService::setStateVariables(
    quint32 /*instanceId*/,
    const HUdn& /*avTransportUdn*/,
    const HResourceType& /*serviceType*/,
    const HServiceId& /*serviceId*/,
    const QString& /*stateVariableValuePairs*/,
    QStringList* /*stateVariableList*/)
{
    return UpnpOptionalActionNotImplemented;
}

}
}
}
