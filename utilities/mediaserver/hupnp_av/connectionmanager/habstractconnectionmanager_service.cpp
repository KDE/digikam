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

#include "habstractconnectionmanager_service.h"
#include "habstractconnectionmanager_service_p.h"

#include "hconnectioninfo.h"
#include "hprotocolinforesult.h"
#include "hconnectionmanager_id.h"
#include "hconnectionmanager_info.h"
#include "hprepareforconnection_result.h"

#include "hav_global_p.h"
#include "hprotocolinfo.h"

#include "hlogger_p.h"

#include "hactionarguments.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAbstractConnectionManagerServicePrivate
 ******************************************************************************/
HAbstractConnectionManagerServicePrivate::HAbstractConnectionManagerServicePrivate() :
    HServerServicePrivate()
{
}

HAbstractConnectionManagerServicePrivate::~HAbstractConnectionManagerServicePrivate()
{
}

qint32 HAbstractConnectionManagerServicePrivate::getProtocolInfo(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractConnectionManagerService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HProtocolInfoResult result;
    qint32 retVal = q->getProtocolInfo(&result);
    if (retVal == UpnpSuccess)
    {
        if (!result.source().isEmpty())
        {
            QString sourceProtocolInfos = strToCsvString(result.source());
            outArgs->setValue(QLatin1String("Source"), sourceProtocolInfos);
        }
        if (!result.sink().isEmpty())
        {
            QString sinkProtocolInfos = strToCsvString(result.sink());
            outArgs->setValue(QLatin1String("Sink"), sinkProtocolInfos);
        }
    }

    return retVal;
}

qint32 HAbstractConnectionManagerServicePrivate::prepareForConnection(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractConnectionManagerService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HPrepareForConnectionResult pfcResult;
    qint32 retVal = q->prepareForConnection(
        HProtocolInfo(inArgs.value(QLatin1String("RemoteProtocolInfo")).toString()),
        HConnectionManagerId(inArgs.value(QLatin1String("PeerConnectionManager")).toString()),
        inArgs.value(QLatin1String("PeerConnectionID")).toInt(),
        HConnectionManagerInfo::directionFromString(inArgs.value(QLatin1String("Direction")).toString()),
        &pfcResult);

    outArgs->setValue(QLatin1String("ConnectionID"), pfcResult.connectionId());
    outArgs->setValue(QLatin1String("AVTransportID"), pfcResult.avTransportId());
    outArgs->setValue(QLatin1String("RcsID"), pfcResult.rcsId());

    return retVal;
}

qint32 HAbstractConnectionManagerServicePrivate::connectionComplete(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractConnectionManagerService);

    return q->connectionComplete(inArgs.value(QLatin1String("ConnectionID")).toInt());
}

qint32 HAbstractConnectionManagerServicePrivate::getCurrentConnectionIDs(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractConnectionManagerService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QList<quint32> connectionIds;
    qint32 retVal = q->getCurrentConnectionIDs(&connectionIds);
    if (retVal == UpnpSuccess)
    {
        QString idsAsCsv = numToCsvString(connectionIds);
        outArgs->setValue(QLatin1String("ConnectionIDs"), idsAsCsv);
    }

    return retVal;
}

qint32 HAbstractConnectionManagerServicePrivate::getCurrentConnectionInfo(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractConnectionManagerService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HConnectionInfo connectionInfo;
    qint32 retVal = q->getCurrentConnectionInfo(
        inArgs.value(QLatin1String("ConnectionID")).toInt(), &connectionInfo);

    if (retVal == UpnpSuccess && connectionInfo.isValid())
    {
        outArgs->setValue(QLatin1String("RcsID"), connectionInfo.rcsId());
        outArgs->setValue(QLatin1String("AVTransportID"), connectionInfo.avTransportId());
        outArgs->setValue(QLatin1String("ProtocolInfo"), connectionInfo.protocolInfo().toString());
        outArgs->setValue(QLatin1String("PeerConnectionManager"), connectionInfo.peerConnectionManager().toString());
        outArgs->setValue(QLatin1String("PeerConnectionID"), connectionInfo.peerConnectionId());
        outArgs->setValue(QLatin1String("Direction"), HConnectionManagerInfo::directionToString(connectionInfo.direction()));
        outArgs->setValue(QLatin1String("Status"), HConnectionManagerInfo::statusToString(connectionInfo.status()));
    }

    return retVal;
}

/*******************************************************************************
 * HAbstractConnectionManagerService
 ******************************************************************************/
HAbstractConnectionManagerService::HAbstractConnectionManagerService(
    HAbstractConnectionManagerServicePrivate& dd) :
        HServerService(dd)
{
}

HAbstractConnectionManagerService::HAbstractConnectionManagerService() :
    HServerService(*new HAbstractConnectionManagerServicePrivate())
{
}

HAbstractConnectionManagerService::~HAbstractConnectionManagerService()
{
}

HServerService::HActionInvokes HAbstractConnectionManagerService::createActionInvokes()
{
    H_D(HAbstractConnectionManagerService);

    HActionInvokes retVal;

    retVal.insert(QLatin1String("GetProtocolInfo"),
        HActionInvoke(h, &HAbstractConnectionManagerServicePrivate::getProtocolInfo));

    retVal.insert(QLatin1String("PrepareForConnection"),
        HActionInvoke(h, &HAbstractConnectionManagerServicePrivate::prepareForConnection));

    retVal.insert(QLatin1String("ConnectionComplete"),
        HActionInvoke(h, &HAbstractConnectionManagerServicePrivate::connectionComplete));

    retVal.insert(QLatin1String("GetCurrentConnectionIDs"),
        HActionInvoke(h, &HAbstractConnectionManagerServicePrivate::getCurrentConnectionIDs));

    retVal.insert(QLatin1String("GetCurrentConnectionInfo"),
        HActionInvoke(h, &HAbstractConnectionManagerServicePrivate::getCurrentConnectionInfo));

    return retVal;
}

qint32 HAbstractConnectionManagerService::prepareForConnection(
    const HProtocolInfo&, const HConnectionManagerId&, qint32,
    HConnectionManagerInfo::Direction, HPrepareForConnectionResult*)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractConnectionManagerService::connectionComplete(qint32)
{
    return UpnpOptionalActionNotImplemented;
}

}
}
}
