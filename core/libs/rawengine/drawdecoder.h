/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-12-09
 * Description : a tread-safe libraw C++ program interface
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007-2008 by Guillaume Castagnino <casta at xwing dot info>
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

#ifndef DRAW_DECODER_H
#define DRAW_DECODER_H

// C++ includes

#include <cmath>

// Qt includes

#include <QBuffer>
#include <QString>
#include <QObject>
#include <QImage>

// Local includes

#include "drawdecodersettings.h"
#include "rawinfo.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DRawDecoder : public QObject
{
    Q_OBJECT

public:

    /** Standard constructor.
     */
    DRawDecoder();

    /** Standard destructor.
     */
    virtual ~DRawDecoder();

public:

    /** Get the preview of RAW picture as a QImage.
        It tries loadEmbeddedPreview() first and if it fails, calls loadHalfPreview().
     */
    static bool loadRawPreview(QImage& image, const QString& path);

    /** Get the preview of RAW picture as a QByteArray holding JPEG data.
        It tries loadEmbeddedPreview() first and if it fails, calls loadHalfPreview().
     */
    static bool loadRawPreview(QByteArray& imgData, const QString& path);

    /** Get the preview of RAW picture passed in QBuffer as a QByteArray holding JPEG data.
        It tries loadEmbeddedPreview() first and if it fails, calls loadHalfPreview().
     */
    static bool loadRawPreview(QByteArray& imgData, const QBuffer& inBuffer);

    /** Get the embedded JPEG preview image from RAW picture as a QByteArray which will include Exif Data.
        This is fast and non cancelable. This method does not require a class instance to run.
     */
    static bool loadEmbeddedPreview(QByteArray& imgData, const QString& path);

    /** Get the embedded JPEG preview image from RAW picture as a QImage. This is fast and non cancelable
        This method does not require a class instance to run.
     */
    static bool loadEmbeddedPreview(QImage& image, const QString& path);

    /** Get the embedded JPEG preview image from RAW image passed in QBuffer as a QByteArray which will include Exif Data.
        This is fast and non cancelable. This method does not require a class instance to run.
     */
    static bool loadEmbeddedPreview(QByteArray& imgData, const QBuffer& inBuffer);

    /** Get the half decoded RAW picture. This is slower than loadEmbeddedPreview() method
        and non cancelable. This method does not require a class instance to run.
     */
    static bool loadHalfPreview(QImage& image, const QString& path);

    /** Get the half decoded RAW picture as JPEG data in QByteArray. This is slower than loadEmbeddedPreview()
        method and non cancelable. This method does not require a class instance to run.
     */
    static bool loadHalfPreview(QByteArray& imgData, const QString& path);

    /** Get the half decoded RAW picture passed in QBuffer as JPEG data in QByteArray. This is slower than loadEmbeddedPreview()
        method and non cancelable. This method does not require a class instance to run.
     */
    static bool loadHalfPreview(QByteArray& imgData, const QBuffer& inBuffer);

    /** Get the full decoded RAW picture. This is a more slower than loadHalfPreview() method
        and non cancelable. This method does not require a class instance to run.
     */
    static bool loadFullImage(QImage& image, const QString& path, const DRawDecoderSettings& settings = DRawDecoderSettings());

    /** Get the camera settings witch have taken RAW file. Look into rawinfo.h
        for more details. This is a fast and non cancelable method witch do not require
        a class instance to run.
     */
    static bool rawFileIdentify(RawInfo& identify, const QString& path);

    /** Return the string of all RAW file type mime supported.
     */
    static const char* rawFiles();

    /** Return the list of all RAW file type mime supported,
        as a QStringList, without wildcard and suffix dot.
     */
    static QStringList rawFilesList();

    /** Returns a version number for the list of supported RAW file types.
        This version is incremented if the list of supported formats has changed
        between library releases.
     */
    static int rawFilesVersion();

    /** Provide a list of supported RAW Camera name.
     */
    static QStringList supportedCamera();

    /** Return LibRaw version string.
     */
    static QString librawVersion();

    /** Return true or false if LibRaw use parallel demosaicing or not (libgomp support).
     *  Return -1 if undefined.
     */
    static int librawUseGomp();

    static bool isRawFile(const QUrl& url);

public:

    /** Extract Raw image data undemosaiced and without post processing from 'filePath' picture file.
        This is a cancelable method which require a class instance to run because RAW pictures loading
        can take a while.

        This method return:

            - A byte array container 'rawData' with raw data.
            - All info about Raw image into 'identify' container.
            - 'false' is returned if loadding failed, else 'true'.
     */
    bool extractRAWData(const QString& filePath, QByteArray& rawData, RawInfo& identify, unsigned int shotSelect=0);

    /** Extract a small size of decode RAW data from 'filePath' picture file using
        'DRawDecoderSettings' settings. This is a cancelable method which require
        a class instance to run because RAW pictures decoding can take a while.

        This method return:

            - A byte array container 'imageData' with picture data. Pixels order is RGB.
              Color depth can be 8 or 16. In 8 bits you can access to color component
              using (uchar*), in 16 bits using (ushort*).

            - Size size of image in number of pixels ('width' and 'height').
            - The max average of RGB components from decoded picture.
            - 'false' is returned if decoding failed, else 'true'.
     */
    bool decodeHalfRAWImage(const QString& filePath, const DRawDecoderSettings& DRawDecoderSettings,
                            QByteArray& imageData, int& width, int& height, int& rgbmax);

    /** Extract a full size of RAW data from 'filePath' picture file using
        'DRawDecoderSettings' settings. This is a cancelable method which require
        a class instance to run because RAW pictures decoding can take a while.

        This method return:

            - A byte array container 'imageData' with picture data. Pixels order is RGB.
              Color depth can be 8 or 16. In 8 bits you can access to color component
              using (uchar*), in 16 bits using (ushort*).

            - Size size of image in number of pixels ('width' and 'height').
            - The max average of RGB components from decoded picture.
            - 'false' is returned if decoding failed, else 'true'.
     */
    bool decodeRAWImage(const QString& filePath, const DRawDecoderSettings& DRawDecoderSettings,
                        QByteArray& imageData, int& width, int& height, int& rgbmax);

    /** To cancel 'decodeHalfRAWImage' and 'decodeRAWImage' methods running
        in a separate thread.
     */
    void cancel();

protected:

    /** Used internally to cancel RAW decoding operation. Normally, you don't need to use it
        directly, excepted if you derivated this class. Usual way is to use cancel() method
     */
    bool                m_cancel;

    /** The settings container used to perform RAW pictures decoding. See 'rawdecodingsetting.h'
        for details.
     */
    DRawDecoderSettings m_decoderSettings;

protected:

    /** Re-implement this method to control the cancelisation of loop witch wait data
        from RAW decoding process with your propers envirronement.
        By default, this method check if m_cancel is true.
     */
    virtual bool checkToCancelWaitingData();

    /** Re-implement this method to control the pseudo progress value during RAW decoding (when dcraw run with an
        internal loop without feedback) with your proper environment. By default, this method does nothing.
        Progress value average for this stage is 0%-n%, with 'n' == 40% max (see setWaitingDataProgress() method).
     */
    virtual void setWaitingDataProgress(double value);

public:

    // Declared public to be called externally by callbackForLibRaw() static method.
    class Private;

private:

    Private* const d;

    friend class Private;
};

} // namespace Digikam

#endif // DRAW_DECODER_H
