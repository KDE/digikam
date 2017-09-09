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

#include "hrenderingcontrol_adapter.h"
#include "hrenderingcontrol_adapter_p.h"
#include "hrenderingcontrol_info.h"
#include "hvolumedbrange_result.h"
#include "hchannel.h"

#include "hav_devicemodel_infoprovider.h"

#include "hudn.h"
#include "hserviceid.h"
#include "hactioninfo.h"
#include "hresourcetype.h"
#include "hclientservice.h"
#include "hclientactionop.h"
#include "hactionarguments.h"
#include "hclientadapterop.h"
#include "hstatevariable_event.h"
#include "hclientstatevariable.h"

#include "hlogger_p.h"

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HRenderingControlAdapterPrivate
 ******************************************************************************/
HRenderingControlAdapterPrivate::HRenderingControlAdapterPrivate() :
    HClientServiceAdapterPrivate(HRenderingControlInfo::supportedServiceType()),
        m_instanceId(0)
{
}

HRenderingControlAdapterPrivate::~HRenderingControlAdapterPrivate()
{
}

bool HRenderingControlAdapterPrivate::listPresets(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    QStringList currentPresetNameList =
        op.outputArguments().value(QLatin1String(
            "CurrentPresetNameList")).toString().split(QLatin1String(","));

    emit q->listPresetsCompleted(q, takeOp(op, currentPresetNameList));

    return false;
}

