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

#include "habstractcontentdirectory_service.h"
#include "habstractcontentdirectory_service_p.h"

#include "hsearchresult.h"
#include "hcreateobjectresult.h"
#include "hfreeformqueryresult.h"
#include "htransferprogressinfo.h"

#include "../common/hprotocolinfo.h"
#include "../cds_model/hsortinfo.h"

#include <../../HUpnpCore/private/hlogger_p.h>

#include <../../HUpnpCore/HServerStateVariable>

#include <QtCore/QSet>
#include <QtCore/QUuid>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HAbstractContentDirectoryServicePrivate
 ******************************************************************************/
HAbstractContentDirectoryServicePrivate::HAbstractContentDirectoryServicePrivate() :
    HServerServicePrivate()
{
}

HAbstractContentDirectoryServicePrivate::~HAbstractContentDirectoryServicePrivate()
{
}

qint32 HAbstractContentDirectoryServicePrivate::getSearchCapabilities(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QStringList searchCapabilities;
    qint32 retVal = q->getSearchCapabilities(&searchCapabilities);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("SearchCaps"), searchCapabilities.join(QLatin1String(",")));
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::getSortCapabilities(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QStringList sortCapabilities;
    qint32 retVal = q->getSortCapabilities(&sortCapabilities);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("SortCaps"), sortCapabilities.join(QLatin1String(",")));
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::getSortExtensionCapabilities(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QStringList sortExtCapabilities;
    qint32 retVal = q->getSortExtensionCapabilities(&sortExtCapabilities);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("SortExtensionCaps"), sortExtCapabilities.join(QLatin1String(",")));
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::getFeatureList(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QString featureList;
    qint32 retVal = q->getFeatureList(&featureList);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("FeatureList"), featureList);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::getSystemUpdateID(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    quint32 systemUpdateId;
    qint32 retVal = q->getSystemUpdateId(&systemUpdateId);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("Id"), systemUpdateId);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::getServiceResetToken(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QString serviceResetToken;
    qint32 retVal = q->getServiceResetToken(&serviceResetToken);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("ResetToken"), serviceResetToken);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::browse(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HSearchResult result;
    qint32 retVal = q->browse(
           inArgs.value(QLatin1String("ObjectID")).toString(),
           HContentDirectoryInfo::browseFlagFromString(inArgs.value(QLatin1String("BrowseFlag")).toString()),
           inArgs.value(QLatin1String("Filter")).toString().split(QLatin1Char(',')).toSet(),
           inArgs.value(QLatin1String("StartingIndex")).toUInt(),
           inArgs.value(QLatin1String("RequestedCount")).toUInt(),
           inArgs.value(QLatin1String("SortCriteria")).toString().split(QLatin1Char(','), QString::SkipEmptyParts),
           &result);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("Result"), result.result());
        outArgs->setValue(QLatin1String("NumberReturned"), result.numberReturned());
        outArgs->setValue(QLatin1String("TotalMatches"), result.totalMatches());
        outArgs->setValue(QLatin1String("UpdateID"), result.updateId());
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::search(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HSearchResult result;
    qint32 retVal = q->search(
          inArgs.value(QLatin1String("ContainerID")).toString(),
          inArgs.value(QLatin1String("SearchCriteria")).toString(),
          inArgs.value(QLatin1String("Filter")).toString().split(QLatin1Char(',')).toSet(),
          inArgs.value(QLatin1String("StartingIndex")).toUInt(),
          inArgs.value(QLatin1String("RequestedCount")).toUInt(),
          inArgs.value(QLatin1String("SortCriteria")).toString().split(QLatin1Char(','), QString::SkipEmptyParts),
          &result);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("Result"), result.result());
        outArgs->setValue(QLatin1String("NumberReturned"), result.numberReturned());
        outArgs->setValue(QLatin1String("TotalMatches"), result.totalMatches());
        outArgs->setValue(QLatin1String("UpdateID"), result.updateId());
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::createObject(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HCreateObjectResult result;
    qint32 retVal = q->createObject(
        inArgs.value(QLatin1String("ContainerID")).toString(),
        inArgs.value(QLatin1String("Elements")).toString(),
        &result);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("ObjectID"), result.objectId());
        outArgs->setValue(QLatin1String("Result"), result.result());
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::destroyObject(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);
    return q->destroyObject(inArgs.value(QLatin1String("ObjectID")).toString());
}

