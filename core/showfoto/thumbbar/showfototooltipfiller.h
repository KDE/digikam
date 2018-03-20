/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 09-08-2013
 * Description : Showfoto tool tip filler
 *
 * Copyright (C) 2013 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef SHOWFOTOTOOLTIPFILLER_H
#define SHOWFOTOTOOLTIPFILLER_H

// Qt include

#include <QString>

namespace ShowFoto
{

class ShowfotoItemInfo;

namespace ShowfotoToolTipFiller
{
    QString ShowfotoItemInfoTipContents(const ShowfotoItemInfo& info);

} // namespace ShowfotoToolTipFiller

} // namespace ShowFoto

#endif // SHOWFOTOTOOLTIPFILLER_H
