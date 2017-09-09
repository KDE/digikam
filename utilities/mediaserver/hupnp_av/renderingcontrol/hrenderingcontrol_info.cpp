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

#include "hrenderingcontrol_info.h"

#include "hresourcetype.h"
#include "hactions_setupdata.h"
#include "hstatevariables_setupdata.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \defgroup hupnp_av_rcs RenderingControl
 * \ingroup hupnp_av
 *
 * \brief This page discusses the design and use of the HUPnPAv's RenderingControl
 * functionality.
 */

/*******************************************************************************
 * HRenderingControlInfo
 ******************************************************************************/
HRenderingControlInfo::HRenderingControlInfo()
{
}

HRenderingControlInfo::~HRenderingControlInfo()
{
}

const HResourceType& HRenderingControlInfo::supportedServiceType()
{
    static const HResourceType retVal(QLatin1String("urn:schemas-upnp-org:service:RenderingControl:2"));
    return retVal;
}

HActionsSetupData HRenderingControlInfo::actionsSetupData()
{
    HActionsSetupData retVal;

    retVal.insert(HActionSetup(QLatin1String("ListPresets")));
    retVal.insert(HActionSetup(QLatin1String("SelectPreset")));
    retVal.insert(HActionSetup(QLatin1String("GetBrightness"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetBrightness"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetContrast"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetContrast"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetSharpness"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetSharpness"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetRedVideoGain"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetRedVideoGain"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetGreenVideoGain"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetGreenVideoGain"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetBlueVideoGain"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetBlueVideoGain"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetRedVideoBlackLevel"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetRedVideoBlackLevel"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetGreenVideoBlackLevel"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetGreenVideoBlackLevel"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetBlueVideoBlackLevel"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetBlueVideoBlackLevel"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetColorTemperature"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetColorTemperature"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetHorizontalKeystone"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetHorizontalKeystone"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetVerticalKeystone"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetVerticalKeystone"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetMute"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetMute"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetVolume"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetVolume"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetVolumeDB"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetVolumeDB"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetVolumeDBRange"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetLoudness"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("SetLoudness"), InclusionOptional));

    HActionSetup setup(QLatin1String("GetStateVariables"), InclusionOptional);
    setup.setVersion(2);
    retVal.insert(setup);

    setup = HActionSetup(QLatin1String("SetStateVariables"), InclusionOptional);
    setup.setVersion(2);
    retVal.insert(setup);

    return retVal;
}

HStateVariablesSetupData HRenderingControlInfo::stateVariablesSetupData()
{
    HStateVariablesSetupData retVal;

    retVal.insert(HStateVariableInfo(QLatin1String("LastChange"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("PresetNameList"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("Brightness"), HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("Contrast"), HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("Sharpness"), HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("RedVideoGain"), HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("GreenVideoGain"), HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("BlueVideoGain"), HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("RedVideoBlackLevel"), HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("BlueVideoBlackLevel"), HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("ColoTemperature"), HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("HorizontalKeystone"), HUpnpDataTypes::i2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("VerticalKeystone"), HUpnpDataTypes::i2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("Mute"), HUpnpDataTypes::boolean, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("Volume"), HUpnpDataTypes::ui2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("VolumeDB"), HUpnpDataTypes::i2, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("Loudness"), HUpnpDataTypes::boolean, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_Channel"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_InstanceID"), HUpnpDataTypes::ui4));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_PresetName"), HUpnpDataTypes::string));

    HStateVariableInfo info(QLatin1String("A_ARG_TYPE_DeviceUDN"), HUpnpDataTypes::string, InclusionOptional);
    info.setVersion(2);
    retVal.insert(info);

    info = HStateVariableInfo(QLatin1String("A_ARG_TYPE_ServiceType"), HUpnpDataTypes::string, InclusionOptional);
    info.setVersion(2);
    retVal.insert(info);

    info = HStateVariableInfo(QLatin1String("A_ARG_TYPE_ServiceID"), HUpnpDataTypes::string, InclusionOptional);
    info.setVersion(2);
    retVal.insert(info);

    info = HStateVariableInfo(QLatin1String("A_ARG_TYPE_StateVariableValuePairs"), HUpnpDataTypes::string, InclusionOptional);
    info.setVersion(2);
    retVal.insert(info);

    info = HStateVariableInfo(QLatin1String("A_ARG_TYPE_StateVariableList"), HUpnpDataTypes::string, InclusionOptional);
    info.setVersion(2);
    retVal.insert(info);

    return retVal;
}

}
}
}
