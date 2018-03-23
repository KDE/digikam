/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QByteArray>
#include <QUrl>

// Local includes

#include "metaengine.h"
#include "metaengine_data.h"
#include "captionvalues.h"
#include "metadatasettingscontainer.h"
#include "infocontainer.h"
#include "metadatainfo.h"
#include "digikam_export.h"
#include "dmetadatasettings.h"

namespace Digikam
{

class Template;
class IccProfile;

class DIGIKAM_EXPORT DMetadata : public MetaEngine
{

public:

    /** Video color model reported by FFMPEG following XMP DM Spec from Adobe.
     *  These value are stored in DB as Image color model properties (extension of DImg::ColorModel)
     */
    enum VIDEOCOLORMODEL
    {
        VIDEOCOLORMODEL_UNKNOWN=1000,
        VIDEOCOLORMODEL_OTHER,
        VIDEOCOLORMODEL_SRGB,
        VIDEOCOLORMODEL_BT709,
        VIDEOCOLORMODEL_BT601
    };

public:

    DMetadata();
    explicit DMetadata(const QString& filePath);
    explicit DMetadata(const MetaEngineData& data);
    ~DMetadata();

    void registerMetadataSettings();
    void setSettings(const MetadataSettingsContainer& settings);

    /** Re-implemented from libMetaEngine to use libraw identify and 
     *  ffmpeg probe methods if Exiv2 failed.
     */
    bool load(const QString& filePath);
    bool save(const QString& filePath, bool setVersion = true) const;
    bool applyChanges() const;

    /** Try to extract metadata using Raw Engine identify method (libraw)
     */
    bool loadUsingRawEngine(const QString& filePath);

    /** Try to extract metadata using FFMpeg probe method (libav)
     */
    bool loadUsingFFmpeg(const QString& filePath);

    /** Metadata manipulation methods */

    CaptionsMap getImageComments(const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings()) const;
    bool setImageComments(const CaptionsMap& comments,
                          const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings()) const;

    int  getImagePickLabel() const;
    bool setImagePickLabel(int pickId) const;

    int  getImageColorLabel() const;
    bool setImageColorLabel(int colorId) const;

    CaptionsMap getImageTitles() const;
    bool setImageTitles(const CaptionsMap& title) const;

    int  getImageRating(const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings()) const;
    bool setImageRating(int rating,
                        const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings()) const;

    bool getImageTagsPath(QStringList& tagsPath,
                          const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings()) const;
    bool setImageTagsPath(const QStringList& tagsPath,
                          const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings()) const;

    bool getACDSeeTagsPath(QStringList& tagsPath) const;

    bool setACDSeeTagsPath(const QStringList& tagsPath) const;

    /** Get Images Face Map based on tags stored in Picassa/Metadatagroup
     * format. Use $ exiv2 -pa image to see the tag structure
     */
    bool getImageFacesMap(QMultiMap<QString,QVariant>& facesPath) const;

    /** Set Images Face Map tags in Picassa/Metadatagroup format
     *  Use exiv2 -pa image to check for face tags,
     *  @param write    - if true all faces will be written, else update mode:
     *                    search if at least a face tag exist and write if true
     */
    bool setImageFacesMap(QMultiMap<QString,QVariant>& facesPath, bool write) const;

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
        This method does not retrieve profiles embedded in the image but not the metadata,
        e.g. embedded profiles in JPEG images.
        Returns a null profile if no profile is found.
     */
    IccProfile getIccProfile() const;

    /** Sets the IccProfile embedded in the Exif metadata. */
    bool setIccProfile(const IccProfile& profile);

    /** Remove the Exif color space identification from the image */
    bool removeExifColorSpace() const;

    PhotoInfoContainer getPhotographInformation() const;

    /** Returns video metadata from Xmp tags.
     */
    VideoInfoContainer getVideoInformation() const;

    /** Returns millisecond time-stamp from Exif tags or 0 if not found.
     */
    int  getMSecsInfo() const;

    /** Extract milliseconds time-stamp of photo from an Exif tag and store it to 'ms'.
     *  Returns true if data are extracted.
     */
    bool mSecTimeStamp(const char* const exifTagName, int& ms) const;

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

