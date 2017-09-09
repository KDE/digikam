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

#include "hconnectionmanager_adapter.h"
#include "hconnectionmanager_adapter_p.h"

#include "hconnectioninfo.h"
#include "hprotocolinforesult.h"
#include "hconnectionmanager_id.h"
#include "hprepareforconnection_result.h"

#include "hprotocolinfo.h"
#include "hav_devicemodel_infoprovider.h"

#include "hlogger_p.h"

#include "hactioninfo.h"
#include "hclientaction.h"
#include "hclientservice.h"
#include "hclientactionop.h"
#include "hactionarguments.h"
#include "hclientadapterop.h"
#include "hstatevariable_event.h"
#include "hclientstatevariable.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

namespace
{
HProtocolInfos parse(const QString& protocolInfos)
{
    HProtocolInfos retVal;
    QStringList sourceCsv = protocolInfos.split(QLatin1Char(','));
    foreach(const QString& str, sourceCsv)
    {
        HProtocolInfo pi = str;
        if (!pi.isEmpty())
        {
            retVal.append(pi);
        }
    }
    return retVal;
}

QList<quint32> parseIDs(const QString& ids)
{
    QList<quint32> retVal;
    QStringList idsCsv = ids.split(QLatin1String(","));
    foreach(const QString& str, idsCsv)
    {
        bool ok = false;
        quint32 id = str.toUInt(&ok);
        if (ok)
        {
            retVal.append(id);
        }
    }
    return retVal;
}
}

/*******************************************************************************
 * HConnectionManagerAdapterPrivate
 ******************************************************************************/
HConnectionManagerAdapterPrivate::HConnectionManagerAdapterPrivate() :
    HClientServiceAdapterPrivate(HConnectionManagerInfo::supportedServiceType()),
        m_connectionIds()
{
}

HConnectionManagerAdapterPrivate::~HConnectionManagerAdapterPrivate()
{
}

bool HConnectionManagerAdapterPrivate::getProtocolInfo(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HConnectionManagerAdapter);

    QList<HProtocolInfo> source, sink;
    if (op.returnValue() == UpnpSuccess)
    {
        HActionArguments outArgs = op.outputArguments();

        QString sourceStr = outArgs.value(QLatin1String("Source")).toString();
        QString sinkStr   = outArgs.value(QLatin1String("Sink")).toString();

        source = parse(sourceStr);
        sink = parse(sinkStr);
    }

    HProtocolInfoResult result(source, sink);
    emit q->getProtocolInfoCompleted(q, takeOp(op, result));

    return false;
}

bool HConnectionManagerAdapterPrivate::prepareForConnection(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HConnectionManagerAdapter);

    HPrepareForConnectionResult result;
    if (op.returnValue() == UpnpSuccess)
    {
        HActionArguments outArgs = op.outputArguments();

        qint32 connectionId = outArgs.value(QLatin1String("ConnectionID")).toInt();
        qint32 avTransportId = outArgs.value(QLatin1String("AVTransportID")).toInt();
        qint32 rcsId = outArgs.value(QLatin1String("RcsID")).toInt();

        result.setConnectionId(connectionId);
        result.setAvTransportId(avTransportId);
        result.setRcsId(rcsId);
    }
    emit q->prepareForConnectionCompleted(q, takeOp(op, result));

    return false;
}

bool HConnectionManagerAdapterPrivate::connectionComplete(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HConnectionManagerAdapter);
    emit q->connectionCompleteCompleted(q, takeOp(op));
    return false;
}

bool HConnectionManagerAdapterPrivate::getCurrentConnectionIDs(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HConnectionManagerAdapter);

    QList<quint32> connectionIds;
    if (op.returnValue() == UpnpSuccess)
    {
        connectionIds = parseIDs(op.outputArguments().value(QLatin1String("ConnectionIDs")).toString());
    }
    emit q->getCurrentConnectionIDsCompleted(q, takeOp(op, connectionIds));

    return false;
}

