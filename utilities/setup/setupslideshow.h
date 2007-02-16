/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-05-21
 * Description : setup tab for slideshow options.
 * 
 * Copyright 2005-2007 by Gilles Caulier
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

#ifndef SETUPSLIDESHOW_H
#define SETUPSLIDESHOW_H

// Qt includes.

#include <qwidget.h>

// Local Includes.

#include "digikam_export.h"

namespace Digikam
{

class SetupSlideShowPriv;

class DIGIKAM_EXPORT SetupSlideShow : public QWidget
{
    Q_OBJECT
    
public:

    SetupSlideShow(QWidget* parent = 0);
    ~SetupSlideShow();

    void applySettings();

private:

    void readSettings();

private:

    SetupSlideShowPriv* d;

};

}   // namespace Digikam

#endif /* SETUPSLIDESHOW_H */