    static MetaEngine::AltLangMap toAltLangMap(const QVariant& var);

    // These methods have been factored to libMetaEngine 2.3.0. Remove it after KDE 4.8.2

    /** Set an Xmp tag content using a list of strings defined by the 'entriesToAdd' parameter.
        The existing entries are preserved. The method will compare
        all new with all already existing entries to prevent duplicates in the image.
        Return true if the entries have been added to metadata.
     */
    bool addToXmpTagStringBag(const char* const xmpTagName, const QStringList& entriesToAdd) const;

    /** Remove those Xmp tag entries that are listed in entriesToRemove from the entries in metadata.
        Return true if tag entries are no longer contained in metadata.
        All other entries are preserved.
     */
    bool removeFromXmpTagStringBag(const char* const xmpTagName, const QStringList& entriesToRemove) const;

    /** Return a strings list of Xmp keywords from image. Return an empty list if no keyword are set.
     */
    QStringList getXmpKeywords() const;

    /** Set Xmp keywords using a list of strings defined by 'newKeywords' parameter.
        The existing keywords from image are preserved. The method will compare
        all new keywords with all already existing keywords to prevent duplicate entries in image.
        Return true if keywords have been changed in metadata.
     */
    bool setXmpKeywords(const QStringList& newKeywords) const;

    /** Remove those Xmp keywords that are listed in keywordsToRemove from the keywords in metadata.
        Return true if keywords are no longer contained in metadata.
     */
    bool removeXmpKeywords(const QStringList& keywordsToRemove);

    /** Return a strings list of Xmp subjects from image. Return an empty list if no subject are set.
     */
    QStringList getXmpSubjects() const;

    /** Set Xmp subjects using a list of strings defined by 'newSubjects' parameter.
        The existing subjects from image are preserved. The method will compare
        all new subject with all already existing subject to prevent duplicate entries in image.
        Return true if subjects have been changed in metadata.
     */
    bool setXmpSubjects(const QStringList& newSubjects) const;

    /** Remove those Xmp subjects that are listed in subjectsToRemove from the subjects in metadata.
        Return true if subjects are no longer contained in metadata.
     */
    bool removeXmpSubjects(const QStringList& subjectsToRemove);

    /** Return a strings list of Xmp sub-categories from image. Return an empty list if no sub-category
        are set.
     */
    QStringList getXmpSubCategories() const;

    /** Set Xmp sub-categories using a list of strings defined by 'newSubCategories' parameter.
        The existing sub-categories from image are preserved. The method will compare
        all new sub-categories with all already existing sub-categories to prevent duplicate entries in image.
        Return true if sub-categories have been changed in metadata.
     */
    bool setXmpSubCategories(const QStringList& newSubCategories) const;

    /** Remove those Xmp sub-categories that are listed in categoriesToRemove from the sub-categories in metadata.
        Return true if subjects are no longer contained in metadata.
     */
    bool removeXmpSubCategories(const QStringList& categoriesToRemove);

    bool removeExifTags(const QStringList& tagFilters);
    bool removeIptcTags(const QStringList& tagFilters);
    bool removeXmpTags(const QStringList& tagFilters);

    /**
     * Helper method to translate enum values to user presentable strings
     */
    static QString videoColorModelToString(VIDEOCOLORMODEL videoColorModel);

private:

    bool setIptcTag(const QString& text, int maxLength, const char* const debugLabel, const char* const tagKey) const;

    QVariant fromExifOrXmp(const char* const exifTagName, const char* const xmpTagName) const;
    QVariant fromIptcOrXmp(const char* const iptcTagName, const char* const xmpTagName) const;
    QVariant fromXmpList(const char* const xmpTagName)                                  const;
    QVariant fromIptcEmulateList(const char* const iptcTagName)                         const;
    QVariant fromXmpLangAlt(const char* const xmpTagName)                               const;
    QVariant fromIptcEmulateLangAlt(const char* const iptcTagName)                      const;
    QVariant toStringListVariant(const QStringList& list)                               const;
    QString getExifTagStringFromTagsList(const QStringList& tagsList)                   const;
};

} // namespace Digikam

#endif // DMETADATA_H