bool HConnectionManagerAdapterPrivate::getCurrentConnectionInfo(
    HClientAction*, const HClientActionOp& op)
{
    HLOG(H_AT, H_FUN);
    H_Q(HConnectionManagerAdapter);

    HConnectionInfo connectionInfo;
    if (op.returnValue() == UpnpSuccess)
    {
        HActionArguments outArgs = op.outputArguments();

        qint32 rcsId = outArgs.value(QLatin1String("RcsID")).toInt();
        qint32 avTransportId = outArgs.value(QLatin1String("AVTransportID")).toInt();
        QString protocolInfo = outArgs.value(QLatin1String("ProtocolInfo")).toString();
        QString peerCm = outArgs.value(QLatin1String("PeerConnectionManager")).toString();
        qint32 peerCid = outArgs.value(QLatin1String("PeerConnectionID")).toInt();
        QString dirStr = outArgs.value(QLatin1String("Direction")).toString();
        QString statusStr = outArgs.value(QLatin1String("Status")).toString();

        qint32 connectionId = m_connectionIds.value(op.id());

        HProtocolInfo pinfo(protocolInfo);
        if (!pinfo.isValid())
        {
            HLOG_WARN_NONSTD(QLatin1String(
                "Received invalid ProtocolInfo information. Assuming all fields were "
                "meant to be wild-cards"));
            pinfo = HProtocolInfo::createUsingWildcards();
        }

        connectionInfo =
            HConnectionInfo(
                connectionId,
                rcsId,
                avTransportId,
                pinfo,
                HConnectionManagerId(peerCm),
                peerCid,
                HConnectionManagerInfo::directionFromString(dirStr),
                HConnectionManagerInfo::statusFromString(statusStr));
    }
    emit q->getCurrentConnectionInfoCompleted(
        q, takeOp(op, connectionInfo));

    return false;
}

/*******************************************************************************
 * HConnectionManagerAdapter
 ******************************************************************************/
HConnectionManagerAdapter::HConnectionManagerAdapter(QObject* parent) :
    HClientServiceAdapter(*new HConnectionManagerAdapterPrivate(), parent)
{
}

HConnectionManagerAdapter::~HConnectionManagerAdapter()
{
}

void HConnectionManagerAdapter::sourceProtocolInfoChanged_(
    const HClientStateVariable* source, const HStateVariableEvent& event)
{
    Q_UNUSED(source)
    emit sourceProtocolInfoChanged(this, parse(event.newValue().toString()));
}

void HConnectionManagerAdapter::sinkProtocolInfoChanged_(
    const HClientStateVariable* source, const HStateVariableEvent& event)
{
    Q_UNUSED(source)
    emit sinkProtocolInfoChanged(this, parse(event.newValue().toString()));
}

void HConnectionManagerAdapter::currentConnectionIDsChanged_(
    const HClientStateVariable* source, const HStateVariableEvent& event)
{
    Q_UNUSED(source)
    emit currentConnectionIdsChanged(this, parseIDs(event.newValue().toString()));
}

