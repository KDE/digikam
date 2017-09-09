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

#include "havtransport_adapter.h"
#include "havtransport_adapter_p.h"

#include "hduration.h"
#include "hplaymode.h"
#include "hseekinfo.h"
#include "hmediainfo.h"
#include "hpositioninfo.h"
#include "htransportinfo.h"
#include "htransportinfo.h"
#include "htransportaction.h"
#include "hrecordqualitymode.h"
#include "htransportsettings.h"
#include "hdevicecapabilities.h"
#include "hrecordmediumwritestatus.h"

#include "../common/hstoragemedium.h"
#include "../hav_devicemodel_infoprovider.h"

#include "hlogger_p.h"

#include "hudn.h"
#include "hasyncop.h"
#include "hserviceid.h"
#include "hactioninfo.h"
#include "hserviceinfo.h"
#include "hresourcetype.h"
#include "hclientaction.h"
#include "hclientservice.h"
#include "hclientactionop.h"
#include "hclientadapterop.h"
#include "hactionarguments.h"
#include "hstatevariable_event.h"
#include "hclientstatevariable.h"

#include <QtCore/QUrl>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAvTransportAdapterPrivate
 ******************************************************************************/
HAvTransportAdapterPrivate::HAvTransportAdapterPrivate() :
    HClientServiceAdapterPrivate(HAvTransportInfo::supportedServiceType()),
        m_instanceId(0)
{
}

HAvTransportAdapterPrivate::~HAvTransportAdapterPrivate()
{
}

