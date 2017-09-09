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

#include "hcdsproperties.h"

#include "hstatevariablecollection.h"
#include "hcdspropertyinfo.h"
#include "hchannelgroupname.h"
#include "hforeignmetadata.h"
#include "hcontentduration.h"
#include "hscheduledtime.h"
#include "hdatetimerange.h"
#include "hprogramcode.h"
#include "hmatching_id.h"
#include "hdeviceudn.h"
#include "hradioband.h"
#include "hchannel_id.h"
#include "hobject.h"
#include "hprice.h"

#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QMutexLocker>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

HCdsProperties* HCdsProperties::s_instance = 0;
QMutex* HCdsProperties::s_instanceLock = new QMutex();

class HCdsPropertiesPrivate
{
H_DISABLE_COPY(HCdsPropertiesPrivate)

public:

    QVector<const HCdsPropertyInfo*> m_propertyInfos;
    QHash<const QString, const HCdsPropertyInfo*> m_propertyInfosHash;

    inline HCdsPropertiesPrivate(){}
    inline ~HCdsPropertiesPrivate()
    {
        qDeleteAll(m_propertyInfos);
    }

    inline void insert(HCdsPropertyInfo* obj)
    {
        Q_ASSERT(obj);
        m_propertyInfos.append(obj);
        m_propertyInfosHash.insert(obj->name(), obj);
    }
};

namespace
{
bool lessThan(const HCdsPropertyInfo* obj1, const HCdsPropertyInfo* obj2)
{
     return obj1->name() < obj2->name();
}
}

