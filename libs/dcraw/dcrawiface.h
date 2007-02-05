/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2006-06-29
 * Description : dcraw interface
 *
 * Copyright 2006-2007 by Gilles Caulier
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

// Qt Includes.

#include <qstring.h>
#include <qimage.h>

// Local Includes.

#include "dcrawinfocontainer.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DcrawIface
{

public:

    static bool loadDcrawPreview(QImage& image, const QString& path);

    static bool rawFileIdentify(DcrawInfoContainer& identify, const QString& path);
};

}  // namespace Digikam
