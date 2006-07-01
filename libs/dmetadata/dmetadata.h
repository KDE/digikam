/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : image metadata interface
 *
 * Copyright 2006 by Gilles Caulier
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

#include <string>

// QT includes.

#include <qcstring.h>
#include <qimage.h>
#include <qdatetime.h>

// Exiv2 includes.

#include <exiv2/types.hpp>
#include <exiv2/exif.hpp>

// Local includes.

#include "dimg.h"
#include "photoinfocontainer.h"
#include "digikam_export.h"

namespace Digikam
{

class DMetadataPriv;

class DIGIKAM_EXPORT DMetadata
{

public:

    enum ImageOrientation
    {
        ORIENTATION_UNSPECIFIED  = 0, 
        ORIENTATION_NORMAL       = 1, 
        ORIENTATION_HFLIP        = 2, 
        ORIENTATION_ROT_180      = 3, 
        ORIENTATION_VFLIP        = 4, 
        ORIENTATION_ROT_90_HFLIP = 5, 
        ORIENTATION_ROT_90       = 6, 
        ORIENTATION_ROT_90_VFLIP = 7, 
        ORIENTATION_ROT_270      = 8
    };

    // Exif color workspace values.
    // TODO : add MakerNote workspace values
    enum ImageColorWorkSpace
    {
        WORKSPACE_UNSPECIFIED  = 0,    
        WORKSPACE_SRGB         = 1,    
        WORKSPACE_ADOBERGB     = 2,    
        WORKSPACE_UNCALIBRATED = 65535 
    };

public:

    DMetadata();
    ~DMetadata();
    
    /** Load Metadata from image file */
    DMetadata(const QString& filePath, DImg::FORMAT ff=DImg::NONE);

    bool applyChanges();

    /** File access method */
    bool load(const QString& filePath, DImg::FORMAT ff=DImg::NONE);
    bool save(const QString& filePath, DImg::FORMAT ff);

    /** Metadata manipulation methods */
    QByteArray getComments() const;
    QByteArray getExif() const;
    QByteArray getIptc(bool addIrbHeader=false) const;

    void setComments(const QByteArray& data);
    void setExif(const QByteArray& data);
    void setIptc(const QByteArray& data);

    void setExif(Exiv2::DataBuf const data);
    void setIptc(Exiv2::DataBuf const data);

    QString             getExifTagString(const char *exifTagName) const;
    QByteArray          getExifTagData(const char *exifTagName) const;
    QImage              getExifThumbnail(bool fixOrientation) const;
    QByteArray          getIptcTagData(const char *iptcTagName) const;

    ImageColorWorkSpace getImageColorWorkSpace();
    bool                getImagePreview(QImage& preview);
    QSize               getImageDimensions();
    ImageOrientation    getImageOrientation();
    QDateTime           getImageDateTime() const;
    QString             getImageComment() const;
    int                 getImageRating() const;
    QStringList         getImageKeywords() const;

    bool setExifTagString(const char *exifTagName, const QString& value);
    bool setExifThumbnail(const QImage& thumb);
    bool setImageColorWorkSpace(ImageColorWorkSpace workspace);
    bool setImagePreview(const QImage& preview);
    bool setImageDimensions(const QSize& size);
    bool setImageOrientation(ImageOrientation orientation);
    bool setImageDateTime(const QDateTime& dateTime, bool setDateTimeDigitized = false);
    bool setImageComment(const QString& comment);
    bool setImageRating(int rating);
    bool setImageKeywords(const QStringList& oldKeywords, const QStringList& newKeywords);
    bool setImagePhotographerId(const QString& author, const QString& authorTitle);
    bool setImageCredits(const QString& credit, const QString& source, const QString& copyright);

    PhotoInfoContainer getPhotographInformations() const;

    static QString convertCommentValue(const Exiv2::Exifdatum &comment);
    static QString detectEncodingAndDecode(const std::string &value);

private:

    bool         setImageProgramId();

private:

    DMetadataPriv *d;
    
    friend class DMetaLoader;
};

}  // NameSpace Digikam

#endif /* DMETADATA_H */
