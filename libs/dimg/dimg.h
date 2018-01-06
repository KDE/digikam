/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIMG_H
#define DIMG_H

// Qt includes

#include <QByteArray>
#include <QFlags>
#include <QSize>
#include <QRect>
#include <QVariant>

// Local includes

#include "digikam_export.h"
#include "dshareddata.h"
#include "drawdecoding.h"
#include "dcolor.h"
#include "dcolorcomposer.h"
#include "historyimageid.h"
#include "iccprofile.h"
#include "metaengine_data.h"

class QImage;
class QPixmap;

namespace Digikam
{

class ExposureSettingsContainer;
class DImageHistory;
class FilterAction;
class IccTransform;
class DImgLoaderObserver;

class DIGIKAM_EXPORT DImg
{
public:

    enum FORMAT
    {
        // NOTE: Order is important here:
        // See filesaveoptionbox.cpp which use these values to fill a stack of widgets.
        NONE = 0,
        JPEG,
        PNG,
        TIFF,
        JP2K,
        PGF,
        // Others file formats.
        RAW,
        PPM,
        QIMAGE
    };

    enum ANGLE
    {
        ROT90=0,
        ROT180,
        ROT270,
        ROTNONE
    };

    enum FLIP
    {
        HORIZONTAL=0,
        VERTICAL
    };

    enum COLORMODEL
    {
        COLORMODELUNKNOWN=0,
        RGB,
        GRAYSCALE,
        MONOCHROME,
        INDEXED,
        YCBCR,
        CMYK,
        CIELAB,
        COLORMODELRAW
    };

public:

    /** Identify file format
     */
    static FORMAT fileFormat(const QString& filePath);

    static QString formatToMimeType(FORMAT frm);

public:

    class Private;

public:

    /** Create null image
     */
    DImg();

    /** Load image using QByteArray as file path
     */
    explicit DImg(const QByteArray& filePath, DImgLoaderObserver* const observer = 0,
                  const DRawDecoding& rawDecodingSettings=DRawDecoding());

    /** Load image using QString as file path
     */
    explicit DImg(const QString& filePath, DImgLoaderObserver* const observer = 0,
                  const DRawDecoding& rawDecodingSettings=DRawDecoding());

    /** Copy image: Creates a shallow copy that refers to the same shared data.
        The two images will be equal. Call detach() or copy() to create deep copies.
     */
    DImg(const DImg& image);

    /** Copy image: Creates a copy of a QImage object. If the QImage is null, a
        null DImg will be created.
     */
    explicit DImg(const QImage& image);

    /** Create image from data.
        If data is 0, a new buffer will be allocated, otherwise the given data will be used:
        If copydata is true, the data will be copied to a newly allocated buffer.
        If copyData is false, this DImg object will take ownership of the data pointer.
        If there is an alpha channel, the data shall be in non-premultiplied form (unassociated alpha).
     */
    DImg(uint width, uint height, bool sixteenBit, bool alpha=false, uchar* const data=0, bool copyData=true);

    ~DImg();

    /** Equivalent to the copy constructor
     */
    DImg&      operator=(const DImg& image);

    /** Detaches from shared data and makes sure that this image
        is the only one referring to the data.
        If multiple images share common data, this image makes a copy
        of the data and detaches itself from the sharing mechanism.
        Nothing is done if there is just a single reference.
    */
    void       detach();

    /** Returns whether two images are equal.
        Two images are equal if and only if they refer to the same shared data.
        (Thus, DImg() == DImg() is not true, both instances refer two their
        own shared data. image == DImg(image) is true.)
        If two or more images refer to the same data, they have the same
        image data, bits() returns the same data, they have the same metadata,
        and a change to one image also affects the others.
        Call detach() to split one image from the group of equal images.
    */
    bool        operator==(const DImg& image) const;

    /** Replaces image data of this object. Metadata is unchanged. Parameters like constructor above.
     */
    void        putImageData(uint width, uint height, bool sixteenBit, bool alpha, uchar* const data, bool copyData = true);

    /** Overloaded function, provided for convenience, behaves essentially
        like the function above if data is not 0.
        Uses current width, height, sixteenBit, and alpha values.
        If data is 0, the current data is deleted and the image is set to null
        (But metadata unchanged).
     */
    void        putImageData(uchar* const data, bool copyData = true);

