/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QtCore/QByteArray>

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>
#include <libkexiv2/kexiv2data.h>
#include <libkexiv2/version.h>

// Local includes

#include "dimg.h"
#include "captionvalues.h"
#include "metadatasettingscontainer.h"
#include "photoinfocontainer.h"
#include "metadatainfo.h"
#include "digikam_export.h"

namespace Digikam
{

class Template;
class IccProfile;

class DIGIKAM_EXPORT DMetadata : public KExiv2Iface::KExiv2
{

public:

    DMetadata();
    DMetadata(const QString& filePath);
    DMetadata(const KExiv2Data& data);
    ~DMetadata();

    void registerMetadataSettings();
    void setSettings(const MetadataSettingsContainer& settings);

    /** Re-implemented from libKexiv2 to use dcraw identify method if Exiv2 failed. */
    bool load(const QString& filePath) const;

    /** Try to extract metadata using dcraw identify method */
    bool loadUsingDcraw(const QString& filePath) const;

    /** Metadata manipulation methods */

    CaptionsMap getImageComments() const;
    bool setImageComments(const CaptionsMap& comments) const;

    int  getImagePickLabel() const;
    bool setImagePickLabel(int pickId) const;

    int  getImageColorLabel() const;
    bool setImageColorLabel(int colorId) const;

    CaptionsMap getImageTitles() const;
    bool setImageTitles(const CaptionsMap& title) const;

    int  getImageRating() const;
    bool setImageRating(int rating) const;

    bool getImageTagsPath(QStringList& tagsPath) const;
    bool setImageTagsPath(const QStringList& tagsPath) const;

    bool getImageFacesMap(QMap<QString,QVariant>& facesPath) const;

    bool     setMetadataTemplate(const Template& t) const;
    Template getMetadataTemplate() const;
    bool     removeMetadataTemplate() const;

    QString getImageHistory() const;
    bool    setImageHistory(QString& imageHistoryXml) const;
    bool    hasImageHistoryTag() const;

    QString getImageUniqueId() const;
    bool    setImageUniqueId(const QString& uuid) const;

    /// Fills only the copyright values in the template. Use getMetadataTemplate() usually.
    /// Returns true if valid fields were read.
    bool getCopyrightInformation(Template& t) const;

    IptcCoreContactInfo getCreatorContactInfo() const;
    bool setCreatorContactInfo(const IptcCoreContactInfo& info) const;

    IptcCoreLocationInfo getIptcCoreLocation() const;
    bool setIptcCoreLocation(const IptcCoreLocationInfo& location) const;

    QStringList getIptcCoreSubjects() const;

    /** Return a string with Lens mounted on the front of camera.
        There no standard Exif tag for Lens information.
        Camera makernotes and Xmp tags are parsed.
        Take a care : lens information are not standardized and string content is not homogeneous between
        camera model/maker.
     */
    QString getLensDescription() const;

    /** Reads an IccProfile that is described or embedded in the metadata.
        This method does retrieve profiles embedded in the image but not the metadata,
        e.g. embedded profiles in JPEG images.
        Returns a null profile if no profile is found.
     */
    IccProfile getIccProfile() const;

    /** Remove the Exif color space identification from the image */
    bool removeExifColorSpace() const;

    PhotoInfoContainer getPhotographInformation() const;

    /** Returns millisecond time-stamp from Exif tags or 0 if not found.
     */
    int  getMSecsInfo() const;

    /** Extract milliseconds time-stamp of photo from an Exif tag and store it to 'ms'.
     *  Returns true if data are extracted.
     */
    bool mSecTimeStamp(const char* exifTagName, int& ms) const;

    /** Returns the requested metadata field as a QVariant. See metadatainfo.h for a specification
        of the format of the QVariant.
     */
    QVariant     getMetadataField(MetadataInfo::Field field) const;
    QVariantList getMetadataFields(const MetadataFields& fields) const;

    /** Convert a QVariant value of the specified field to a user-presentable, i18n'ed string.
        The QVariant must be of the type as specified in metadatainfo.h and as obtained by getMetadataField.
     */
    static QString     valueToString (const QVariant& value, MetadataInfo::Field field);
    static QStringList valuesToString(const QVariantList& list, const MetadataFields& fields);

    /** Returns a map of possible enum values and their user-presentable, i18n'ed representation.
        Valid fields are those which are described as "enum from" or "bit mask from" in metadatainfo.h.
     */
    static QMap<int, QString> possibleValuesForEnumField(MetadataInfo::Field field);

    static double apexApertureToFNumber(double aperture);
    static double apexShutterSpeedToExposureTime(double shutterSpeed);