qint32 HAbstractContentDirectoryServicePrivate::updateObject(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);
    return q->updateObject(
        inArgs.value(QLatin1String("ObjectID")).toString(),
        inArgs.value(QLatin1String("CurrentTagValue")).toString().split(QLatin1Char(',')),
        inArgs.value(QLatin1String("NewTagValue")).toString().split(QLatin1Char(',')));
}

qint32 HAbstractContentDirectoryServicePrivate::moveObject(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QString newObjectId;
    qint32 retVal = q->moveObject(
        inArgs.value(QLatin1String("ObjectID")).toString(),
        inArgs.value(QLatin1String("NewParentID")).toString(),
        &newObjectId);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("NewObjectId"), newObjectId);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::importResource(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    quint32 transferId;
    qint32 retVal = q->importResource(
        inArgs.value(QLatin1String("SourceURI")).toUrl(),
        inArgs.value(QLatin1String("DestinationURI")).toUrl(),
        &transferId);

    if (retVal == UpnpSuccess && outArgs)
    {
        outArgs->setValue(QLatin1String("TransferID"), transferId);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::exportResource(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    quint32 transferId;
    qint32 retVal = q->exportResource(
        inArgs.value(QLatin1String("SourceURI")).toUrl(),
        inArgs.value(QLatin1String("DestinationURI")).toUrl(),
        &transferId);

    if (retVal == UpnpSuccess && outArgs)
    {
        outArgs->setValue(QLatin1String("TransferID"), transferId);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::deleteResource(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);
    return q->deleteResource(inArgs.value(QLatin1String("ResourceURI")).toUrl());
}

qint32 HAbstractContentDirectoryServicePrivate::stopTransferResource(
    const HActionArguments& inArgs, HActionArguments* /*outArgs*/)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);
    return q->stopTransferResource(inArgs.value(QLatin1String("TransferID")).toUInt());
}

qint32 HAbstractContentDirectoryServicePrivate::getTransferProgress(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HTransferProgressInfo info;
    qint32 retVal = q->getTransferProgress(inArgs.value(QLatin1String("TransferID")).toUInt(), &info);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("TransferStatus"), info.status());
        outArgs->setValue(QLatin1String("TransferLength"), info.length());
        outArgs->setValue(QLatin1String("TransferTotal"), info.total());
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::createReference(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QString newId;
    qint32 retVal = q->createReference(
       inArgs.value(QLatin1String("ContainerID")).toString(),
       inArgs.value(QLatin1String("ObjectID")).toString(),
       &newId);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("NewID"), newId);
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::freeFormQuery(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    HFreeFormQueryResult queryResult;
    qint32 retVal = q->freeFormQuery(
        inArgs.value(QLatin1String("ContainerID")).toString(),
        inArgs.value(QLatin1String("CDSView")).toUInt(),
        inArgs.value(QLatin1String("QueryRequest")).toString(),
        &queryResult);

    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("QueryResult"), queryResult.queryResult());
        outArgs->setValue(QLatin1String("UpdateID"), queryResult.updateId());
    }

    return retVal;
}

qint32 HAbstractContentDirectoryServicePrivate::getFreeFormQueryCapabilities(
    const HActionArguments& /*inArgs*/, HActionArguments* outArgs)
{
    HLOG2(H_AT, H_FUN, (char*) (m_loggingIdentifier.data()));
    H_Q(HAbstractContentDirectoryService);

    Q_ASSERT_X(outArgs, "", "An object for output arguments have to be defined");

    QString ffqCapabilities;
    qint32 retVal = q->getFreeFormQueryCapabilities(&ffqCapabilities);
    if (retVal == UpnpSuccess)
    {
        outArgs->setValue(QLatin1String("FFQCapabilities"), ffqCapabilities);
    }

    return retVal;
}

/*******************************************************************************
 * HAbstractContentDirectoryService
 ******************************************************************************/
HAbstractContentDirectoryService::HAbstractContentDirectoryService(
    HAbstractContentDirectoryServicePrivate& dd) :
        HServerService(dd)
{
}

HAbstractContentDirectoryService::HAbstractContentDirectoryService() :
    HServerService(*new HAbstractContentDirectoryServicePrivate())
{
}

