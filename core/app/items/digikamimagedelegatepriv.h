/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAMIMAGEDELEGATEPRIV_H
#define DIGIKAMIMAGEDELEGATEPRIV_H

// Qt includes

#include <QCache>

// Local includes

#include "imagedelegatepriv.h"

namespace Digikam
{

class DigikamImageDelegatePrivate : public ImageDelegate::ImageDelegatePrivate
{
public:

    DigikamImageDelegatePrivate()
    {
    }
    virtual ~DigikamImageDelegatePrivate();

    void init(DigikamImageDelegate* const q, ImageCategorizedView* const parent);
};

// -----------------------------------------------------------------------------------------

class DigikamImageFaceDelegatePrivate : public DigikamImageDelegatePrivate
{
public:

    DigikamImageFaceDelegatePrivate()
    {
    }
};

} // namespace Digikam

#endif // DIGIKAMIMAGEDELEGATEPRIV_H
