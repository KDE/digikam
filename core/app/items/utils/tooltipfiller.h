/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : album icon view tool tip
 *
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_TOOL_TIP_FILLER_H
#define DIGIKAM_TOOL_TIP_FILLER_H

// Qt includes

#include <QString>

namespace Digikam
{

class FilterAction;
class ItemInfo;
class PAlbum;

namespace ToolTipFiller
{

    QString imageInfoTipContents(const ItemInfo& info);
    QString albumTipContents(PAlbum* const album, int count);
    QString filterActionTipContents(const FilterAction& action);

} // namespace ToolTipFiller

} // namespace Digikam

#endif // DIGIKAM_TOOL_TIP_FILLER_H
