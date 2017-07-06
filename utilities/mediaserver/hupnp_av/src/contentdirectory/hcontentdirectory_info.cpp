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

#include "hcontentdirectory_info.h"

#include <../../HUpnpCore/HResourceType>
#include <../../HUpnpCore/HActionArguments>
#include <../../HUpnpCore/HActionsSetupData>
#include <../../HUpnpCore/HStateVariablesSetupData>

#include <QtCore/QString>

/*!
 * \defgroup hupnp_av_cds Content Directory
 * \ingroup hupnp_av
 *
 * \brief This page discusses the design and use of the HUPnPAv's ContentDirectory
 * functionality.
 */

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HContentDirectoryInfo
 ******************************************************************************/
HContentDirectoryInfo::HContentDirectoryInfo()
{
}

HContentDirectoryInfo::~HContentDirectoryInfo()
{
}

QString HContentDirectoryInfo::browseFlagToString(BrowseFlag flag)
{
    QString retVal = QLatin1String("");
    switch(flag)
    {
    case Undefined:
        break;;
    case BrowseMetadata:
        retVal = QLatin1String("BrowseMetadata");
        break;
    case BrowseDirectChildren:
        retVal = QLatin1String("BrowseDirectChildren");
        break;
    default:
        Q_ASSERT(false);
        break;
    };
    return retVal;
}

HContentDirectoryInfo::BrowseFlag
    HContentDirectoryInfo::browseFlagFromString(const QString& arg)
{
    BrowseFlag retVal = Undefined;
    if (arg.compare(QLatin1String("BrowseMetadata"), Qt::CaseInsensitive) == 0)
    {
        retVal = BrowseMetadata;
    }
    else if (arg.compare(QLatin1String("BrowseDirectChildren"), Qt::CaseInsensitive) == 0)
    {
        retVal = BrowseDirectChildren;
    }
    return retVal;
}

const HResourceType& HContentDirectoryInfo::supportedServiceType()
{
    static HResourceType retVal(QLatin1String(
        "urn:schemas-upnp-org:service:ContentDirectory:3"));

    return retVal;
}

HActionsSetupData HContentDirectoryInfo::actionsSetupData()
{
    HActionsSetupData retVal;

    retVal.insert(HActionSetup(QLatin1String("GetSearchCapabilities")));
    retVal.insert(HActionSetup(QLatin1String("GetSortCapabilities")));

    HActionSetup setup(QLatin1String("GetSortExtensionCapabilities"), InclusionOptional);
    setup.setVersion(2);
    retVal.insert(setup);

    setup = HActionSetup(QLatin1String("GetFeatureList"));
    setup.setVersion(2);
    retVal.insert(setup);

    retVal.insert(HActionSetup(QLatin1String("GetSystemUpdateID")));

    setup = HActionSetup(QLatin1String("GetServiceResetToken"));
    setup.setVersion(3);
    retVal.insert(setup);

    retVal.insert(HActionSetup(QLatin1String("Browse")));
    retVal.insert(HActionSetup(QLatin1String("Search"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("CreateObject"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("DestroyObject"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("UpdateObject"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("MoveObject"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("ImportResource"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("ExportResource"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("DeleteResource"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("StopTransferResource"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("GetTransferProgress"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("CreateReference"), InclusionOptional));

    setup = HActionSetup(QLatin1String("FreeFormQuery"), InclusionOptional);
    setup.setVersion(3);
    retVal.insert(setup);

    setup = HActionSetup(QLatin1String("GetFreeFormQueryCapabilities"), InclusionOptional);
    setup.setVersion(3);
    retVal.insert(setup);

    return retVal;
}

HStateVariablesSetupData HContentDirectoryInfo::stateVariablesSetupData()
{
    HStateVariablesSetupData retVal;

    retVal.insert(HStateVariableInfo(QLatin1String("SearchCapabilities"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("SortCapabilities"), HUpnpDataTypes::string));

    HStateVariableInfo svInfo(QLatin1String("SortExtensionCapabilities"), HUpnpDataTypes::string, InclusionOptional);
    svInfo.setVersion(2);
    retVal.insert(svInfo);

    retVal.insert(HStateVariableInfo(QLatin1String("SystemUpdateID"), HUpnpDataTypes::ui4));
    retVal.insert(HStateVariableInfo(QLatin1String("ContainerUpdateIDs"), HUpnpDataTypes::string, InclusionOptional));

    svInfo = HStateVariableInfo(QLatin1String("ServiceResetToken"), HUpnpDataTypes::string);
    svInfo.setVersion(3);
    retVal.insert(svInfo);

    svInfo = HStateVariableInfo(QLatin1String("LastChange"), HUpnpDataTypes::string, InclusionOptional);
    svInfo.setVersion(3);
    retVal.insert(svInfo);

    retVal.insert(HStateVariableInfo(QLatin1String("TransferIDs"), HUpnpDataTypes::string, InclusionOptional));

    svInfo = HStateVariableInfo(QLatin1String("FeatureList"), HUpnpDataTypes::string);
    svInfo.setVersion(2);
    retVal.insert(svInfo);

    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_ObjectID"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_Result"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_SearchCriteria"), HUpnpDataTypes::string, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_BrowseFlag"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_Filter"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_SortCriteria"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_Index"), HUpnpDataTypes::ui4));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_Count"), HUpnpDataTypes::ui4));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_UpdateID"), HUpnpDataTypes::ui4));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_TransferID"), HUpnpDataTypes::ui4, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_TransferStatus"), HUpnpDataTypes::string, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_TransferLength"), HUpnpDataTypes::string, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_TransferTotal"), HUpnpDataTypes::string, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_TagValueList"), HUpnpDataTypes::string, InclusionOptional));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_URI"), HUpnpDataTypes::uri, InclusionOptional));

    svInfo = HStateVariableInfo(QLatin1String("A_ARG_TYPE_CDSView"), HUpnpDataTypes::ui4, InclusionOptional);
    svInfo.setVersion(3);
    retVal.insert(svInfo);

    svInfo = HStateVariableInfo(QLatin1String("A_ARG_TYPE_TagValueList"), HUpnpDataTypes::string, InclusionOptional);
    svInfo.setVersion(3);
    retVal.insert(svInfo);

    svInfo = HStateVariableInfo(QLatin1String("A_ARG_TYPE_QueryRequest"), HUpnpDataTypes::string, InclusionOptional);
    svInfo.setVersion(3);
    retVal.insert(svInfo);

    svInfo = HStateVariableInfo(QLatin1String("A_ARG_TYPE_QueryResult"), HUpnpDataTypes::string, InclusionOptional);
    svInfo.setVersion(3);
    retVal.insert(svInfo);

    svInfo = HStateVariableInfo(QLatin1String("A_ARG_TYPE_FFQCapabilities"), HUpnpDataTypes::string, InclusionOptional);
    svInfo.setVersion(3);
    retVal.insert(svInfo);

    return retVal;
}

}
}
}