bool HAvTransportAdapterPrivate::setAVTransportURI(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->setAVTransportURICompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::setNextAVTransportURI(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->setNextAVTransportURICompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::getMediaInfo(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HMediaInfo mediaInfo;
    if (op.returnValue() == UpnpSuccess)
    {
        const HActionArguments& outArgs = op.outputArguments();

        quint32 nrTracks = outArgs.value(QLatin1String("NrTracks")).toUInt();
        HDuration duration = outArgs.value(QLatin1String("MediaDuration")).toString();
        QUrl curUri = outArgs.value(QLatin1String("CurrentURI")).toUrl();
        QString curUriMetadata = outArgs.value(QLatin1String("CurrentURIMetaData")).toString();
        QUrl nextUri = outArgs.value(QLatin1String("NextURI")).toUrl();
        QString nextUriMetadata = outArgs.value(QLatin1String("NextURIMetaData")).toString();
        HStorageMedium playMedium = outArgs.value(QLatin1String("PlayMedium")).toString();
        HStorageMedium recMedium  = outArgs.value(QLatin1String("RecordMedium")).toString();
        HRecordMediumWriteStatus ws = outArgs.value(QLatin1String("WriteStatus")).toString();

        mediaInfo = HMediaInfo(
            nrTracks, duration, curUri, curUriMetadata, nextUri, nextUriMetadata,
            playMedium, recMedium, ws, HMediaInfo::Undefined);
    }
    emit q->getMediaInfoCompleted(q, takeOp(op, mediaInfo));

    return false;
}

bool HAvTransportAdapterPrivate::getMediaInfo_ext(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HMediaInfo mediaInfo;
    if (op.returnValue() == UpnpSuccess)
    {
        const HActionArguments& outArgs = op.outputArguments();

        QString currentType = outArgs.value(QLatin1String("CurrentType")).toString();
        quint32 nrTracks = outArgs.value(QLatin1String("NrTracks")).toUInt();
        HDuration duration = outArgs.value(QLatin1String("MediaDuration")).toString();
        QUrl curUri = outArgs.value(QLatin1String("CurrentURI")).toUrl();
        QString curUriMetadata = outArgs.value(QLatin1String("CurrentURIMetaData")).toString();
        QUrl nextUri = outArgs.value(QLatin1String("NextURI")).toUrl();
        QString nextUriMetadata = outArgs.value(QLatin1String("NextURIMetaData")).toString();
        HStorageMedium playMedium = outArgs.value(QLatin1String("PlayMedium")).toString();
        HStorageMedium recMedium  = outArgs.value(QLatin1String("RecordMedium")).toString();
        HRecordMediumWriteStatus ws = outArgs.value(QLatin1String("WriteStatus")).toString();

        mediaInfo = HMediaInfo(
            nrTracks, duration, curUri, curUriMetadata, nextUri, nextUriMetadata,
            playMedium, recMedium, ws,
            HMediaInfo::mediaCategoryFromString(currentType));
    }
    emit q->getMediaInfo_extCompleted(q, takeOp(op, mediaInfo));

    return false;
}

bool HAvTransportAdapterPrivate::getTransportInfo(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HTransportInfo info;
    if (op.returnValue() == UpnpSuccess)
    {
        const HActionArguments& outArgs = op.outputArguments();

        QString state = outArgs.value(QLatin1String("CurrentTransportState")).toString();
        QString status = outArgs.value(QLatin1String("CurrentTransportStatus")).toString();
        QString speed = outArgs.value(QLatin1String("CurrentSpeed")).toString();

       info = HTransportInfo(state, status, speed);
    }
    emit q->getTransportInfoCompleted(q, takeOp(op, info));

    return false;
}

bool HAvTransportAdapterPrivate::getPositionInfo(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HPositionInfo info;
    if (op.returnValue() == UpnpSuccess)
    {
        const HActionArguments& outArgs = op.outputArguments();

        quint32 track = outArgs.value(QLatin1String("Track")).toUInt();
        HDuration trackDuration = outArgs.value(QLatin1String("TrackDuration")).toString();
        QString trackMd = outArgs.value(QLatin1String("TrackMetaData")).toString();
        QUrl trackUri = (QUrl) outArgs.value(QLatin1String("TrackURI")).toString();
        HDuration relTime = outArgs.value(QLatin1String("RelTime")).toString();
        HDuration absTime = outArgs.value(QLatin1String("AbsTime")).toString();
        qint32 relCountPos = outArgs.value(QLatin1String("RelCount")).toInt();
        quint32 absCountPos = outArgs.value(QLatin1String("AbsCount")).toUInt();

       info = HPositionInfo(
           track, trackDuration, trackMd, trackUri, relTime, absTime,
           relCountPos, absCountPos);
    }
    emit q->getPositionInfoCompleted(q, takeOp(op, info));

    return false;
}

bool HAvTransportAdapterPrivate::getDeviceCapabilities(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HDeviceCapabilities capabilities;
    if (op.returnValue() == UpnpSuccess)
    {
        const HActionArguments& outArgs = op.outputArguments();

        QStringList pmedia = outArgs.value(QLatin1String("PlayMedia")).toString().split(QLatin1String(","));
        QStringList rmedia = outArgs.value(QLatin1String("RecMedia")).toString().split(QLatin1String(","));
        QStringList rqMode = outArgs.value(QLatin1String("RecQualityModes")).toString().split(QLatin1String(","));

        capabilities =
            HDeviceCapabilities(pmedia.toSet(), rmedia.toSet(), rqMode.toSet());
    }
    emit q->getDeviceCapabilitiesCompleted(q, takeOp(op, capabilities));

    return false;
}

bool HAvTransportAdapterPrivate::getTransportSettings(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HTransportSettings settings;
    if (op.returnValue() == UpnpSuccess)
    {
        const HActionArguments& outArgs = op.outputArguments();

        HPlayMode pm = outArgs.value(QLatin1String("PlayMode")).toString();
        QString rqMode = outArgs.value(QLatin1String("RecQualityMode")).toString();

        settings = HTransportSettings(pm, rqMode);
    }
    emit q->getTransportSettingsCompleted(q, takeOp(op, settings));

    return false;
}

bool HAvTransportAdapterPrivate::stop(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->stopCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::play(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->playCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::pause(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->pauseCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::record(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->recordCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::seek(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->seekCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::next(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->nextCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::previous(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->previousCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::setPlayMode(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);
    emit q->setPlayModeCompleted(q, takeOp(op));
    return false;
}

bool HAvTransportAdapterPrivate::setRecordQualityMode(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    emit q->setRecordQualityModeCompleted(
        q, takeOp(op));

    return false;
}

bool HAvTransportAdapterPrivate::getCurrentTransportActions(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    QSet<HTransportAction> actions;
    if (op.returnValue() == UpnpSuccess)
    {
        QStringList slist = op.outputArguments().value(QLatin1String("Actions")).toString().split(QLatin1String(","));

        foreach(const QString& action, slist)
        {
            HTransportAction ta(action);
            if (ta.isValid())
            {
                actions.insert(ta);
            }
        }
    }
    emit q->getCurrentTransportActionsCompleted(q, takeOp(op, actions));

    return false;
}

bool HAvTransportAdapterPrivate::getDRMState(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    HAvTransportInfo::DrmState drmState = HAvTransportInfo::DrmState_Unknown;
    if (op.returnValue() == UpnpSuccess)
    {
        drmState = HAvTransportInfo::drmStateFromString(
            op.outputArguments().value(QLatin1String("CurrentDRMState")).toString());
    }
    emit q->getDrmStateCompleted(q, takeOp(op, drmState));

    return false;
}

bool HAvTransportAdapterPrivate::getStateVariables(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    QString stateVariableValuePairs;
    if (op.returnValue() == UpnpSuccess)
    {
        stateVariableValuePairs =
            op.outputArguments().value(QLatin1String("StateVariableValuePairs")).toString();
    }
    emit q->getStateVariablesCompleted(q, takeOp(op, stateVariableValuePairs));

    return false;
}

bool HAvTransportAdapterPrivate::setStateVariables(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HAvTransportAdapter);

    QStringList retVal;
    if (op.returnValue() == UpnpSuccess)
    {
        retVal = op.outputArguments().value(QLatin1String("StateVariableList")).toString().split(QLatin1String(","));
    }
    emit q->setStateVariablesCompleted(q, takeOp(op, retVal));

    return false;
}

/*******************************************************************************
 * HAvTransportAdapter
 ******************************************************************************/
HAvTransportAdapter::HAvTransportAdapter(
    quint32 instanceId, QObject* parent) :
        HClientServiceAdapter(*new HAvTransportAdapterPrivate(), parent)
{
    H_D(HAvTransportAdapter);
    h->m_instanceId = instanceId;
}

HAvTransportAdapter::~HAvTransportAdapter()
{
}

void HAvTransportAdapter::lastChange(
    const HClientStateVariable*, const HStateVariableEvent& event)
{
    emit lastChangeReceived(this, event.newValue().toString());
}

bool HAvTransportAdapter::prepareService(HClientService* service)
{
    const HClientStateVariable* lastChange = service->stateVariables().value(QLatin1String("LastChange"));
    if (lastChange)
    {
        bool ok = connect(
            lastChange,
            SIGNAL(valueChanged(const Herqq::Upnp::HClientStateVariable*,Herqq::Upnp::HStateVariableEvent)),
            this,
            SLOT(lastChange(const Herqq::Upnp::HClientStateVariable*,Herqq::Upnp::HStateVariableEvent)));
        Q_ASSERT(ok); Q_UNUSED(ok)
    }
    return true;
}

HClientAdapterOpNull HAvTransportAdapter::setAVTransportURI(
    const QUrl& currentUri, const QString& currentUriMetaData)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetAVTransportURI"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    if (!inArgs.setValue(QLatin1String("CurrentURI"), currentUri))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }
    if (!inArgs.setValue(QLatin1String("CurrentURIMetaData"), currentUriMetaData))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::setAVTransportURI));
}

HClientAdapterOpNull HAvTransportAdapter::setNextAVTransportURI(
    const QUrl& currentUri, const QString& currentUriMetaData)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetNextAVTransportURI"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    if (!inArgs.setValue(QLatin1String("NextURI"), currentUri))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }
    if (!inArgs.setValue(QLatin1String("NextURIMetaData"), currentUriMetaData))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::setNextAVTransportURI));
}

HClientAdapterOp<HMediaInfo> HAvTransportAdapter::getMediaInfo()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetMediaInfo"), &rc);
    if (!action)
    {
        return HClientAdapterOp<HMediaInfo>::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<HMediaInfo>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getMediaInfo));
}

