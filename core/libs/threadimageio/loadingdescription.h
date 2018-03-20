/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-16
 * Description : image file IO threaded interface.
 *
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

#ifndef LOADING_DESCRIPTION_H
#define LOADING_DESCRIPTION_H

// Qt includes

#include <QFlags>

// Local includes

#include "dimg.h"
#include "digikam_export.h"
#include "previewsettings.h"

namespace Digikam
{

class IccTransform;
class ThumbnailIdentifier;

class DIGIKAM_EXPORT LoadingDescription
{
public:

    enum ColorManagementSettings
    {
        NoColorConversion,
        ApplyTransform,    /// IccData is an IccTransform
        ConvertForEditor,
        ConvertToSRGB,
        ConvertForDisplay, /// IccData can be the output profile
        ConvertForOutput   /// IccData is the output profile
    };

    enum RawDecodingHint
    {
        /// The raw decoding options passed are taken from default, hardcoded settings
        RawDecodingDefaultSettings,
        /// The raw decoding options passed are taken from global settings
        RawDecodingGlobalSettings,
        /// The raw decoding options may be customly edited by the user
        RawDecodingCustomSettings,
        /// The raw decoding options are hardcoded settings optimized for loading time
        /// The halfSizeColorImage and 16bit settings can be adjusted separately
        RawDecodingTimeOptimized
    };

public:

    class PreviewParameters
    {
    public:

        enum PreviewType
        {
            NoPreview,
            PreviewImage,
            Thumbnail,
            DetailThumbnail
        };

        enum PreviewFlag
        {
            NoFlags         = 0,
            OnlyPregenerate = 1 << 0
        };
        Q_DECLARE_FLAGS(PreviewFlags, PreviewFlag)

    public:

        PreviewParameters();

        bool onlyPregenerate() const
        {
            return flags & OnlyPregenerate;
        }

        bool operator==(const PreviewParameters& other) const;

    public:

        PreviewType  type;
        int          size;
        PreviewFlags flags;
        PreviewSettings previewSettings;
        QVariant     extraParameter;
        QVariant     storageReference;
    };

    // ---------------------------------------------------------------------

    class PostProcessingParameters
    {
    public:

        PostProcessingParameters()
        {
            colorManagement = NoColorConversion;
        }

        bool needsProcessing() const;

        void         setTransform(const IccTransform& transform);
        bool         hasTransform() const;
        IccTransform transform()    const;

        void       setProfile(const IccProfile& profile);
        bool       hasProfile() const;
        IccProfile profile()    const;

        bool operator==(const PostProcessingParameters& other) const;

    public:

        ColorManagementSettings colorManagement;
        QVariant                iccData;
    };

public:

    /**
     * An invalid LoadingDescription
     */
    LoadingDescription()
    {
        rawDecodingHint = RawDecodingDefaultSettings;
    }

    /**
     * Use this for full loading of non-raw files
     */
    LoadingDescription(const QString& filePath, ColorManagementSettings = NoColorConversion);

    /**
     * Use this for full loading of raw files
     */
    LoadingDescription(const QString& filePath, const DRawDecoding& settings,
                       RawDecodingHint rawDecodingHint = RawDecodingCustomSettings,
                       ColorManagementSettings = NoColorConversion);

    /**
     * For preview and thumbnail jobs:
     * Stores preview max size and Exif rotation.
     * Raw files / preview jobs:
     *    If size is not 0, the embedded preview will be loaded if available.
     *    If size is 0, DImg based loading will be used with default raw decoding settings.
     *    You can also adjust raw decoding settings and hint in this case.
     */
    LoadingDescription(const QString& filePath,
                       const PreviewSettings& settings, int size,
                       ColorManagementSettings = NoColorConversion,
                       PreviewParameters::PreviewType = PreviewParameters::PreviewImage);

    /**
     * Return the cache key for this description
     */
    QString             cacheKey() const;
    /**
     * For some RAW images, the same cache key is not enough to say it is the correct result.
     * You must check the raw decoding settings in this case.
     */
    bool                needCheckRawDecoding() const;
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
     * If this referenced a thumbnail, recreate the identifier
     */
    ThumbnailIdentifier thumbnailIdentifier() const;

    /**
      * Returns whether the other loading task equals this one
      */
    bool operator==(const LoadingDescription& other) const;
    bool operator!=(const LoadingDescription& other) const
    {
        return !operator==(other);
    }

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

public:

    /**
     * Returns all possible cacheKeys for the given file path
     * (all cache keys under which the given file could be stored in the cache).
     */
    static QStringList possibleCacheKeys(const QString& filePath);
    static QStringList possibleThumbnailCacheKeys(const QString& filePath);

public:

    QString                  filePath;
    DRawDecoding             rawDecodingSettings;
    RawDecodingHint          rawDecodingHint;
    PreviewParameters        previewParameters;
    PostProcessingParameters postProcessingParameters;
};

}   // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::LoadingDescription::PreviewParameters::PreviewFlags)

#endif // LOADING_DESCRIPTION_H