    /** Reset metadata and image data to null image
     */
    void        reset();

    /** Reset metadata, but do not change image data
     */
    void        resetMetaData();

    /** Returns the data of this image.
        Ownership of the buffer is passed to the caller, this image will be null afterwards.
     */
    uchar*      stripImageData();

    bool        load(const QString& filePath, DImgLoaderObserver* const observer = 0,
                     const DRawDecoding& rawDecodingSettings=DRawDecoding());

    bool        load(const QString& filePath,
                     bool loadMetadata, bool loadICCData, bool loadUniqueHash, bool loadHistory,
                     DImgLoaderObserver* const observer = 0,
                     const DRawDecoding& rawDecodingSettings=DRawDecoding());

    bool        save(const QString& filePath, FORMAT frm, DImgLoaderObserver* const observer = 0);
    bool        save(const QString& filePath, const QString& format, DImgLoaderObserver* const observer = 0);

    /**
     * It is common that images are not directly saved to the destination path.
     * For this reason, save() does not call addAsReferredImage(), and the stored
     * save path may be wrong.
     * Call this method after save() with the final destination path.
     * This path will be stored in the image history as well.
     */
    void        imageSavedAs(const QString& savePath);

    /** Loads most parts of the meta information, but never the image data.
        If loadMetadata is true, the metadata will be available with getComments, getExif, getIptc, getXmp .
        If loadICCData is true, the ICC profile will be available with getICCProfile.
     */
    bool        loadImageInfo(const QString& filePath, bool loadMetadata = true,
                              bool loadICCData = true, bool loadUniqueHash = true,
                              bool loadImageHistory = true);

    bool        isNull()         const;
    uint        width()          const;
    uint        height()         const;
    QSize       size()           const;
    uchar*      copyBits()       const;
    uchar*      bits()           const;
    uchar*      scanLine(uint i) const;
    bool        hasAlpha()       const;
    bool        sixteenBit()     const;
    uint        numBytes()       const;
    uint        numPixels()      const;

    /** Return the number of bytes depth of one pixel : 4 (non sixteenBit) or 8 (sixteen)
     */
    int         bytesDepth() const;

    /** Return the number of bits depth of one color component for one pixel : 8 (non sixteenBit) or 16 (sixteen)
     */
    int         bitsDepth()  const;

    /** Returns the file path from which this DImg was originally loaded.
     *  Returns a null string if the DImg was not loaded from a file.
     */
    QString     originalFilePath() const;

    /** Returns the file path to which this DImg was saved.
     *  Returns the file path set with imageSavedAs(), if that was not called,
     *  save(), if that was not called, a null string.
     */
    QString     lastSavedFilePath() const;

    /** Returns the color model in which the image was stored in the file.
        The color space of the loaded image data is always RGB.
    */
    COLORMODEL  originalColorModel() const;

    /** Returns the bit depth (in bits per channel, e.g. 8 or 16) of the original file.
     */
    int         originalBitDepth() const;

    /** Returns the size of the original file.
     */
    QSize       originalSize() const;

    /** Returns the file format in form of the FORMAT enum that was detected in the load()
        method. Other than the format attribute which is written by the DImgLoader,
        this can include the QIMAGE or NONE values.
        Returns NONE for images that have not been loaded.
        For unknown image formats, a value of QIMAGE can be returned to indicate that the
        QImage-based loader will have been used. To find out if this has worked, check
        the return value you got from load().
     */
    FORMAT      detectedFormat() const;

    /** Returns the format string as written by the image loader this image was originally
        loaded from. Format strings used include JPEG, PNG, TIFF, PGF, JP2K, RAW, PPM.
        For images loaded with the platform QImage loader, the file suffix is used.
        Returns null if this DImg was not loaded from a file, but created in memory.
    */
    QString     format() const;

    /** Returns the format string of the format that this image was last saved to.
        An image can be loaded from a file - retrieve that format with fileFormat()
        and loadedFormat() - and can the multiple times be saved to different formats.
        Format strings used include JPG, PGF, PNG, TIFF and JP2K.
        If this file was not save, a null string is returned.
    */
    QString     savedFormat() const;

    /** Returns the DRawDecoding options that this DImg was loaded with.
     *  If this is not a RAW image or no options were specified, returns DRawDecoding().
     */
    DRawDecoding rawDecodingSettings() const;

