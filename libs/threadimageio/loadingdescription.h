/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-16
 * Description : image file IO threaded interface.
 *
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef LOADING_DESCRIPTION_H
#define LOADING_DESCRIPTION_H

// Local includes

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class IccTransform;

class DIGIKAM_EXPORT LoadingDescription
{
public:

    enum ColorManagementSettings
    {
        NoColorConversion,
        ApplyTransform, /// IccData is an IccTransform
        ConvertForEditor,
        ConvertToSRGB,
        ConvertForDisplay, /// IccData can be the output profile
        ConvertForOutput /// IccData is the output profile
    };

    class PreviewParameters
    {
    public:

        enum PreviewType
        {
            NoPreview,
            PreviewImage,
            Thumbnail
        };

        PreviewParameters()
        {
            type       = NoPreview;
            size       = 0;
            exifRotate = false;
        }

        PreviewType type;
        int  size;
        bool exifRotate;

        bool operator==(const PreviewParameters& other) const;
    };

    class PostProcessingParameters
    {
    public:

        PostProcessingParameters()
        {
            colorManagement = NoColorConversion;
        }

        bool needsProcessing() const;

        void setTransform(const IccTransform& transform);
        bool hasTransform() const;
        IccTransform transform() const;
        void setProfile(const IccProfile& profile);
        bool hasProfile() const;
        IccProfile profile() const;

        ColorManagementSettings colorManagement;
        QVariant                iccData;

        bool operator==(const PostProcessingParameters& other) const;
    };

    /**
     * An invalid LoadingDescription
     */
    LoadingDescription()
    {
    }

    /**
     * Use this for files that are not raw files.
     * Stores only the filePath.
     */
    explicit LoadingDescription(const QString& filePath,
                                ColorManagementSettings = NoColorConversion);

    /**
     * For raw files:
     * Stores filePath and RawDecodingSettings
     */
    LoadingDescription(const QString& filePath, const DRawDecoding& settings,
                       ColorManagementSettings = NoColorConversion);

    /**
     * For preview and thumbnail jobs:
     * Stores preview max size and Exif rotation.
     * Exif Rotation:
     *    The Exif rotation is only a hint.
     *    Call LoadSaveThread::exifRotate to make sure that the image is really
     *    rotated. It is safe to call this method even if the image is rotated.
     * Raw files / preview jobs:
     *    If size is not 0, the embedded preview will be loaded if available.
     *    If size is 0, DImg based loading will be used with default raw decoding settings.
     */
    LoadingDescription(const QString& filePath, int size, bool exifRotate,
                       ColorManagementSettings = NoColorConversion,
                       PreviewParameters::PreviewType = PreviewParameters::PreviewImage);

    QString           filePath;
    DRawDecoding      rawDecodingSettings;
    PreviewParameters previewParameters;
    PostProcessingParameters postProcessingParameters;

    /**
     * Return the cache key this description shall be stored as
     */
    QString             cacheKey() const;
    /**
     * Return all possible cache keys, starting with the best choice,
     * for which a result may be found in the cache for this description.
     * Included in the list are better quality versions, if this description is reduced.
     */
    QStringList         lookupCacheKeys() const;
    /**
     * Returns whether this description describes a loading operation which
     * loads the image in a reduced version (quality, size etc.)
     */
    bool                isReducedVersion() const;

    /**
     * Returns if this description will load a thumbnail
     */
    bool                isThumbnail() const;


    /**
     * Returns if this description will load a preview
     */
    bool                isPreviewImage() const;

   /**
     * Returns whether the other loading task equals this one
     */
    bool operator==(const LoadingDescription& other) const;
    bool operator!=(const LoadingDescription& other) const
        { return !operator==(other); }
    /**
     * Returns whether the other loading task equals this one
     * ignoring parameters used to specify a reduced version.
     */
    bool equalsIgnoreReducedVersion(const LoadingDescription& other) const;

    /**
     * Returns whether this loading task equals the other one
     * or is superior to it, if the other one is a reduced version
     */
    bool equalsOrBetterThan(const LoadingDescription& other) const;

    /**
     * Returns all possible cacheKeys for the given file path
     * (all cache keys under which the given file could be stored in the cache).
     */
    static QStringList possibleCacheKeys(const QString& filePath);
    static QStringList possibleThumbnailCacheKeys(const QString& filePath);
};

}   // namespace Digikam

#endif // LOADING_DESCRIPTION_H