HCdsProperties::HCdsProperties() :
    h_ptr(new HCdsPropertiesPrivate())
{
    h_ptr->m_propertyInfos.reserve(92);

    HCdsPropertyInfo* obj = new HCdsPropertyInfo(HCdsPropertyInfo::empty());
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("@id"), HCdsProperties::dlite_id, QVariant::String,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::Mandatory) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
       QLatin1String("@parentID"), HCdsProperties::dlite_id, QVariant::String,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::Mandatory) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("@restricted"), HCdsProperties::dlite_restricted, QVariant::Bool,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::Mandatory) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("res"), HCdsProperties::dlite_res, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::StandardType) | HCdsPropertyInfo::MultiValued);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("@refID"), HCdsProperties::dlite_refId, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("@childCount"), HCdsProperties::dlite_childCount, QVariant::Int, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("@searchable"), HCdsProperties::dlite_searchable, QVariant::Bool, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("@neverPlayable"), HCdsProperties::dlite_neverPlayable, QVariant::Bool, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("dc:title"), HCdsProperties::dc_title, QVariant::String,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::StandardType) | HCdsPropertyInfo::Mandatory);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("dc:creator"), HCdsProperties::dc_creator, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("dc:description"), HCdsProperties::dc_description, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("dc:publisher"), HCdsProperties::dc_publisher, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::StandardType) | HCdsPropertyInfo::MultiValued);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("dc:date"), HCdsProperties::dc_date, QVariant::DateTime, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("dc:rights"), HCdsProperties::dc_rights, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("dc:relation"), HCdsProperties::dc_relation, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("dc:language"), HCdsProperties::dc_language, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("dc:contributor"), HCdsProperties::dc_contributor, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:class"), HCdsProperties::upnp_class, QVariant::String,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::Mandatory) | HCdsPropertyInfo::StandardType);
     h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:writeStatus"), HCdsProperties::upnp_writeStatus, QVariant::fromValue(HObject::UnknownWriteStatus),
        HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:objectUpdateID"), HCdsProperties::upnp_objectUpdateID, QVariant::UInt,
        HCdsPropertyInfo::StandardType | HCdsPropertyInfo::Disableable);
     h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:bookmarkID"), HCdsProperties::upnp_bookmarkID, QVariant::StringList, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:longDescription"), HCdsProperties::upnp_longDescription, QVariant::String, HCdsPropertyInfo::StandardType);
     h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:rating"), HCdsProperties::upnp_rating, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:album"), HCdsProperties::upnp_album, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:genre"), HCdsProperties::upnp_genre, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:artist"), HCdsProperties::upnp_artist, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:originalTrackNumber"), HCdsProperties::upnp_originalTrackNumber, QVariant::UInt, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:producer"), HCdsProperties::upnp_producer, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:actor"), HCdsProperties::upnp_actor, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:playList"), HCdsProperties::upnp_playList, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:director"), HCdsProperties::upnp_director, QVariant::StringList,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
     h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:playbackCount"), HCdsProperties::upnp_playbackCount, QVariant::UInt, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:lastPlaybackTime"), HCdsProperties::upnp_lastPlaybackTime, QVariant::DateTime, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:lastPlaybackPosition"), HCdsProperties::upnp_lastPlaybackPosition, QVariant::fromValue(HContentDuration()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:recordedStartDateTime"), HCdsProperties::upnp_recordedStartDateTime, QVariant::DateTime, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:recordedDuration"), HCdsProperties::upnp_recordedDuration, QVariant::fromValue(HContentDuration()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:recordedDayOfWeek"), HCdsProperties::upnp_recordedDayOfWeek, QVariant::fromValue(Undefined_DayOfWeek),
        HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:srsRecordScheduleID"), HCdsProperties::upnp_srsRecordScheduleID, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:srsRecordTaskID"), HCdsProperties::upnp_srsRecordTaskID, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:author"), HCdsProperties::upnp_author, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:storageMedium"), HCdsProperties::upnp_storageMedium, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:storageTotal"), HCdsProperties::upnp_storageTotal, QVariant::ULongLong, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:storageUsed"), HCdsProperties::upnp_storageUsed, QVariant::ULongLong, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:storageFree"), HCdsProperties::upnp_storageFree, QVariant::ULongLong, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:storageMaxPartition"), HCdsProperties::upnp_storageMaxPartition, QVariant::ULongLong, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:containerUpdateID"), HCdsProperties::upnp_containerUpdateID, QVariant::String,
        HCdsPropertyInfo::StandardType | HCdsPropertyInfo::Disableable);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:totalDeletedChildCount"), HCdsProperties::upnp_totalDeletedChildCount, QVariant::UInt,
        HCdsPropertyInfo::StandardType | HCdsPropertyInfo::Disableable);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:createClass"), HCdsProperties::upnp_createClass, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:searchClass"), HCdsProperties::upnp_searchClass, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::MultiValued) | HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:region"), HCdsProperties::upnp_region, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:radioCallSign"), HCdsProperties::upnp_radioCallSign, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:radioStationID"), HCdsProperties::upnp_radioStationID, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
       QLatin1String("upnp:radioBand"), HCdsProperties::upnp_radioBand, QVariant::fromValue(HRadioBand()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:channelNr"), HCdsProperties::upnp_channelNr, QVariant::Int, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:signalStrength"), HCdsProperties::upnp_signalStrength, QVariant::Int, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:signalLocked"), HCdsProperties::upnp_signalLocked, QVariant::Bool, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:tuned"), HCdsProperties::upnp_tuned, QVariant::Bool, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:recordable"), HCdsProperties::upnp_recordable, QVariant::Bool, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:DVDRegionCode"), HCdsProperties::upnp_dvdRegionCode, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
       QLatin1String("upnp:channelName"), HCdsProperties::upnp_channelName, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:scheduledStartTime"), HCdsProperties::upnp_scheduledStartTime, QVariant::fromValue(HScheduledTime()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:scheduledEndTime"), HCdsProperties::upnp_scheduledEndTime, QVariant::fromValue(HScheduledTime()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:scheduledDuration"), HCdsProperties::upnp_scheduledDuration, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:programTitle"), HCdsProperties::upnp_programTitle, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:seriesTitle"), HCdsProperties::upnp_seriesTitle, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:episodeCount"), HCdsProperties::upnp_episodeCount, QVariant::Int, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:episodeNumber"), HCdsProperties::upnp_episodeNumber, QVariant::Int, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:icon"), HCdsProperties::upnp_icon, QVariant::Url, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:callSign"), HCdsProperties::upnp_callSign, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:price"), HCdsProperties::upnp_price, QVariant::fromValue(HPrice()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:payPerView"), HCdsProperties::upnp_payPerView, QVariant::Bool, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:bookmarkedObjectID"), HCdsProperties::upnp_bookmarkedObjectID, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:deviceUDN"), HCdsProperties::upnp_deviceUdn, QVariant::fromValue(HDeviceUdn()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:stateVariableCollection"), HCdsProperties::upnp_stateVariableCollection,
        QVariant::fromValue(HStateVariableCollection()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:channelGroupName"), HCdsProperties::upnp_channelGroupName, QVariant::fromValue(HChannelGroupName()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:epgProviderName"), HCdsProperties::upnp_epgProviderName, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:dateTimeRange"), HCdsProperties::upnp_dateTimeRange, QVariant::fromValue(HDateTimeRange()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:serviceProvider"), HCdsProperties::upnp_serviceProvider, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:programID"), HCdsProperties::upnp_programID, QVariant::fromValue(HMatchingId()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:seriesID"), HCdsProperties::upnp_seriesID, QVariant::fromValue(HMatchingId()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:channelID"), HCdsProperties::upnp_channelID, QVariant::fromValue(HChannelId()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:programCode"), HCdsProperties::upnp_programCode, QVariant::fromValue(HProgramCode()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:episodeType"), HCdsProperties::upnp_episodeType, QVariant::fromValue(HEpisodeType()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:networkAffiliation"), HCdsProperties::upnp_networkAffiliation, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:foreignMetadata"), HCdsProperties::upnp_foreignMetadata, QVariant::fromValue(HForeignMetadata()), HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
       QLatin1String("upnp:artistDiscographyURI"), HCdsProperties::upnp_artistDiscographyURI, QVariant::Url, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:lyricsURI"), HCdsProperties::upnp_lyricsURI, QVariant::List, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:albumArtURI"), HCdsProperties::upnp_albumArtURI, QVariant::List,
        HCdsPropertyInfo::PropertyFlags(HCdsPropertyInfo::StandardType) | HCdsPropertyInfo::MultiValued);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:toc"), HCdsProperties::upnp_toc, QVariant::String, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(
        QLatin1String("upnp:userAnnotation"), HCdsProperties::upnp_userAnnotation, QVariant::List, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);

    obj = HCdsPropertyInfo::create(QLatin1String("desc"), HCdsProperties::dlite_desc, QVariant::List, HCdsPropertyInfo::StandardType);
    h_ptr->insert(obj);
}

HCdsProperties::~HCdsProperties()
{
    delete h_ptr;
}

const HCdsProperties& HCdsProperties::instance()
{
    QMutexLocker locker(s_instanceLock);
    if (!s_instance)
    {
        s_instance = new HCdsProperties();
    }
    return *s_instance;
}

const HCdsPropertyInfo& HCdsProperties::get(Property property) const
{
    return *(*(h_ptr->m_propertyInfos.begin()+property));
}

const HCdsPropertyInfo& HCdsProperties::get(const QString& property) const
{
    if (h_ptr->m_propertyInfosHash.contains(property))
    {
        return *h_ptr->m_propertyInfosHash.value(property);
    }
    return HCdsPropertyInfo::empty();
}

}
}
}