    /** Access a single pixel of the image.
        These functions add some safety checks and then use the methods from DColor.
        In optimized code working directly on the data,
        better use the inline methods from DColor.
     */
    DColor      getPixelColor(uint x, uint y) const;
    void        setPixelColor(uint x, uint y, const DColor& color);
    void        prepareSubPixelAccess();
    DColor      getSubPixelColor(float x, float y) const;
    DColor      getSubPixelColorFast(float x, float y) const;

    /** If the image has an alpha channel, check if there exist pixels
     *  which actually have non-opaque color, that is alpha < 1.0.
     *  Note that all pixels are scanned to reach a return value of "false".
     *  If hasAlpha() is false, always returns false.
     */
    bool        hasTransparentPixels() const;

    /** Return true if the original image file format cannot be saved.
        This is depending of DImgLoader::save() implementation. For example
        RAW file formats are supported by DImg using dcraw than cannot support
        writing operations.
     */
    bool       isReadOnly() const;

    /** Metadata manipulation methods
     */
    MetaEngineData getMetadata()   const;
    IccProfile getIccProfile() const;
    void       setMetadata(const MetaEngineData& data);
    void       setIccProfile(const IccProfile& profile);

    void       setAttribute(const QString& key, const QVariant& value);
    QVariant   attribute(const QString& key)    const;
    bool       hasAttribute(const QString& key) const;
    void       removeAttribute(const QString& key);

    void       setEmbeddedText(const QString& key, const QString& text);
    QString    embeddedText(const QString& key) const;

    const DImageHistory& getImageHistory() const;
    DImageHistory&       getImageHistory();
    void                 setImageHistory(const DImageHistory& history);
    bool                 hasImageHistory() const;
    DImageHistory        getOriginalImageHistory() const;
    void                 addFilterAction(const FilterAction& action);

    /** Sets a step in the history to constitute the beginning of a branch.
     *  Use setHistoryBranch() to take getOriginalImageHistory() and set the first added step as a branch.
     *  Use setHistoryBranchForLastSteps(n) to start the branch before the last n steps in the history.
     *  (Assume the history had 3 steps and you added 2, call setHistoryBranchForLastSteps(2))
     *  Use setHistoryBranchAfter() if have a copy of the history before branching,
     *  the first added step on top of that history will be made a branch.
     */
    void                 setHistoryBranchAfter(const DImageHistory& historyBeforeBranch, bool isBranch = true);
    void                 setHistoryBranchForLastSteps(int numberOfLastHistorySteps, bool isBranch = true);
    void                 setHistoryBranch(bool isBranch = true);

    /**
     * When saving, several changes to the image metadata are necessary
     * before it can safely be written to the new file.
     * This method updates the stored DMetadata object in preparation to a subsequent
     * call to save() with the same target file.
     * 'intendedDestPath' is the finally intended file name. Do not give the temporary
     *   file name if you are going to save() to a temp file.
     * 'destMimeType' is destination type mime. In some cases, metadata is updated depending on this value.
     * 'originalFileName' is the original file's name, for simplistic history tracking in metadata.
     *   This is completely independent from the DImageHistory framework.
     * For the 'flags' see below.
     * Not all steps are optional and can be controlled with flags.
     */
    enum PrepareMetadataFlag
    {
        /// A small preview can be stored in the metadata.
        /// Remove old preview entries
        RemoveOldMetadataPreviews = 1 << 0,
        /// Create a new preview from current image data.
        CreateNewMetadataPreview  = 1 << 1,
        /// Set the exif orientation tag to "normal"
        /// Applicable if the image data was rotated according to the tag
        ResetExifOrientationTag   = 1 << 2,
        /// Creates a new UUID for the image history.
        /// Applicable if the file was changed.
        CreateNewImageHistoryUUID = 1 << 3,

        PrepareMetadataFlagsAll   = RemoveOldMetadataPreviews |
                                    CreateNewMetadataPreview  |
                                    ResetExifOrientationTag   |
                                    CreateNewImageHistoryUUID
    };
    Q_DECLARE_FLAGS(PrepareMetadataFlags, PrepareMetadataFlag)

    void prepareMetadataToSave(const QString& intendedDestPath,
                               const QString& destMimeType,
                               const QString& originalFileName = QString(),
                               PrepareMetadataFlags flags      = PrepareMetadataFlagsAll);