    static KExiv2::AltLangMap toAltLangMap(const QVariant& var);

    //------------------------------------------------------------------------------------------------
    // Pushed to libkexiv2 for KDE4.4

    /** Set an Xmp tag content using a list of strings defined by the 'entriesToAdd' parameter.
        The existing entries are preserved. The method will compare
        all new with all already existing entries to prevent duplicates in the image.
        Return true if the entries have been added to metadata.
     */
    bool addToXmpTagStringBag(const char* xmpTagName, const QStringList& entriesToAdd,
                              bool setProgramName) const;

    /** Remove those Xmp tag entries that are listed in entriesToRemove from the entries in metadata.
        Return true if tag entries are no longer contained in metadata.
        All other entries are preserved.
     */
    bool removeFromXmpTagStringBag(const char* xmpTagName, const QStringList& entriesToRemove,
                                   bool setProgramName) const;

    /** Return a strings list of Xmp keywords from image. Return an empty list if no keyword are set.
     */
    QStringList getXmpKeywords() const;

    /** Set Xmp keywords using a list of strings defined by 'newKeywords' parameter.
        The existing keywords from image are preserved. The method will compare
        all new keywords with all already existing keywords to prevent duplicate entries in image.
        Return true if keywords have been changed in metadata.
     */
    bool setXmpKeywords(const QStringList& newKeywords, bool setProgramName=true) const;

    /** Remove those Xmp keywords that are listed in keywordsToRemove from the keywords in metadata.
        Return true if keywords are no longer contained in metadata.
     */
    bool removeXmpKeywords(const QStringList& keywordsToRemove, bool setProgramName=true);

    /** Return a strings list of Xmp subjects from image. Return an empty list if no subject are set.
     */
    QStringList getXmpSubjects() const;

    /** Set Xmp subjects using a list of strings defined by 'newSubjects' parameter.
        The existing subjects from image are preserved. The method will compare
        all new subject with all already existing subject to prevent duplicate entries in image.
        Return true if subjects have been changed in metadata.
     */
    bool setXmpSubjects(const QStringList& newSubjects, bool setProgramName=true) const;

    /** Remove those Xmp subjects that are listed in subjectsToRemove from the subjects in metadata.
        Return true if subjects are no longer contained in metadata.
     */
    bool removeXmpSubjects(const QStringList& subjectsToRemove, bool setProgramName=true);

    /** Return a strings list of Xmp sub-categories from image. Return an empty list if no sub-category
        are set.
     */
    QStringList getXmpSubCategories() const;

    /** Set Xmp sub-categories using a list of strings defined by 'newSubCategories' parameter.
        The existing sub-categories from image are preserved. The method will compare
        all new sub-categories with all already existing sub-categories to prevent duplicate entries in image.
        Return true if sub-categories have been changed in metadata.
     */
    bool setXmpSubCategories(const QStringList& newSubCategories, bool setProgramName=true) const;

    /** Remove those Xmp sub-categories that are listed in categoriesToRemove from the sub-categories in metadata.
        Return true if subjects are no longer contained in metadata.
     */
    bool removeXmpSubCategories(const QStringList& categoriesToRemove, bool setProgramName=true);

    // End: Pushed to libkexiv2 for KDE4.4
    //------------------------------------------------------------------------------------------------

    //------------------------------------------------------------------------------------------------
    // Compatibility for < KDE 4.4.
#if KEXIV2_VERSION < 0x010000
    KExiv2Data data() const;
    void setData(const KExiv2Data& data);
    QByteArray getExifEncoded(bool addExifHeader=false) const
    {
        return getExif(addExifHeader);
    }
#endif
    // End: Compatibility for < KDE 4.4
    //------------------------------------------------------------------------------------------------

private:

    bool setProgramId(bool on=true) const;
    bool setIptcTag(const QString& text, int maxLength, const char* debugLabel, const char* tagKey) const;

    QVariant fromExifOrXmp(const char* exifTagName, const char* xmpTagName) const;
    QVariant fromIptcOrXmp(const char* iptcTagName, const char* xmpTagName) const;
    QVariant fromXmpList(const char* xmpTagName) const;
    QVariant fromIptcEmulateList(const char* iptcTagName) const;
    QVariant fromXmpLangAlt(const char* xmpTagName) const;
    QVariant fromIptcEmulateLangAlt(const char* iptcTagName) const;
    QVariant toStringListVariant(const QStringList& list) const;

    QString getExifTagStringFromTagsList(const QStringList& tagsList) const;
};

}  // namespace Digikam

#endif /* DMETADATA_H */
