/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PIXELIZEPHOTOEFFECT_H
#define PIXELIZEPHOTOEFFECT_H

// Local includes

#include "PhotoEffectsLoader.h"

using namespace PhotoLayoutsEditor;

namespace PhotoLayoutsEditor
{

class PixelizePhotoEffect : public PhotoEffectsLoader
{
    Q_OBJECT

    int m_pixelSize;
    class PixelizeUndoCommand;

public:

    explicit PixelizePhotoEffect(int pixelSize, QObject* parent = 0);
    virtual QtAbstractPropertyBrowser* propertyBrowser() const;
    virtual QString toString() const;

    /**
     * Pixel size property
     */
    Q_PROPERTY(int m_pixelSize READ pixelSize WRITE setPixelSize)

    void setPixelSize(int pixelSize)
    {
        m_pixelSize = pixelSize;
        emit effectChanged(this);
    }

    int pixelSize() const
    {
        return m_pixelSize;
    }

    /**
     * Pixelize effect identifier (name).
     */
    virtual QString effectName() const;

protected:

    static const QString PIXEL_SIZE_STRING;
    virtual QImage apply(const QImage &image);

protected Q_SLOTS:

    virtual void propertyChanged(QtProperty* property);

private:

    static inline QImage pixelize(const QImage & image, int pixelSize)
    {
        Q_ASSERT(pixelSize > 0);

        if (pixelSize <= 1)
            return image;

        int width     = image.width();
        int height    = image.height();
        QImage result = image.copy(image.rect());

        for (int y = 0; y < height; y += pixelSize)
        {
            int ys           = qMin(height - 1, y + pixelSize / 2);
            QRgb* const sbuf = reinterpret_cast<QRgb*>(result.scanLine(ys));

            for (int x = 0; x < width; x += pixelSize)
            {
                int xs     = qMin(width - 1, x + pixelSize / 2);
                QRgb color = sbuf[xs];

                for (int yi = 0; yi < qMin(pixelSize, height - y); ++yi)
                {
                    QRgb* const buf = reinterpret_cast<QRgb*>(result.scanLine(y + yi));

                    for (int xi = 0; xi < qMin(pixelSize, width - x); ++xi)
                        buf[x + xi] = color;
                }
            }
        }
        return result;
    }
};

}

#endif // PIXELIZEPHOTOEFFECT_H