    /** For convenience: Including all flags, except for ResetExifOrientationTag which can be selected.
     *  Uses originalFilePath() to fill the original file name.
     */
    void prepareMetadataToSave(const QString& intendedDestPath,
                               const QString& destMimeType,
                               bool           resetExifOrientationTag);

    /** Create a HistoryImageId for _this_ image _already_ saved at the given file path.*/
    HistoryImageId createHistoryImageId(const QString& filePath, HistoryImageId::Type type) const;

    /** If you have saved this DImg to filePath, and want to continue using this DImg object
     *  to add further changes to the image history, you can call this method to add to the image history
     *  a reference to the just saved image.
     *  First call updateMetadata(), then call save(), then call addAsReferredImage().
     *  Do not call this directly after loading, before applying any changes:
     *  The history is correctly initialized when loading.
     *  If you need to insert the referred file to an entry which is not the last entry,
     *  which may happen if the added image was saved after this image's history was created,
     *  you can use insertAsReferredImage.
     *  The added id is returned.
     */
    HistoryImageId addAsReferredImage(const QString& filePath, HistoryImageId::Type type = HistoryImageId::Intermediate);
    void           addAsReferredImage(const HistoryImageId& id);
    void           insertAsReferredImage(int afterHistoryStep, const HistoryImageId& otherImagesId);

    /** In the history, adjusts the UUID of the ImageHistoryId of the current file.
     *  Call this if you have associated a UUID with this file which is not written to the metadata.
     *  If there is already a UUID present, read from metadata, it will not be replaced.
     */
    void addCurrentUniqueImageId(const QString& uuid);

    /** When loaded from a file, some attributes like format and isReadOnly still depend on this
        originating file. When saving in a different format to a different file,
        you may wish to switch these attributes to the new file.
        - fileOriginData() returns the current origin data, bundled in the returned QVariant.
        - setFileOriginData() takes such a variant and adjusts the properties
        - lastSavedFileOriginData() returns the origin data as if the image was loaded from
          the last saved image.
        - switchOriginToLastSaved is equivalent to setting origin data returned from lastSavedFileOriginData()

        Example: an image loaded from a RAW and saved to PNG will be read-only and format RAW.
        After calling switchOriginToLastSaved, it will not be read-only, format will be PNG,
        and rawDecodingSettings will be null. detectedFormat() will not change.
        In the history, the last referred image that was added (as intermediate) is made
        the new Current image.
        NOTE: Set the saved image path with imageSavedAs() before!
    */
    QVariant    fileOriginData()          const;
    void        setFileOriginData(const QVariant& data);
    QVariant    lastSavedFileOriginData() const;
    void        switchOriginToLastSaved();

    /** Return a deep copy of full image
      */
    DImg       copy() const;

    /** Return a deep copy of the image, but do not include metadata.
     */
    DImg       copyImageData() const;

    /** Return an image that contains a deep copy of
        this image's metadata and the information associated
        with the image data (width, height, hasAlpha, sixteenBit),
        but no image data, i.e. isNull() is true.
     */
    DImg       copyMetaData() const;

    /** Return a region of image
     */
    DImg       copy(const QRect& rect)          const;
    DImg       copy(const QRectF& relativeRect) const;
    DImg       copy(int x, int y, int w, int h) const;

    /** Copy a region of pixels from a source image to this image.
        Parameters:
        sx|sy  Coordinates in the source image of the rectangle to be copied
        w h    Width and height of the rectangle (Default, or when both are -1: whole source image)
        dx|dy  Coordinates in this image of the rectangle in which the region will be copied
        (Default: 0|0)
        The bit depth of source and destination must be identical.
     */
    void       bitBltImage(const DImg* const src, int dx, int dy);
    void       bitBltImage(const DImg* const src, int sx, int sy, int dx, int dy);
    void       bitBltImage(const DImg* const src, int sx, int sy, int w, int h, int dx, int dy);
    void       bitBltImage(const uchar* const src, int sx, int sy, int w, int h, int dx, int dy,
                           uint swidth, uint sheight, int sdepth);