bool HRenderingControlAdapterPrivate::selectPreset(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->selectPresetCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getBrightness(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentBrightness =
        op.outputArguments().value(QLatin1String("CurrentBrightness")).toUInt();

    emit q->getBrightnessCompleted(q, takeOp(op, currentBrightness));

    return false;
}

bool HRenderingControlAdapterPrivate::setBrightness(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setBrightnessCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getContrast(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentContrast =
        op.outputArguments().value(QLatin1String("CurrentContrast")).toUInt();

    emit q->getContrastCompleted(q, takeOp(op, currentContrast));

    return false;
}

bool HRenderingControlAdapterPrivate::setContrast(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setContrastCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getSharpness(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentSharpness =
        op.outputArguments().value(QLatin1String("CurrentSharpness")).toUInt();

    emit q->getSharpnessCompleted(q, takeOp(op, currentSharpness));

    return false;
}

bool HRenderingControlAdapterPrivate::setSharpness(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setSharpnessCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getRedVideoGain(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentRedVideoGain =
        op.outputArguments().value(QLatin1String("CurrentRedVideoGain")).toUInt();

    emit q->getRedVideoGainCompleted(q, takeOp(op, currentRedVideoGain));

    return false;
}

bool HRenderingControlAdapterPrivate::setRedVideoGain(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setRedVideoGainCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getGreenVideoGain(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentGreenVideoGain =
        op.outputArguments().value(QLatin1String("CurrentGreenVideoGain")).toUInt();

    emit q->getGreenVideoGainCompleted(q, takeOp(op, currentGreenVideoGain));

    return false;
}

bool HRenderingControlAdapterPrivate::setGreenVideoGain(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setGreenVideoGainCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getBlueVideoGain(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentBlueVideoGain =
        op.outputArguments().value(QLatin1String("CurrentBlueVideoGain")).toUInt();

    emit q->getBlueVideoGainCompleted(q, takeOp(op, currentBlueVideoGain));

    return false;
}

bool HRenderingControlAdapterPrivate::setBlueVideoGain(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setBlueVideoGainCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getRedVideoBlackLevel(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentRedVideoBlackLevel =
        op.outputArguments().value(QLatin1String("CurrentRedVideoBlackLevel")).toUInt();

    emit q->getRedVideoBlackLevelCompleted(
        q, takeOp(op, currentRedVideoBlackLevel));

    return false;
}

bool HRenderingControlAdapterPrivate::setRedVideoBlackLevel(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setRedVideoBlackLevelCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getGreenVideoBlackLevel(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentGreenVideoBlackLevel =
        op.outputArguments().value(QLatin1String("CurrentGreenVideoBlackLevel")).toUInt();

    emit q->getGreenVideoBlackLevelCompleted(
        q, takeOp(op, currentGreenVideoBlackLevel));

    return false;
}

bool HRenderingControlAdapterPrivate::setGreenVideoBlackLevel(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setGreenVideoBlackLevelCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getBlueVideoBlackLevel(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentBlueVideoBlackLevel =
        op.outputArguments().value(QLatin1String("CurrentBlueVideoBlackLevel")).toUInt();

    emit q->getBlueVideoBlackLevelCompleted(
        q, takeOp(op, currentBlueVideoBlackLevel));

    return false;
}

bool HRenderingControlAdapterPrivate::setBlueVideoBlackLevel(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setBlueVideoBlackLevelCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getColorTemperature(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentColorTemperature =
        op.outputArguments().value(QLatin1String("CurrentColorTemperature")).toUInt();

    emit q->getColorTemperatureCompleted(q, takeOp(op, currentColorTemperature));

    return false;
}

bool HRenderingControlAdapterPrivate::setColorTemperature(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setColorTemperatureCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getHorizontalKeystone(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentHorizontalKeyStone =
        op.outputArguments().value(QLatin1String("CurrentHorizontalKeystone")).toUInt();

    emit q->getHorizontalKeystoneCompleted(
        q, takeOp(op, currentHorizontalKeyStone));

    return false;
}

bool HRenderingControlAdapterPrivate::setHorizontalKeystone(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setHorizontalKeystoneCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getVerticalKeystone(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentVerticalKeyStone =
        op.outputArguments().value(QLatin1String("CurrentVerticalKeystone")).toUInt();

    emit q->getVerticalKeystoneCompleted(
        q, takeOp(op, currentVerticalKeyStone));

    return false;
}

bool HRenderingControlAdapterPrivate::setVerticalKeystone(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setVerticalKeystoneCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getMute(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    bool currentlyMuted = op.outputArguments().value(QLatin1String("CurrentMute")).toBool();
    emit q->getMuteCompleted(q, takeOp(op, currentlyMuted));

    return false;
}

bool HRenderingControlAdapterPrivate::setMute(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setMuteCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getVolume(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    quint16 currentVolume = op.outputArguments().value(QLatin1String("CurrentVolume")).toUInt();
    emit q->getVolumeCompleted(q, takeOp(op, currentVolume));

    return false;
}

bool HRenderingControlAdapterPrivate::setVolume(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setVolumeCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getVolumeDB(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    qint16 currentVolumeDb = op.outputArguments().value(QLatin1String("CurrentVolume")).toInt();
    emit q->getVolumeDBCompleted(q, takeOp(op, currentVolumeDb));

    return false;
}

bool HRenderingControlAdapterPrivate::setVolumeDB(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setVolumeDBCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getVolumeDBRange(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    qint32 minValue = op.outputArguments().value(QLatin1String("MinValue")).toInt();
    qint32 maxValue = op.outputArguments().value(QLatin1String("MaxValue")).toInt();

    HVolumeDbRangeResult result(minValue, maxValue);

    emit q->getVolumeDBRangeCompleted(q, takeOp(op, result));

    return false;
}

bool HRenderingControlAdapterPrivate::getLoudness(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    bool loudnessOn = op.outputArguments().value(QLatin1String("CurrentLoudness")).toBool();
    emit q->getLoudnessCompleted(q, takeOp(op, loudnessOn));

    return false;
}

bool HRenderingControlAdapterPrivate::setLoudness(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);
    emit q->setLoudnessCompleted(q, takeOp(op));
    return false;
}

bool HRenderingControlAdapterPrivate::getStateVariables(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    QString stateVariableValuePairs =
        op.outputArguments().value(QLatin1String("StateVariableValuePairs")).toString();

    emit q->getStateVariablesCompleted(q, takeOp(op, stateVariableValuePairs));

    return false;
}

bool HRenderingControlAdapterPrivate::setStateVariables(
    HClientAction*, const HClientActionOp& op)
{
    H_Q(HRenderingControlAdapter);

    QStringList stateVariableList =
        op.outputArguments().value(QLatin1String("StateVariableList")).toString().split(QLatin1String(","));

    emit q->setStateVariablesCompleted(q, takeOp(op, stateVariableList));

    return false;
}

/*******************************************************************************
 * HRenderingControlAdapter
 ******************************************************************************/
HRenderingControlAdapter::HRenderingControlAdapter(
    quint32 instanceId, QObject* parent) :
        HClientServiceAdapter(*new HRenderingControlAdapterPrivate(), parent)
{
    H_D(HRenderingControlAdapter);
    h->m_instanceId = instanceId;
}

HRenderingControlAdapter::~HRenderingControlAdapter()
{
}

void HRenderingControlAdapter::lastChange(
    const HClientStateVariable*, const HStateVariableEvent& event)
{
    emit lastChangeReceived(this, event.newValue().toString());
}

bool HRenderingControlAdapter::prepareService(HClientService* service)
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

HClientAdapterOp<QStringList> HRenderingControlAdapter::listPresets()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("ListPresets"), &rc);
    if (!action)
    {
        return HClientAdapterOp<QStringList>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<QStringList>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::listPresets));
}

HClientAdapterOpNull HRenderingControlAdapter::selectPreset(const QString& presetName)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SelectPreset"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("PresetName"), presetName);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::selectPreset));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getBrightness()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetBrightness"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getBrightness));
}

HClientAdapterOpNull HRenderingControlAdapter::setBrightness(
    quint16 desiredBrightness)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetBrightness"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("DesiredBrightness"), desiredBrightness);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setBrightness));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getContrast()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetContrast"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getContrast));
}

HClientAdapterOpNull HRenderingControlAdapter::setContrast(
    quint16 desiredContrast)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetContrast"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("DesiredContrast"), desiredContrast);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setContrast));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getSharpness()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetSharpness"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getSharpness));
}

HClientAdapterOpNull HRenderingControlAdapter::setSharpness(
    quint16 desiredSharpness)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetSharpness"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("DesiredSharpness"), desiredSharpness);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setSharpness));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getRedVideoGain()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetRedVideoGain"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getRedVideoGain));
}

