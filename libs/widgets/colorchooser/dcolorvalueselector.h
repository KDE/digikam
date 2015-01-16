/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 1997-02-20
 * Description : color chooser widgets
 *
 * Copyright (C) 1997 by Martin Jones <mjones at kde dot org>
 * Copyright (C) 2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DCOLORVALUESELECTOR_H
#define DCOLORVALUESELECTOR_H

// Qt includes

#include <QPixmap>

// KDE includes

#include <kselector.h>

// Local includes

#include "dcolorchoosermode.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DColorValueSelector : public KSelector
{
    Q_OBJECT
    Q_PROPERTY( int hue        READ hue        WRITE setHue)
    Q_PROPERTY( int saturation READ saturation WRITE setSaturation)
    Q_PROPERTY( int colorValue READ colorValue WRITE setColorValue)

public:

    /**
    * Constructs a widget for color selection.
    */
    explicit DColorValueSelector(QWidget* const parent=0);

    /**
    * Constructs a widget for color selection with a given orientation
    */
    explicit DColorValueSelector(Qt::Orientation o, QWidget* const parent = 0);

    ~DColorValueSelector();

    /**
    * Updates the widget's contents.
    */
    void updateContents();

    /**
    * Returns the current hue value.
    * 
    * @return The hue value (0-359)
    */
    int hue() const;

    /**
    * Sets the hue value. Doesn't automatically update the widget;
    * you have to call updateContents manually.
    * 
    * @param hue Sets the hue value (0-359)
    */
    void setHue(int hue);

    /**
    * Returns the current saturation value.
    * 
    * @return The saturation value (0-255)
    */  
    int saturation() const;

    /**
    * Sets the saturation value. Doesn't automatically update the widget;
    * you have to call updateContents manually.
    * 
    * @param saturation Sets the saturation value (0-255)
    */
    void setSaturation(int saturation);

    /**
    * Returns the current color value.
    * 
    * @return The color value (0-255)
    */
    int colorValue() const;

    /**
    * Sets the color value. Doesn't automatically update the widget;
    * you have to call updateContents manually.
    * 
    * @param colorValue Sets the color value (0-255)
    */
    void setColorValue(int colorValue);

    /**
    * Sets the chooser mode. Doesn't automatically update the widget;
    * you have to call updateContents manually.
    *
    * @param chooserMode Sets the chooser mode (one of the DColorChooserMode constants)
    */
    void setChooserMode(DColorChooserMode chooserMode);

    /**
    * Returns the current chooser mode.
    * 
    * @return The chooser mode (one of the DColorChooserMode constants)
    */
    DColorChooserMode chooserMode() const;

protected:

    /**
    * Draws the contents of the widget on a pixmap,
    * which is used for buffering.
    */
    virtual void drawPalette(QPixmap* pixmap);
    virtual void resizeEvent(QResizeEvent*);

    /**
    * Reimplemented from KSelector. The drawing is
    * buffered in a pixmap here. As real drawing
    * routine, drawPalette() is used.
    */
    virtual void drawContents(QPainter* painter);

private:

    class Private;
    friend class Private;
    Private *const d;

    Q_DISABLE_COPY(DColorValueSelector)
};

}  // namespace Digikam

#endif /* DCOLORVALUESELECTOR_H */
