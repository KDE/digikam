/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Embedded preview loading.
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

// Local includes

#include "metaengine_previews.h"
#include "metaengine_p.h"
#include "metaengine.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN MetaEnginePreviews::Private
{
public:

    Private()
    {
        manager = 0;
    }

    ~Private()
    {
        delete manager;
    }

    void load(Exiv2::Image::AutoPtr image_)
    {
        image                              = image_;

        image->readMetadata();

        manager                            = new Exiv2::PreviewManager(*image);
        Exiv2::PreviewPropertiesList props = manager->getPreviewProperties();

        // reverse order of list, which is smallest-first
        Exiv2::PreviewPropertiesList::reverse_iterator it;

        for (it = props.rbegin() ; it != props.rend() ; ++it)
        {
            properties << *it;
        }
    }

public:

    Exiv2::Image::AutoPtr           image;
    Exiv2::PreviewManager*          manager;
    QList<Exiv2::PreviewProperties> properties;
};

MetaEnginePreviews::MetaEnginePreviews(const QString& filePath)
    : d(new Private)
{
    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)(QFile::encodeName(filePath).constData()));
        d->load(image);
    }
    catch( Exiv2::Error& e )
    {
        MetaEngine::Private::printExiv2ExceptionError(QString::fromLatin1("Cannot load metadata using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }
}

MetaEnginePreviews::MetaEnginePreviews(const QByteArray& imgData)
    : d(new Private)
{
    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((Exiv2::byte*)imgData.data(), imgData.size());
        d->load(image);
    }
    catch( Exiv2::Error& e )
    {
        MetaEngine::Private::printExiv2ExceptionError(QString::fromLatin1("Cannot load metadata using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }
}

MetaEnginePreviews::~MetaEnginePreviews()
{
    delete d;
}

bool MetaEnginePreviews::isEmpty()
{
    return d->properties.isEmpty();
}

QSize MetaEnginePreviews::originalSize() const
{
    if (d->image.get())
        return QSize(d->image->pixelWidth(), d->image->pixelHeight());

    return QSize();
}

QString MetaEnginePreviews::originalMimeType() const
{
    if (d->image.get())
        return QString::fromLatin1(d->image->mimeType().c_str());

    return QString();
}

int MetaEnginePreviews::count()
{
    return d->properties.size();
}

int MetaEnginePreviews::dataSize(int index)
{
    if (index < 0 || index >= size()) return 0;

    return d->properties[index].size_;
}

int MetaEnginePreviews::width(int index)
{
    if (index < 0 || index >= size()) return 0;

    return d->properties[index].width_;
}

int MetaEnginePreviews::height(int index)
{
    if (index < 0 || index >= size()) return 0;

    return d->properties[index].height_;
}

QString MetaEnginePreviews::mimeType(int index)
{
    if (index < 0 || index >= size()) return QString();

    return QString::fromLatin1(d->properties[index].mimeType_.c_str());
}

QString MetaEnginePreviews::fileExtension(int index)
{
    if (index < 0 || index >= size()) return QString();

    return QString::fromLatin1(d->properties[index].extension_.c_str());
}

QByteArray MetaEnginePreviews::data(int index)
{
    if (index < 0 || index >= size()) return QByteArray();

    qCDebug(DIGIKAM_METAENGINE_LOG) << "index: "         << index;
    qCDebug(DIGIKAM_METAENGINE_LOG) << "d->properties: " << count();

    try
    {
        Exiv2::PreviewImage image = d->manager->getPreviewImage(d->properties[index]);
        return QByteArray((const char*)image.pData(), image.size());
    }
    catch( Exiv2::Error& e )
    {
        MetaEngine::Private::printExiv2ExceptionError(QString::fromLatin1("Cannot load metadata using Exiv2 "), e);
        return QByteArray();
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
        return QByteArray();
    }
}

QImage MetaEnginePreviews::image(int index)
{
    QByteArray previewData = data(index);
    QImage     image;

    if (!image.loadFromData(previewData))
        return QImage();

    return image;
}

} // namespace Digikam
