/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DMETADATA_H
#define DMETADATA_H

// Qt includes.

#include <QtCore/QByteArray>

// LibKExiv2 includes.

#include <libkexiv2/kexiv2.h>

// Local includes.

#include "dimg.h"
#include "photoinfocontainer.h"
#include "metadatainfo.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DMetadata : public KExiv2Iface::KExiv2
{

public:

    DMetadata();
    DMetadata(const QString& filePath);
    ~DMetadata();

    /** Re-implemented from libKexiv2 to use dcraw identify method if Exiv2 failed. */
    bool load(const QString& filePath) const;

    /** Try to extract metadata using dcraw identify method */
    bool loadUsingDcraw(const QString& filePath) const;

    /** Metadata manipulation methods */

    QString getImageComment() const;
    bool    setImageComment(const QString& comment) const;

    int  getImageRating() const;
    bool setImageRating(int rating) const;

    bool getImageTagsPath(QStringList& tagsPath) const;
    bool setImageTagsPath(const QStringList& tagsPath) const;

    bool setImagePhotographerId(const QString& author, const QString& authorTitle) const;
    bool setImageCredits(const QString& credit, const QString& source, const QString& copyright) const;

    /** Return a string with Lens mounted on the front of camera.
        There no standard Exif tag for Lens information.
        Camera makernotes and Xmp tags are parsed. 
        Take a care : lens information are not standardized and string content is not homogeneous between 
        camera model/maker.
     */
    QString getLensDescription() const;

    PhotoInfoContainer getPhotographInformations() const;

    /** Returns the requested metadata field as a QVariant. See metadatainfo.h for a specification
        of the format of the QVariant.
     */
    QVariant     getMetadataField(MetadataInfo::Field field);
    QVariantList getMetadataFields(const MetadataFields &fields);

    /** Convert a QVariant value of the specified field to a user-presentable, i18n'ed string.
        The QVariant must be of the type as specified in metadatainfo.h and as obtained by getMetadataField.
     */
    static QString     valueToString (const QVariant &value, MetadataInfo::Field field);
    static QStringList valuesToString(const QVariantList &list, const MetadataFields &fields);

    /** Returns a map of possible enum values and their user-presentable, i18n'ed representation.
        Valid fields are those which are described as "enum from" or "bit mask from" in metadatainfo.h.
     */
    static QMap<int, QString> possibleValuesForEnumField(MetadataInfo::Field field);

    /** Methods dedicated to record/read a private IPTC tag used to store digiKam image properties.
        Code tested but not used because XMP is more simple to use for that. */
    bool getXMLImageProperties(QString& comments, QDateTime& date,
                               int& rating, QStringList& tagsPath) const;
    bool setXMLImageProperties(const QString& comments, const QDateTime& date, 
                               int rating, const QStringList& tagsPath) const;

    static double apexApertureToFNumber(double aperture);
    static double apexShutterSpeedToExposureTime(double shutterSpeed);

private:

    bool setProgramId(bool on=true) const;
    bool setIptcTag(const QString& text, int maxLength, const char* debugLabel, const char* tagKey) const;
    QVariant fromExifOrXmp(const char *exifTagName, const char *xmpTagName) const;
    QVariant fromIptcOrXmp(const char *iptcTagName, const char *xmpTagName) const;
    QVariant fromXmpList(const char *xmpTagName) const;
    QVariant fromIptcEmulateList(const char *iptcTagName) const;
    QVariant fromXmpLangAlt(const char *xmpTagName) const;
    QVariant fromIptcEmulateLangAlt(const char *iptcTagName) const;
};

}  // namespace Digikam

#endif /* DMETADATA_H */