HClientAdapterOpNull HRenderingControlAdapter::setRedVideoGain(
    quint16 desiredRedVideoGain)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetRedVideoGain"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("DesiredRedVideoGain"), desiredRedVideoGain);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setRedVideoGain));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getGreenVideoGain()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetGreenVideoGain"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getGreenVideoGain));
}

HClientAdapterOpNull HRenderingControlAdapter::setGreenVideoGain(
    quint16 desiredGreenVideoGain)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetGreenVideoGain"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("DesiredGreenVideoGain"), desiredGreenVideoGain);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setGreenVideoGain));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getBlueVideoGain()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetBlueVideoGain"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getBlueVideoGain));
}

HClientAdapterOpNull HRenderingControlAdapter::setBlueVideoGain(
    quint16 desiredBlueVideoGain)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetBlueVideoGain"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("DesiredBlueVideoGain"), desiredBlueVideoGain);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setBlueVideoGain));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getRedVideoBlackLevel()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetRedVideoBlackLevel"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getRedVideoBlackLevel));
}

HClientAdapterOpNull HRenderingControlAdapter::setRedVideoBlackLevel(
    quint16 desiredRedVideoBlackLevel)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetRedVideoBlackLevel"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("DesiredRedVideoBlackLevel"), desiredRedVideoBlackLevel);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setRedVideoBlackLevel));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getGreenVideoBlackLevel()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetGreenVideoBlackLevel"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getGreenVideoBlackLevel));
}

HClientAdapterOpNull HRenderingControlAdapter::setGreenVideoBlackLevel(
    quint16 desiredGreenVideoBlackLevel)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetGreenVideoBlackLevel"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("DesiredGreenVideoBlackLevel"),
        desiredGreenVideoBlackLevel);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setGreenVideoBlackLevel));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getBlueVideoBlackLevel()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetBlueVideoBlackLevel"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getBlueVideoBlackLevel));
}

HClientAdapterOpNull HRenderingControlAdapter::setBlueVideoBlackLevel(
    quint16 desiredBlueVideoBlackLevel)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetBlueVideoBlackLevel"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("DesiredBlueVideoBlackLevel"), desiredBlueVideoBlackLevel);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setBlueVideoBlackLevel));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getColorTemperature()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetColorTemperature"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getColorTemperature));
}

HClientAdapterOpNull HRenderingControlAdapter::setColorTemperature(
    quint16 desiredColorTemperature)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetColorTemperature"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("DesiredColorTemperature"), desiredColorTemperature);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setColorTemperature));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getHorizontalKeystone()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetHorizontalKeystone"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getHorizontalKeystone));
}

HClientAdapterOpNull HRenderingControlAdapter::setHorizontalKeystone(
    qint16 desiredHorizontalKeyStone)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetHorizontalKeystone"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("DesiredHorizontalKeystone"), desiredHorizontalKeyStone);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setHorizontalKeystone));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getVerticalKeystone()
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetVerticalKeystone"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getVerticalKeystone));
}

HClientAdapterOpNull HRenderingControlAdapter::setVerticalKeystone(
    qint16 desiredVerticalKeyStone)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetVerticalKeystone"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("DesiredVerticalKeystone"), desiredVerticalKeyStone);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setVerticalKeystone));
}