HAbstractContentDirectoryService::~HAbstractContentDirectoryService()
{
    HLOG2(H_AT, H_FUN, (char *)h_ptr->m_loggingIdentifier.data());
}

HServerService::HActionInvokes HAbstractContentDirectoryService::createActionInvokes()
{
    HLOG2(H_AT, (char *)H_FUN, h_ptr->m_loggingIdentifier.data());
    H_D(HAbstractContentDirectoryService);

    HActionInvokes retVal;

    retVal.insert(QLatin1String("GetSearchCapabilities"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getSearchCapabilities));

    retVal.insert(QLatin1String("GetSortCapabilities"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getSortCapabilities));

    retVal.insert(QLatin1String("GetSortExtensionCapabilities"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getSortExtensionCapabilities));

    retVal.insert(QLatin1String("GetFeatureList"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getFeatureList));

    retVal.insert(QLatin1String("GetSystemUpdateID"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getSystemUpdateID));

    retVal.insert(QLatin1String("GetServiceResetToken"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getServiceResetToken));

    retVal.insert(QLatin1String("Browse"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::browse));

    retVal.insert(QLatin1String("Search"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::search));

    retVal.insert(QLatin1String("CreateObject"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::createObject));

    retVal.insert(QLatin1String("DestroyObject"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::destroyObject));

    retVal.insert(QLatin1String("UpdateObject"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::updateObject));

    retVal.insert(QLatin1String("MoveObject"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::moveObject));

    retVal.insert(QLatin1String("ImportResource"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::importResource));

    retVal.insert(QLatin1String("ExportResource"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::exportResource));

    retVal.insert(QLatin1String("DeleteResource"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::deleteResource));

    retVal.insert(QLatin1String("StopTransferResource"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::stopTransferResource));

    retVal.insert(QLatin1String("GetTransferProgress"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getTransferProgress));

    retVal.insert(QLatin1String("CreateReference"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::createReference));

    retVal.insert(QLatin1String("FreeFormQuery"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::freeFormQuery));

    retVal.insert(QLatin1String("GetFreeFormQueryCapabilities"),
        HActionInvoke(h, &HAbstractContentDirectoryServicePrivate::getFreeFormQueryCapabilities));

    return retVal;
}

bool HAbstractContentDirectoryService::finalizeInit(QString* errDescription)
{
    if (!HServerService::finalizeInit(errDescription))
    {
        return false;
    }

    stateVariables().value(QLatin1String("ServiceResetToken"))->setValue(
        QUuid::createUuid().toString().remove(QLatin1String("{")).remove(QLatin1String("}")));

    return true;
}

qint32 HAbstractContentDirectoryService::getSortExtensionCapabilities(
    QStringList* /*oarg*/) const
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::search(
    const QString& /*containerId*/,
    const QString& /*searchCriteria*/,
    const QSet<QString>& /*filter*/,
    quint32 /*startingIndex*/,
    quint32 /*requestedCount*/,
    const QStringList& /*sortCriteria*/,
    HSearchResult* /*result*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::createObject(
    const QString& /*containerId*/,
    const QString& /*elements*/,
    HCreateObjectResult*)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::destroyObject(const QString& /*objectId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::updateObject(
    const QString& /*objectId*/,
    const QStringList& /*currentTagValues*/,
    const QStringList& /*newTagValues*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::moveObject(
    const QString& /*objectId*/,
    const QString& /*newParentId*/,
    QString* /*newObjectId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::importResource(
    const QUrl& /*source*/, const QUrl& /*destination*/, quint32* /*transferId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::exportResource(
    const QUrl& /*source*/, const QUrl& /*destination*/, quint32* /*transferId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::deleteResource(const QUrl& /*resourceUrl*/)
{return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::stopTransferResource(quint32 /*transferId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::getTransferProgress(
    quint32 /*transferId*/, HTransferProgressInfo* /*transferInfo*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::createReference(
    const QString& /*containerId*/, const QString& /*objectId*/,
    QString* /*newId*/)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::freeFormQuery(
    const QString& /*containerId*/, quint32 /*cdsView*/,
    const QString& /*queryRequest*/, HFreeFormQueryResult*)
{
    return UpnpOptionalActionNotImplemented;
}

qint32 HAbstractContentDirectoryService::getFreeFormQueryCapabilities(
    QString* /*ffqCapabilities*/)
{
    return UpnpOptionalActionNotImplemented;
}

}
}
}