bool HConnectionManagerAdapter::prepareService(HClientService* service)
{
    const HClientStateVariable* source =
        service->stateVariables().value(QLatin1String("SourceProtocolInfo"));

    bool ok = connect(
        source,
        SIGNAL(valueChanged(
            const Herqq::Upnp::HClientStateVariable*,
            Herqq::Upnp::HStateVariableEvent)),
        this,
        SLOT(sourceProtocolInfoChanged_(
            const Herqq::Upnp::HClientStateVariable*,
            Herqq::Upnp::HStateVariableEvent)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    const HClientStateVariable* sink =
        service->stateVariables().value(QLatin1String("SinkProtocolInfo"));

    ok = connect(
        sink,
        SIGNAL(valueChanged(
            const Herqq::Upnp::HClientStateVariable*,
            Herqq::Upnp::HStateVariableEvent)),
        this,
        SLOT(sinkProtocolInfoChanged_(
            const Herqq::Upnp::HClientStateVariable*,
            Herqq::Upnp::HStateVariableEvent)));
    Q_ASSERT(ok);

    const HClientStateVariable* cids =
        service->stateVariables().value(QLatin1String("CurrentConnectionIDs"));

    ok = connect(
        cids,
        SIGNAL(valueChanged(
            const Herqq::Upnp::HClientStateVariable*,
            Herqq::Upnp::HStateVariableEvent)),
        this,
        SLOT(currentConnectionIDsChanged_(
            const Herqq::Upnp::HClientStateVariable*,
            Herqq::Upnp::HStateVariableEvent)));
    Q_ASSERT(ok);

    return true;
}

HClientAdapterOp<HProtocolInfoResult> HConnectionManagerAdapter::getProtocolInfo()
{
    H_D(HConnectionManagerAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetProtocolInfo"), &rc);
    if (!action)
    {
        return HClientAdapterOp<HProtocolInfoResult>::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    return h_ptr->beginInvoke<HProtocolInfoResult>(
        action, inArgs,
        HActionInvokeCallback(h, &HConnectionManagerAdapterPrivate::getProtocolInfo));
}

HClientAdapterOp<QList<quint32> > HConnectionManagerAdapter::getCurrentConnectionIDs()
{
    H_D(HConnectionManagerAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetCurrentConnectionIDs"), &rc);
    if (!action)
    {
        return HClientAdapterOp<QList<quint32> >::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    return h_ptr->beginInvoke<QList<quint32> >(
        action, inArgs,
        HActionInvokeCallback(h, &HConnectionManagerAdapterPrivate::getCurrentConnectionIDs));
}

HClientAdapterOp<HConnectionInfo>
    HConnectionManagerAdapter::getCurrentConnectionInfo(qint32 connectionId)
{
    H_D(HConnectionManagerAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetCurrentConnectionInfo"), &rc);
    if (!action)
    {
        return HClientAdapterOp<HConnectionInfo>::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("ConnectionID"), QString::number(connectionId));

    HClientActionOp op = action->beginInvoke(inArgs,
        HActionInvokeCallback(h, &HConnectionManagerAdapterPrivate::getCurrentConnectionInfo));

    h->m_connectionIds.insert(op.id(), connectionId);

    return h_ptr->addOp<HConnectionInfo>(op);
}

HClientAdapterOp<HPrepareForConnectionResult> HConnectionManagerAdapter::prepareForConnection(
    const HProtocolInfo& remoteProtocolInfo,
    const HConnectionManagerId& peerConnectionManager,
    qint32 peerConnectionId,
    HConnectionManagerInfo::Direction direction)
{
    H_D(HConnectionManagerAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("PrepareForConnection"), &rc);
    if (!action)
    {
        return HClientAdapterOp<HPrepareForConnectionResult>::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    if (!inArgs.setValue(QLatin1String("RemoteProtocolInfo"), remoteProtocolInfo.toString()))
    {
        return HClientAdapterOp<HPrepareForConnectionResult>::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }
    if (!inArgs.setValue(QLatin1String("PeerConnectionManager"), peerConnectionManager.toString()))
    {
        return HClientAdapterOp<HPrepareForConnectionResult>::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }
    if (!inArgs.setValue(QLatin1String("PeerConnectionID"), QString::number(peerConnectionId)))
    {
        return HClientAdapterOp<HPrepareForConnectionResult>::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }
    if (!inArgs.setValue(QLatin1String("Direction"), HConnectionManagerInfo::directionToString(direction)))
    {
        return HClientAdapterOp<HPrepareForConnectionResult>::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }

    return h_ptr->beginInvoke<HPrepareForConnectionResult>(
        action, inArgs,
        HActionInvokeCallback(h, &HConnectionManagerAdapterPrivate::prepareForConnection));
}

HClientAdapterOpNull HConnectionManagerAdapter::connectionComplete(qint32 connectionId)
{
    H_D(HConnectionManagerAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("ConnectionComplete"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("ConnectionID"), QString::number(connectionId));

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HConnectionManagerAdapterPrivate::connectionComplete));
}

}
}
}
