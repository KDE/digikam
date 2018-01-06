/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : album icon view tool tip
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TOOLTIPFILLER_H
#define TOOLTIPFILLER_H

// Qt includes

#include <QString>

namespace Digikam
{

class FilterAction;
class ImageInfo;
class PAlbum;

namespace ToolTipFiller
{

    QString imageInfoTipContents(const ImageInfo& info);
    QString albumTipContents(PAlbum* const album, int count);
    QString filterActionTipContents(const FilterAction& action);

} // namespace ToolTipFiller

} // namespace Digikam

#endif /* TOOLTIPFILLER_H */
