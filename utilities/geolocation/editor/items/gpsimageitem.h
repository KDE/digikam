/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-03-21
 * @brief  An item to hold information about an image.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef GPSIMAGEITEM_H
#define GPSIMAGEITEM_H

// Qt includes

#include <QPersistentModelIndex>
#include <QVariant>
#include <QDateTime>
#include <QUrl>

// Libkgeomap includes

#include "geoiface_types.h"

// Local includes

#include "digikam_debug.h"
#include "gpsdatacontainer.h"
#include "dmetadata.h"

using namespace GeoIface;

namespace Digikam
{

/**
 * @class RGInfo
 *
 * @brief This class contains data needed in reverse geocoding process.
 */

class RGInfo
{
    public:

    /**
     * Constructor
     */
    RGInfo()
      : id(),
        coordinates(),
        rgData()
    {
    }

    /**
     * The image index.
     */
    QPersistentModelIndex  id;

    /**
     * The coordinates of current image.
     */
    GeoCoordinates         coordinates;

    /**
     * The address elements and their names.
     */
    QMap<QString, QString> rgData;
};

enum Type
{
    TypeChild    = 1,
    TypeSpacer   = 2,
    TypeNewChild = 4
};

typedef struct TagData
{
    QString tagName;
    Type    tagType;

} TagData;

class GPSImageModel;

class GPSImageItem
{
public:

    static const int RoleCoordinates         = Qt::UserRole + 1;

    static const int ColumnThumbnail         = 0;
    static const int ColumnFilename          = 1;
    static const int ColumnDateTime          = 2;
    static const int ColumnLatitude          = 3;
    static const int ColumnLongitude         = 4;
    static const int ColumnAltitude          = 5;
    static const int ColumnAccuracy          = 6;
    static const int ColumnTags              = 7;
    static const int ColumnStatus            = 8;
    static const int ColumnDOP               = 9;
    static const int ColumnFixType           = 10;
    static const int ColumnNSatellites       = 11;
    static const int ColumnSpeed             = 12;

    static const int ColumnGPSImageItemCount = 13;

    GPSImageItem(const QUrl& url);
    virtual ~GPSImageItem();

    /// @name Loading and saving
    //@{
    QString saveChanges();
    bool loadImageData();

    inline bool isDirty() const       { return m_dirty; }
    //@}

    inline QUrl url() const           { return m_url; };

    inline QDateTime dateTime() const { return m_dateTime; };

    /// @name Functions used by the model
    //@{
    static void setHeaderData(GPSImageModel* const model);
    bool lessThan(const GPSImageItem* const otherItem, const int column) const;
    //@}

    /// @name GPS related functions
    //@{
    void setCoordinates(const GeoCoordinates& newCoordinates);
    inline GeoCoordinates coordinates() const                 { return m_gpsData.getCoordinates();                        }
    inline GPSDataContainer gpsData() const                   { return m_gpsData;                                         }
    inline void setGPSData(const GPSDataContainer& container) { m_gpsData = container; m_dirty = true; emitDataChanged(); }
    void restoreGPSData(const GPSDataContainer& container);
    //@}

    /// @name Tag related functions
    //@{
    /**
     * The tags added in reverse geocoding process are stored in each image, before they end up in external tag model. This function adds them.
     * @param externalTagList A list containing tags.
     */
    inline void setTagList(const QList<QList<TagData> >& externalTagList) { m_tagList = externalTagList; m_tagListDirty = true; emitDataChanged(); };

    /**
     * @return Returns true is the current image has been modified and not saved.
     */
    inline bool isTagListDirty() const { return m_tagListDirty; }

    /**
     * Returns the tag list of the current image.
     */
    inline QList<QList<TagData> > getTagList() const { return m_tagList; };

    /**
     * Replaces the current tag list with the one contained in tagList.
     */
    void restoreRGTagList(const QList<QList<TagData> >& tagList);

    /**
     * Writes the current tags to XMP metadata.
     */
    void writeTagsToXmp(const bool writeXmpTags) { m_writeXmpTags = writeXmpTags; }
    //@}

protected:

    // these are only to be called by the GPSImageModel
    QVariant data(const int column, const int role) const;
    void setModel(GPSImageModel* const model);
    void emitDataChanged();
    DMetadata* getMetadataForFile() const;

protected:

    GPSImageModel*         m_model;

    QUrl                   m_url;
    QDateTime              m_dateTime;

    bool                   m_dirty;
    GPSDataContainer       m_gpsData;
    GPSDataContainer       m_savedState;

    bool                   m_tagListDirty;
    QList<QList<TagData> > m_tagList;
    QList<QList<TagData> > m_savedTagList;
    bool                   m_writeXmpTags;

    friend class GPSImageModel;
};

} /* namespace Digikam */

#endif /* GPSIMAGEITEM_H */
