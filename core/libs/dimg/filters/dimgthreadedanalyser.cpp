/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : threaded image analys class.
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dimgthreadedanalyser.h"

namespace Digikam
{

DImgThreadedAnalyser::DImgThreadedAnalyser(QObject* const parent, const QString& name)
    : DImgThreadedFilter(parent, name)
{
}

DImgThreadedAnalyser::DImgThreadedAnalyser(DImg* const orgImage, QObject* const parent,
                                           const QString& name)
    : DImgThreadedFilter(orgImage, parent, name)
{
}

DImgThreadedAnalyser::~DImgThreadedAnalyser()
{
}

}  // namespace Digikam
