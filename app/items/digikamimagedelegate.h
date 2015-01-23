/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAMIMAGEDELEGATE_H
#define DIGIKAMIMAGEDELEGATE_H

// Local includes

#include "imagedelegate.h"

namespace Digikam
{

class ImageCategoryDrawer;
class DigikamImageDelegatePrivate;

class DigikamImageDelegate : public ImageDelegate
{
    Q_OBJECT

public:

    explicit DigikamImageDelegate(ImageCategorizedView* parent);
    ~DigikamImageDelegate();

protected:

    virtual void updateRects();

    DigikamImageDelegate(DigikamImageDelegatePrivate& dd, ImageCategorizedView* parent);

private:

    Q_DECLARE_PRIVATE(DigikamImageDelegate)
};

} // namespace Digikam

#endif /* DIGIKAMIMAGEDELEGATE_H */