HClientAdapterOp<HMediaInfo> HAvTransportAdapter::getMediaInfo_ext()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetMediaInfo_Ext"), &rc);
    if (!action)
    {
        return HClientAdapterOp<HMediaInfo>::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<HMediaInfo>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getMediaInfo_ext));
}

HClientAdapterOp<HTransportInfo> HAvTransportAdapter::getTransportInfo()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetTransportInfo"), &rc);
    if (!action)
    {
        return HClientAdapterOp<HTransportInfo>::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<HTransportInfo>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getTransportInfo));
}

HClientAdapterOp<HPositionInfo> HAvTransportAdapter::getPositionInfo()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetPositionInfo"), &rc);
    if (!action)
    {
        return HClientAdapterOp<HPositionInfo>::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<HPositionInfo>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getPositionInfo));
}

HClientAdapterOp<HDeviceCapabilities> HAvTransportAdapter::getDeviceCapabilities()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetDeviceCapabilities"), &rc);
    if (!action)
    {
        return HClientAdapterOp<HDeviceCapabilities>::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<HDeviceCapabilities>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getDeviceCapabilities));
}

HClientAdapterOp<HTransportSettings> HAvTransportAdapter::getTransportSettings()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetTransportSettings"), &rc);
    if (!action)
    {
        return HClientAdapterOp<HTransportSettings>::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<HTransportSettings>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getTransportSettings));
}

