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

#include "havtransport_info.h"

#include <../../HUpnpCore/HActionSetup>
#include <../../HUpnpCore/HResourceType>
#include <../../HUpnpCore/HActionArguments>
#include <../../HUpnpCore/HActionsSetupData>
#include <../../HUpnpCore/HStateVariablesSetupData>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \defgroup hupnp_av_avt AVTransport
 * \ingroup hupnp_av
 *
 * \brief This page discusses the design and use of the HUPnPAv's AV Transport
 * functionality.
 */

/*******************************************************************************
 * HAvTransportInfo
 ******************************************************************************/
HAvTransportInfo::HAvTransportInfo()
{
}

HAvTransportInfo::~HAvTransportInfo()
{
}

QString HAvTransportInfo::drmStateToString(DrmState state)
{
    QString retVal;
    switch(state)
    {
    case DrmState_Ok:
        retVal = QLatin1String("OK");
        break;
    case DrmState_Unknown:
        retVal = QLatin1String("UNKNOWN");
        break;
    case DrmState_ProcessingContentKey:
        retVal = QLatin1String("PROCESSING_CONTENT_KEY");
        break;
    case DrmState_ContentKeyFailure:
        retVal = QLatin1String("CONTENT_KEY_FAILURE");
        break;
    case DrmState_AttemptingAuthentication:
        retVal = QLatin1String("ATTEMPTING_AUTHENTICATION");
        break;
    case DrmState_FailedAuthentication:
        retVal = QLatin1String("FAILED_AUTHENTICATION");
        break;
    case DrmState_NotAuthenticated:
        retVal = QLatin1String("NOT_AUTHENTICATED");
        break;
    case DrmState_DeviceRevocation:
        retVal = QLatin1String("DEVICE_REVOCATION");
        break;
    default:
        break;
    }
    return retVal;
}

