/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 1997-02-20
 * Description : color chooser widgets
 *
 * Copyright (C) 1997 by Martin Jones (mjones at kde dot org)
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

#ifndef DHUESATURATIONSELECT_H
#define DHUESATURATIONSELECT_H

// Local includes

#include "digikam_export.h"
#include "dcolorchoosermode.h"
#include "dpointselect.h"

namespace Digikam
{

class DIGIKAM_EXPORT DHueSaturationSelector : public DPointSelect
{
    Q_OBJECT

public:

    /**
     * Constructs a hue/saturation selection widget.
     */
    explicit DHueSaturationSelector(QWidget* const parent = 0);

    /**
     * Destructor.
     */
    ~DHueSaturationSelector();

    /**
     * Sets the chooser mode. The allowed modes are defined
     * in DColorChooserMode.
     * 
     * @param    The chooser mode as defined in DColorChooserMode
     */
    void setChooserMode(DColorChooserMode chooserMode);

    /**
     * Returns the chooser mode.
     * 
     * @return   The chooser mode (defined in DColorChooserMode)
     */
    DColorChooserMode chooserMode() const;


    /**
     * Returns the hue value
     * 
     * @return   The hue value (0-360)
     */
    int hue() const;

    /**
     * Sets the hue value (0-360)
     * 
     * @param  hue  The hue value (0-360)
     */
    void setHue(int hue);

    /**
     * Returns the saturation (0-255)
     * 
     * @return The saturation (0-255)
     */
    int saturation() const;

    /**
     * Sets the saturation (0-255)
     * 
     * @param  saturation   The saturation (0-255)
     */
    void setSaturation(int saturation);

    /**
     * Returns the color value (also known as lumniousity, 0-255)
     * 
     * @return  The color value (0-255)
     */
    int colorValue() const;

    /**
     * Sets the color value (0-255)
     * 
     * @param  colorValue  The color value (0-255)
     */
    void setColorValue(int color);

    /**
     * Updates the contents
     */
    void updateContents();

protected:

    /**
     * Draws the contents of the widget on a pixmap,
     * which is used for buffering.
     */
    virtual void drawPalette(QPixmap* pixmap);
    virtual void resizeEvent(QResizeEvent*);

    /**
     * Reimplemented from DPointSelect. This drawing is
     * buffered in a pixmap here. As real drawing
     * routine, drawPalette() is used.
     */
    virtual void drawContents(QPainter* painter);

private:

    class Private;
    friend class Private;
    Private* const d;

    Q_DISABLE_COPY(DHueSaturationSelector)
};

}  // namespace Digikam

#endif /* DHUESATURATIONSELECT_H */