HClientAdapterOpNull HAvTransportAdapter::stop()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("Stop"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::stop));
}

HClientAdapterOpNull HAvTransportAdapter::play(const QString& speed)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("Play"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    if (!inArgs.setValue(QLatin1String("Speed"), speed))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::play));
}

HClientAdapterOpNull HAvTransportAdapter::pause()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("Pause"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::pause));
}

HClientAdapterOpNull HAvTransportAdapter::record()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("Record"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::record));
}

HClientAdapterOpNull HAvTransportAdapter::seek(const HSeekInfo& info)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("Seek"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    if (!inArgs.setValue(QLatin1String("Unit"), info.unit().toString()))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }
    if (!inArgs.setValue(QLatin1String("Target"), info.target()))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::seek));
}

HClientAdapterOpNull HAvTransportAdapter::next()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("Next"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::next));
}

HClientAdapterOpNull HAvTransportAdapter::previous()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("Previous"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::previous));
}

HClientAdapterOpNull HAvTransportAdapter::setPlayMode(const HPlayMode& mode)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetPlayMode"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    if (!inArgs.setValue(QLatin1String("NewPlayMode"), mode.toString()))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::setPlayMode));
}

HClientAdapterOpNull HAvTransportAdapter::setRecordQualityMode(const HRecordQualityMode& mode)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetRecordQualityMode"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    if (!inArgs.setValue(QLatin1String("NewRecordQualityMode"), mode.toString()))
    {
        return HClientAdapterOpNull::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }

    return h_ptr->beginInvoke(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::setRecordQualityMode));
}

HClientAdapterOp<QSet<HTransportAction> > HAvTransportAdapter::getCurrentTransportActions()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetCurrentTransportActions"), &rc);
    if (!action)
    {
        return HClientAdapterOp<QSet<HTransportAction> >::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<QSet<HTransportAction> >(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getCurrentTransportActions));
}

HClientAdapterOp<HAvTransportInfo::DrmState> HAvTransportAdapter::getDrmState()
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetDRMState"), &rc);
    if (!action)
    {
        return HClientAdapterOp<HAvTransportInfo::DrmState>::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<HAvTransportInfo::DrmState>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getDRMState));
}

HClientAdapterOp<QString> HAvTransportAdapter::getStateVariables(
    const QSet<QString>& stateVariableNames)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetStateVariables"), &rc);
    if (!action)
    {
        return HClientAdapterOp<QString>::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    if (!inArgs.setValue(
        QLatin1String("StateVariableList"), QStringList(stateVariableNames.toList()).join(QLatin1String(","))))
    {
        return HClientAdapterOp<QString>::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }

    return h_ptr->beginInvoke<QString>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::getStateVariables));
}

HClientAdapterOp<QStringList> HAvTransportAdapter::setStateVariables(
    const HUdn& avTransportUdn, const HResourceType& serviceType,
    const HServiceId& serviceId, const QString& stateVariableValuePairs)
{
    H_D(HAvTransportAdapter);

    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetStateVariables"), &rc);
    if (!action)
    {
        return HClientAdapterOp<QStringList>::createInvalid(rc, QLatin1String(""));
    }

    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    if (!inArgs.setValue(QLatin1String("AVTransportUDN"), avTransportUdn.toSimpleUuid()))
    {
        return HClientAdapterOp<QStringList>::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }
    if (!inArgs.setValue(QLatin1String("ServiceType"), serviceType.toString()))
    {
        return HClientAdapterOp<QStringList>::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }
    if (!inArgs.setValue(QLatin1String("ServiceId"), serviceId.toString()))
    {
        return HClientAdapterOp<QStringList>::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }
    if (!inArgs.setValue(QLatin1String("StateVariableValuePairs"), stateVariableValuePairs))
    {
        return HClientAdapterOp<QStringList>::createInvalid(UpnpInvalidArgs, QLatin1String(""));
    }

    return h_ptr->beginInvoke<QStringList>(action,
        inArgs, HActionInvokeCallback(h, &HAvTransportAdapterPrivate::setStateVariables));
}

}
}
}
