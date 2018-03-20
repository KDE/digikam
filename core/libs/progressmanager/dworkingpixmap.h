/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : A working pixmap manager.
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DWORKING_PIXMAP_H
#define DWORKING_PIXMAP_H

// Qt includes

#include <QString>
#include <QSize>
#include <QPixmap>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/** A widget to draw progress wheel indicator over thumbnails.
 */
class DIGIKAM_EXPORT DWorkingPixmap
{
public:

    explicit DWorkingPixmap();
    virtual ~DWorkingPixmap();

    bool    isEmpty()          const;
    QSize   frameSize()        const;
    int     frameCount()       const;
    QPixmap frameAt(int index) const;

private:

    QVector<QPixmap> m_frames;
};

} // namespace Digikam

#endif // DWORKING_PIXMAP_H
