/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-02-15
 * Description : a led indicator.
 * 
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef STATUS_LED_H
#define STATUS_LED_H

// Qt includes.

#include <qlabel.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class StatusNavigateBarPriv;

class DIGIKAM_EXPORT StatusLed : public QLabel
{
Q_OBJECT

public:

    enum LedColor
    {
        Gray=0,
        Green,
        Red
    };

public:

    StatusLed(QWidget *parent=0);
    ~StatusLed();

    LedColor ledColor() const;

public slots:

    void setLedColor(LedColor color);

private:

    LedColor m_color;
};

}  // namespace Digikam

#endif /* STATUS_LED_H */