    /** Blend src image on this image (this is dest) with the specified composer
        and multiplication flags. See documentation of DColorComposer for more info.
        For the other arguments, see documentation of bitBltImage above.
     */
    void       bitBlendImage(DColorComposer* const composer, const DImg* const src,
                             int sx, int sy, int w, int h, int dx, int dy,
                             DColorComposer::MultiplicationFlags multiplicationFlags =
                                 DColorComposer::NoMultiplication);

    /** For the specified region, blend this image on the given color with the specified
        composer and multiplication flags. See documentation of DColorComposer for more info.
        Note that the result pixel is again written to this image, which is, for the blending, source.
     */
    void       bitBlendImageOnColor(DColorComposer* const composer, const DColor& color,
                                    int x, int y, int w, int h,
                                    DColorComposer::MultiplicationFlags multiplicationFlags =
                                        DColorComposer::NoMultiplication);
    void       bitBlendImageOnColor(const DColor& color, int x, int y, int w, int h);
    void       bitBlendImageOnColor(const DColor& color);

    /** QImage wrapper methods
     */
    QImage     copyQImage()                           const;
    QImage     copyQImage(const QRect& rect)          const;
    QImage     copyQImage(const QRectF& relativeRect) const;
    QImage     copyQImage(int x, int y, int w, int h) const;

    /** Crop image to the specified region
     */
    void       crop(const QRect& rect);
    void       crop(int x, int y, int w, int h);

    /** Set width and height of this image, smoothScale it to the given size
     */
    void       resize(int w, int h);

    /**
     * If the image has an alpha channel and transparent pixels,
     * it will be blended on the specified color and the alpha channel will be removed.
     * This is a no-op if hasTransparentPixels() is false, but this method can be expensive,
     * therefore it is _not_ checked inside removeAlphaChannel().
     * (the trivial hasAlpha() is checked)
     */
    void       removeAlphaChannel(const DColor& destColor);
    void       removeAlphaChannel();

    /** Return a version of this image scaled to the specified size with the specified mode.
        See QSize documentation for information on available modes
     */
    DImg       smoothScale(int width, int height, Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio) const;
    DImg       smoothScale(const QSize& destSize, Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio) const;

    /** Executes the same scaling as smoothScale(width, height), but from the result of this call,
     *  returns only the section specified by clipx, clipy, clipwidth, clipheight.
     *  This is thus equivalent to calling
     *  Dimg scaled = smoothScale(width, height); scaled.crop(clipx, clipy, clipwidth, clipheight);
     *  but potentially much faster.
     *  In smoothScaleSection, you specify the source region, here, the result region.
     *  It will often not be possible to find _integer_ source coordinates for a result region!
     */
    DImg smoothScaleClipped(int width, int height, int clipx, int clipy, int clipwidth, int clipheight) const;
    DImg smoothScaleClipped(const QSize& destSize, const QRect& clip) const;

    /** Take the region specified by the rectangle sx|sy, width and height sw * sh,
        and scale it to an image with size dw * dh
     */
    DImg       smoothScaleSection(int sx, int sy, int sw, int sh, int dw, int dh) const;
    DImg       smoothScaleSection(const QRect& sourceRect, const QSize& destSize) const;

    void       rotate(ANGLE angle);
    void       flip(FLIP direction);

    /** Rotates and/or flip the DImg according to the given DMetadata::Orientation,
     *  so that the current state is orientation and the resulting step is normal orientation.
     *  Returns true if the image was actually rotated or flipped (e.g. if ORIENTATION_NORMAL
     *  is given, returns false, because no action is taken).
     */
    bool       rotateAndFlip(int orientation);

    /** Reverses the previous function.
     */
    bool       reverseRotateAndFlip(int orientation);

    /** Rotates and/or flip the DImg according to the given transform action,
     *  which is a MetaEngineRotation::TransformAction.
     *  Returns true if the image was actually rotated or flipped.
     */
    bool       transform(int transformAction);

    QPixmap    convertToPixmap()                              const;
    QPixmap    convertToPixmap(IccTransform& monitorICCtrans) const;

    /** Return a mask image where pure white and pure black pixels are over-colored.
        This way is used to identify over and under exposed pixels.
     */
    QImage     pureColorMask(ExposureSettingsContainer* const expoSettings) const;

    /** Convert depth of image. Depth is bytesDepth * bitsDepth.
        If depth is 32, converts to 8 bits,
        if depth is 64, converts to 16 bits.
     */
    void       convertDepth(int depth);