HClientAdapterOp<bool> HRenderingControlAdapter::getMute(
    const HChannel& channel)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetMute"), &rc);
    if (!action)
    {
        return HClientAdapterOp<bool>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("Channel"), channel.toString());

    return h_ptr->beginInvoke<bool>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getMute));
}

HClientAdapterOpNull HRenderingControlAdapter::setMute(
    const HChannel& channel, bool desiredMute)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetMute"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("Channel"), channel.toString());
    inArgs.setValue(QLatin1String("DesiredMute"), desiredMute);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setMute));
}

HClientAdapterOp<quint16> HRenderingControlAdapter::getVolume(
    const HChannel& channel)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetVolume"), &rc);
    if (!action)
    {
        return HClientAdapterOp<quint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("Channel"), channel.toString());

    return h_ptr->beginInvoke<quint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getVolume));
}

HClientAdapterOpNull HRenderingControlAdapter::setVolume(
    const HChannel& channel, quint16 desiredVolume)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetVolume"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("Channel"), channel.toString());
    inArgs.setValue(QLatin1String("DesiredVolume"), desiredVolume);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setVolume));
}

HClientAdapterOp<qint16> HRenderingControlAdapter::getVolumeDB(const HChannel& channel)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetVolumeDB"), &rc);
    if (!action)
    {
        return HClientAdapterOp<qint16>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("Channel"), channel.toString());

    return h_ptr->beginInvoke<qint16>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getVolumeDB));
}

HClientAdapterOpNull HRenderingControlAdapter::setVolumeDB(
    const HChannel& channel, qint16 desiredVolumeDb)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetVolumeDB"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("Channel"), channel.toString());
    inArgs.setValue(QLatin1String("DesiredVolume"), desiredVolumeDb);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setVolumeDB));
}

HClientAdapterOp<HVolumeDbRangeResult> HRenderingControlAdapter::getVolumeDBRange(const HChannel& channel)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetVolumeDBRange"), &rc);
    if (!action)
    {
        return HClientAdapterOp<HVolumeDbRangeResult>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("Channel"), channel.toString());

    return h_ptr->beginInvoke<HVolumeDbRangeResult>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getVolumeDBRange));
}

HClientAdapterOp<bool> HRenderingControlAdapter::getLoudness(
    const HChannel& channel)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetLoudness"), &rc);
    if (!action)
    {
        return HClientAdapterOp<bool>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("Channel"), channel.toString());

    return h_ptr->beginInvoke<bool>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getLoudness));
}

HClientAdapterOpNull HRenderingControlAdapter::setLoudness(
    const HChannel& channel, bool loudnessOn)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetLoudness"), &rc);
    if (!action)
    {
        return HClientAdapterOpNull::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("Channel"), channel.toString());
    inArgs.setValue(QLatin1String("DesiredLoudness"), loudnessOn);

    return h_ptr->beginInvoke(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setLoudness));
}

HClientAdapterOp<QString> HRenderingControlAdapter::getStateVariables(
    const QSet<QString>& stateVariableNames)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("GetStateVariables"), &rc);
    if (!action)
    {
        return HClientAdapterOp<QString>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("StateVariableList"), QStringList(stateVariableNames.toList()).join(QLatin1String(",")));

    return h_ptr->beginInvoke<QString>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::getStateVariables));
}

HClientAdapterOp<QStringList> HRenderingControlAdapter::setStateVariables(
    const HUdn& renderingControlUdn,
    const HResourceType& serviceType, const HServiceId& serviceId,
    const QString& stateVariableValuePairs)
{
    qint32 rc = UpnpUndefinedFailure;
    HClientAction* action = h_ptr->getAction(QLatin1String("SetStateVariables"), &rc);
    if (!action)
    {
        return HClientAdapterOp<QStringList>::createInvalid(rc, QLatin1String(""));
    }

    H_D(HRenderingControlAdapter);
    HActionArguments inArgs = action->info().inputArguments();
    inArgs.setValue(QLatin1String("InstanceID"), h->m_instanceId);
    inArgs.setValue(QLatin1String("RenderingControlUDN"), renderingControlUdn.toString());
    inArgs.setValue(QLatin1String("ServiceType"), serviceType.toString());
    inArgs.setValue(QLatin1String("ServiceId"), serviceId.toString());
    inArgs.setValue(QLatin1String("StateVariableValuePairs"), stateVariableValuePairs);

    return h_ptr->beginInvoke<QStringList>(
        action, inArgs,
        HActionInvokeCallback(h, &HRenderingControlAdapterPrivate::setStateVariables));
}

}
}
}