HAvTransportInfo::DrmState
    HAvTransportInfo::drmStateFromString(const QString& state)
{
    DrmState retVal = DrmState_Unknown;
    if (state.compare(QLatin1String("OK"), Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_Ok;
    }
    else if (state.compare(QLatin1String("UNKNOWN"), Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_Unknown;
    }
    else if (state.compare(QLatin1String("PROCESSING_CONTENT_KEY"), Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_ProcessingContentKey;
    }
    else if (state.compare(QLatin1String("CONTENT_KEY_FAILURE"), Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_ContentKeyFailure;
    }
    else if (state.compare(QLatin1String("ATTEMPTING_AUTHENTICATION"), Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_AttemptingAuthentication;
    }
    else if (state.compare(QLatin1String("FAILED_AUTHENTICATION"), Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_FailedAuthentication;
    }
    else if (state.compare(QLatin1String("NOT_AUTHENTICATED"), Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_NotAuthenticated;
    }
    else if (state.compare(QLatin1String("DEVICE_REVOCATION"), Qt::CaseInsensitive) == 0)
    {
        retVal = DrmState_DeviceRevocation;
    }
    return retVal;
}

const HResourceType& HAvTransportInfo::supportedServiceType()
{
    static HResourceType retVal(QLatin1String("urn:schemas-upnp-org:service:AVTransport:2"));
    return retVal;
}

HActionsSetupData HAvTransportInfo::actionsSetupData()
{
    HActionsSetupData retVal;

    HStateVariablesSetupData svSetupData = stateVariablesSetupData();

    HActionArguments inArgsSetup, outArgsSetup;

    // SetAVTransportURI
    HActionSetup actionSetup(QLatin1String("SetAVTransportURI"), InclusionMandatory);
    inArgsSetup.append(HActionArgument(QLatin1String("InstanceID"), svSetupData.get(QLatin1String("A_ARG_TYPE_InstanceID"))));
    inArgsSetup.append(HActionArgument(QLatin1String("CurrentURI"), svSetupData.get(QLatin1String("AVTransportURI"))));
    inArgsSetup.append(HActionArgument(QLatin1String("CurrentURIMetaData"), svSetupData.get(QLatin1String("AVTransportURIMetaData"))));
    actionSetup.setInputArguments(inArgsSetup);
    retVal.insert(actionSetup);

    // SetNextAVTransportURI
    inArgsSetup.clear();
    actionSetup = HActionSetup(QLatin1String("SetNextAVTransportURI"), InclusionOptional);
    inArgsSetup.append(HActionArgument(QLatin1String("InstanceID"), svSetupData.get(QLatin1String("A_ARG_TYPE_InstanceID"))));
    inArgsSetup.append(HActionArgument(QLatin1String("NextURI"), svSetupData.get(QLatin1String("AVTransportURI"))));
    inArgsSetup.append(HActionArgument(QLatin1String("NextURIMetaData"), svSetupData.get(QLatin1String("AVTransportURIMetaData"))));
    actionSetup.setInputArguments(inArgsSetup);
    retVal.insert(actionSetup);

    // GetMediaInfo
    inArgsSetup.clear();
    outArgsSetup.clear();
    actionSetup = HActionSetup(QLatin1String("GetMediaInfo"));
    inArgsSetup.append(HActionArgument(QLatin1String("InstanceID"), svSetupData.get(QLatin1String("A_ARG_TYPE_InstanceID"))));
    actionSetup.setInputArguments(inArgsSetup);
    outArgsSetup.append(HActionArgument(QLatin1String("NrTracks"), svSetupData.get(QLatin1String("NumberOfTracks"))));
    outArgsSetup.append(HActionArgument(QLatin1String("MediaDuration"), svSetupData.get(QLatin1String("CurrentMediaDuration"))));
    outArgsSetup.append(HActionArgument(QLatin1String("CurrentURI"), svSetupData.get(QLatin1String("AVTransportURI"))));
    outArgsSetup.append(HActionArgument(QLatin1String("CurrentURIMetaData"), svSetupData.get(QLatin1String("AVTransportURIMetaData"))));
    outArgsSetup.append(HActionArgument(QLatin1String("NextURI"), svSetupData.get(QLatin1String("NextAVTransportURI"))));
    outArgsSetup.append(HActionArgument(QLatin1String("NextURIMetaData"), svSetupData.get(QLatin1String("NextAVTransportURIMetaData"))));
    outArgsSetup.append(HActionArgument(QLatin1String("PlayMedium"), svSetupData.get(QLatin1String("PlaybackStorageMedium"))));
    outArgsSetup.append(HActionArgument(QLatin1String("RecordMedium"), svSetupData.get(QLatin1String("RecordStorageMedium"))));
    outArgsSetup.append(HActionArgument(QLatin1String("WriteStatus"), svSetupData.get(QLatin1String("RecordMediumWriteStatus"))));
    actionSetup.setOutputArguments(outArgsSetup);
    retVal.insert(actionSetup);
    //

    // GetMediaInfo_Ext
    actionSetup = HActionSetup(QLatin1String("GetMediaInfo_Ext"), 2);
    actionSetup.setInputArguments(inArgsSetup);
    outArgsSetup.append(HActionArgument(QLatin1String("CurrentType"), svSetupData.get(QLatin1String("CurrentMediaCategory"))));
    actionSetup.setOutputArguments(outArgsSetup);
    retVal.insert(actionSetup);
    //

    // GetTransportInfo
    outArgsSetup.clear();
    actionSetup = HActionSetup(QLatin1String("GetTransportInfo"));
    actionSetup.setInputArguments(inArgsSetup);
    outArgsSetup.append(HActionArgument(QLatin1String("CurrentTransportState"), svSetupData.get(QLatin1String("TransportState"))));
    outArgsSetup.append(HActionArgument(QLatin1String("CurrentTransportStatus"), svSetupData.get(QLatin1String("TransportStatus"))));
    outArgsSetup.append(HActionArgument(QLatin1String("CurrentSpeed"), svSetupData.get(QLatin1String("TransportPlaySpeed"))));
    actionSetup.setOutputArguments(outArgsSetup);
    retVal.insert(actionSetup);

    // GetPositionInfo
    outArgsSetup.clear();
    actionSetup = HActionSetup(QLatin1String("GetPositionInfo"));
    actionSetup.setInputArguments(inArgsSetup);
    outArgsSetup.append(HActionArgument(QLatin1String("Track"), svSetupData.get(QLatin1String("CurrentTrack"))));
    outArgsSetup.append(HActionArgument(QLatin1String("TrackDuration"), svSetupData.get(QLatin1String("CurrentTrackDuration"))));
    outArgsSetup.append(HActionArgument(QLatin1String("TrackMetaData"), svSetupData.get(QLatin1String("CurrentTrackMetaData"))));
    outArgsSetup.append(HActionArgument(QLatin1String("TrackURI"), svSetupData.get(QLatin1String("CurrentTrackURI"))));
    outArgsSetup.append(HActionArgument(QLatin1String("RelTime"), svSetupData.get(QLatin1String("RelativeTimePosition"))));
    outArgsSetup.append(HActionArgument(QLatin1String("AbsTime"), svSetupData.get(QLatin1String("AbsoluteTimePosition"))));
    outArgsSetup.append(HActionArgument(QLatin1String("RelCount"), svSetupData.get(QLatin1String("RelativeCounterPosition"))));
    outArgsSetup.append(HActionArgument(QLatin1String("AbsCount"), svSetupData.get(QLatin1String("AbsoluteCounterPosition"))));
    actionSetup.setOutputArguments(outArgsSetup);
    retVal.insert(actionSetup);

    // GetDeviceCapabilities
    retVal.insert(HActionSetup(QLatin1String("GetDeviceCapabilities")));

    // GetTransportSettings
    retVal.insert(HActionSetup(QLatin1String("GetTransportSettings")));

    // Stop
    retVal.insert(HActionSetup(QLatin1String("Stop")));

    // Play
    retVal.insert(HActionSetup(QLatin1String("Play")));

    // Pause
    retVal.insert(HActionSetup(QLatin1String("Pause", InclusionOptional)));

    // Record
    retVal.insert(HActionSetup(QLatin1String("Record", InclusionOptional)));

    // Seek
    retVal.insert(HActionSetup(QLatin1String("Seek")));

    // Next
    retVal.insert(HActionSetup(QLatin1String("Next")));

    // Previous
    retVal.insert(HActionSetup(QLatin1String("Previous")));

    // SetPlayMode
    retVal.insert(HActionSetup(QLatin1String("SetPlayMode", InclusionOptional)));

    // SetRecordQualityMode
    retVal.insert(HActionSetup(QLatin1String("SetRecordQualityMode", InclusionOptional)));

    // GetCurrentTransportActions
    retVal.insert(HActionSetup(QLatin1String("GetCurrentTransportActions", InclusionOptional)));

    // GetDRMState
    retVal.insert(HActionSetup(QLatin1String("GetDRMState"), 2, InclusionOptional));

    // GetStateVariables
    retVal.insert(HActionSetup(QLatin1String("GetStateVariables"), 2, InclusionOptional));

    // SetStateVariables
    retVal.insert(HActionSetup(QLatin1String("SetStateVariables"), 2, InclusionOptional));

    return retVal;
}

HStateVariablesSetupData HAvTransportInfo::stateVariablesSetupData()
{
    HStateVariablesSetupData retVal;

    retVal.insert(HStateVariableInfo(QLatin1String("TransportState"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("TransportStatus"), HUpnpDataTypes::string));

    HStateVariableInfo setupData(QLatin1String("CurrentMediaCategory"), HUpnpDataTypes::string);
    setupData.setVersion(2);
    retVal.insert(setupData);

    retVal.insert(HStateVariableInfo(QLatin1String("PlaybackStorageMedium"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("RecordStorageMedium"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("PossiblePlaybackStorageMedia"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("PossibleRecordStorageMedia"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("CurrentPlayMode"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("TransportPlaySpeed"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("RecordMediumWriteStatus"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("CurrentRecordQualityMode"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("PossibleRecordQualityModes"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("NumberOfTracks"), HUpnpDataTypes::ui4));
    retVal.insert(HStateVariableInfo(QLatin1String("CurrentTrack"), HUpnpDataTypes::ui4));
    retVal.insert(HStateVariableInfo(QLatin1String("CurrentTrackDuration"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("CurrentMediaDuration"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("CurrentTrackMetaData"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("CurrentTrackURI"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("AVTransportURI"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("AVTransportURIMetaData"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("NextAVTransportURI"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("NextAVTransportURIMetaData"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("RelativeTimePosition"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("AbsoluteTimePosition"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("RelativeCounterPosition"), HUpnpDataTypes::i4));
    retVal.insert(HStateVariableInfo(QLatin1String("AbsoluteCounterPosition"), HUpnpDataTypes::ui4));
    retVal.insert(HStateVariableInfo(QLatin1String("CurrentTransportActions"), HUpnpDataTypes::string, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("LastChange"), HUpnpDataTypes::string));

    setupData = HStateVariableInfo(QLatin1String("DRMState"), HUpnpDataTypes::string, InclusionOptional);
    setupData.setVersion(2);
    retVal.insert(setupData);

    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_SeekMode"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_SeekTarget"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_InstanceID"), HUpnpDataTypes::ui4));

    setupData = HStateVariableInfo(QLatin1String("A_ARG_TYPE_DeviceUDN"), HUpnpDataTypes::string, InclusionOptional);
    setupData.setVersion(2);
    retVal.insert(setupData);

    setupData = HStateVariableInfo(QLatin1String("A_ARG_TYPE_ServiceType"), HUpnpDataTypes::string, InclusionOptional);
    setupData.setVersion(2);
    retVal.insert(setupData);

    setupData = HStateVariableInfo(QLatin1String("A_ARG_TYPE_ServiceID"), HUpnpDataTypes::string, InclusionOptional);
    setupData.setVersion(2);
    retVal.insert(setupData);

    setupData = HStateVariableInfo(QLatin1String("A_ARG_TYPE_StateVariableValuePairs"), HUpnpDataTypes::string, InclusionOptional);
    setupData.setVersion(2);
    retVal.insert(setupData);

    setupData = HStateVariableInfo(QLatin1String("A_ARG_TYPE_StateVariableList"), HUpnpDataTypes::string, InclusionOptional);
    setupData.setVersion(2);
    retVal.insert(setupData);

    return retVal;
}

}
}
}
