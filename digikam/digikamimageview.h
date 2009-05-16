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

class ImageViewUtilities;
class DigikamImageViewPriv;

class DigikamImageView : public ImageCategorizedView
{
    Q_OBJECT

public:

    DigikamImageView(QWidget *parent = 0);
    ~DigikamImageView();

    ImageViewUtilities *utilities() const;

public Q_SLOTS:

    virtual void copy();
    virtual void paste();

    void openInEditor(const ImageInfo& info);
    void openCurrentInEditor();

    void insertSelectedToLightTable(bool addTo);
    void insertSelectedToCurrentQueue();
    void insertSelectedToNewQueue();
    void insertSelectedToExistingQueue(int queueid);

    void deleteSelected(bool permanently = false);
    void deleteSelectedDirectly(bool permanently = false);
    void assignTagToSelected(int tagID);
    void removeTagFromSelected(int tagID);
    void assignRatingToSelected(int rating);
    void setAsAlbumThumbnail(const ImageInfo& setAsThumbnail);
    void createNewAlbumForSelected();
    void setExifOrientationOfSelected(int orientation);
    void renameCurrent();

Q_SIGNALS:

    void previewRequested(const ImageInfo& info);
    void gotoAlbumAndImageRequested(const ImageInfo& info);
    void gotoTagAndImageRequested(int tagId);
    void gotoDataAndImageRequested(const ImageInfo& info);

protected:

    virtual void activated(const ImageInfo& info);
    virtual void showContextMenu(QContextMenuEvent* event, const ImageInfo& info);
    virtual void showContextMenu(QContextMenuEvent* event);
    virtual void slotSetupChanged();

private:

    DigikamImageViewPriv* const d;
};

} // namespace Digikam

#endif /* DIGIKAMIMAGEVIEW_H */
