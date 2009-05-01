/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt item view for images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAMIMAGEVIEW_H
#define DIGIKAMIMAGEVIEW_H

// Qt includes

// KDE includes

// Local includes

#include "imagecategorizedview.h"

namespace Digikam
{

class DigikamImageViewPriv;

class DigikamImageView : public ImageCategorizedView
{
    Q_OBJECT

public:

    DigikamImageView(QWidget *parent = 0);
    ~DigikamImageView();

public Q_SLOTS:

    virtual void copy();
    virtual void paste();

    void openInEditor(const ImageInfo &info);

Q_SIGNALS:

    void previewRequested(const ImageInfo &info);

protected:

    virtual void activated(const ImageInfo& info);
    virtual void showContextMenu(QContextMenuEvent* event, const ImageInfo& info);
    virtual void showContextMenu(QContextMenuEvent* event);

private:

    DigikamImageViewPriv* const d;
};

} // namespace Digikam

#endif /* DIGIKAMIMAGEVIEW_H */
