/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-23
 * Description : view for displaying all other versions of current image
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "imagehistoryview.h"

namespace Digikam
{

class ImageHistoryView::ImageHistoryViewPriv
{
};

ImageHistoryView::ImageHistoryView(QWidget* parent)
    : QTreeView(parent)
{
}

ImageHistoryView::~ImageHistoryView()
{
}

} // namespace Digikam
