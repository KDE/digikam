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

#ifndef IMAGEHISTORYVIEW_H
#define IMAGEHISTORYVIEW_H

// Qt includes

#include <QTreeView>

namespace Digikam
{

class ImageHistoryView : QTreeView
{
    Q_OBJECT

public:

    explicit ImageHistoryView(QWidget* parent);
    ~ImageHistoryView();

private:

    class ImageHistoryViewPriv;
    ImageHistoryViewPriv* const d;
};

} // namespace Digikam

#endif // IMAGEHISTORYVIEW_H