    /** Wrapper methods for convertDepth
     */
    void       convertToSixteenBit();
    void       convertToEightBit();
    void       convertToDepthOfImage(const DImg* const otherImage);

    /** Fill whole image with specified color.
        The bit depth of the color must be identical to the depth of this image.
     */
    void       fill(const DColor& color);


    /** This methods return a 128-bit MD5 hex digest which is meant to uniquely identify
        the file. The hash is calculated on parts of the file and the file metadata.
        It cannot be used to find similar images. It is not calculated from the image data.
        The hash will be returned as a 32-byte hexadecimal string.

        If you already have a DImg object of the file, use the member method.
        The object does not need to have the full image data loaded, but it shall at least
        have been loaded with loadImageInfo with loadMetadata = true, or have the metadata
        set later with setComments, setExif, setIptc, setXmp.
        If the object does not have the metadata loaded, a non-null, but invalid hash will
        be returned! In this case, use the static method.
        If the image has been loaded with loadUniqueHash = true, the hash can be retrieved
        with the member method.

        You do not need a DImg object of the file to retrieve the unique hash;
        Use the static method and pass just the file path.
     */
    QByteArray getUniqueHash() const;
    static QByteArray getUniqueHash(const QString& filePath);

    /** This methods return a 128-bit MD5 hex digest which is meant to uniquely identify
        the file. The hash is calculated on parts of the file.
        It cannot be used to find similar images. It is not calculated from the image data.
        The hash will be returned as a 32-byte hexadecimal string.

        If you already have a DImg object loaded from the file, use the member method.
        If the image has been loaded with loadUniqueHash = true, the hash will already
        be available.

        You do not need a DImg object of the file to retrieve the unique hash;
        Use the static method and pass just the file path.
     */
    QByteArray getUniqueHashV2() const;
    static QByteArray getUniqueHashV2(const QString& filePath);

    /** This method creates a new 256-bit UUID meant to be globally unique.
     *  The UUID will be returned as a 64-byte hexadecimal string.
     *  At least 128bits of the UUID will be created by the platform random number
     *  generator. The rest may be created from a content-based hash similar to the uniqueHash, see above.
     *  This method only generates a new UUID for this image without in any way changing this image object
     *  or saving the UUID anywhere.
     */
    QByteArray createImageUniqueId() const;

    /**
     * Helper method to translate enum values to user presentable strings
     */
    static QString colorModelToString(COLORMODEL colorModel);

    /** Return true if image file is an animation, as GIFa or NMG
     */
    static bool isAnimatedImage(const QString& filePath);

private:

    DImg(const DImg& image, int w, int h);

    bool load(const QString& filePath, int loadFlags, DImgLoaderObserver* const observer,
              const DRawDecoding& rawDecodingSettings=DRawDecoding());
    void copyMetaData(const Private* const src);
    void copyImageData(const Private* const src);
    void setImageData(bool null, uint width, uint height, bool sixteenBit, bool alpha);
    void setImageDimension(uint width, uint height);
    size_t allocateData();

    static void bitBlt(const uchar* const src, uchar* const dest,
                       int sx, int sy, int w, int h, int dx, int dy,
                       uint swidth, uint sheight, uint dwidth, uint dheight,
                       bool sixteenBit, int sdepth, int ddepth);
    static void bitBlend(DColorComposer* const composer, uchar* const src, uchar* const dest,
                         int sx, int sy, int w, int h, int dx, int dy,
                         uint swidth, uint sheight, uint dwidth, uint dheight,
                         bool sixteenBit, int sdepth, int ddepth,
                         DColorComposer::MultiplicationFlags multiplicationFlags);
    static void bitBlendOnColor(DColorComposer* const composer, const DColor& color,
                                uchar* data, int x, int y, int w, int h,
                                uint width, uint height, bool sixteenBit, int depth,
                                DColorComposer::MultiplicationFlags multiplicationFlags);
    static bool normalizeRegionArguments(int& sx, int& sy, int& w, int& h, int& dx, int& dy,
                                         uint swidth, uint sheight, uint dwidth, uint dheight);

private:

    DSharedDataPointer<Private> m_priv;

    friend class DImgLoader;
};

}  // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DImg::PrepareMetadataFlags)

#endif /* DIMG_H */
