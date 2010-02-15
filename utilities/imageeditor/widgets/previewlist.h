/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-13
 * Description : a list of selectable options with preview
 *               effects as thumbnails.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PREVIEWLIST_H
#define PREVIEWLIST_H

// Qt includes

#include <QtGui/QListWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImgThreadedFilter;
class PreviewListItemPriv;

class DIGIKAM_EXPORT PreviewListItem : public QListWidgetItem
{

public:

    PreviewListItem(QListWidget* parent=0);
    ~PreviewListItem();

    void setPreviewFilter(DImgThreadedFilter* filter);

private:

    PreviewListItemPriv* const d;
};

// -------------------------------------------------------------------

class PreviewListPriv;

class DIGIKAM_EXPORT PreviewList : public QListWidget
{
    Q_OBJECT

public:

    PreviewList(QWidget* parent=0);
    ~PreviewList();

private:

    PreviewListPriv* const d;
};

} // namespace Digikam

#endif /* PREVIEWLIST_H */
