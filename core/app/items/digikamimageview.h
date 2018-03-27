/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt item view for images
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_IMAGE_VIEW_H
#define DIGIKAM_IMAGE_VIEW_H

// Local includes

#include "imagecategorizedview.h"
#include "imageviewutilities.h"

namespace Digikam
{

class ImageViewUtilities;

class DigikamImageView : public ImageCategorizedView
{
    Q_OBJECT

public:

    explicit DigikamImageView(QWidget* const parent = 0);
    ~DigikamImageView();

    ImageViewUtilities* utilities() const;

    int  fitToWidthIcons();

    virtual void setThumbnailSize(const ThumbnailSize& size);

public Q_SLOTS:

    void openFile(const ImageInfo& info);

    void deleteSelected(const ImageViewUtilities::DeleteMode deleteMode = ImageViewUtilities::DeleteUseTrash);
    void deleteSelectedDirectly(const ImageViewUtilities::DeleteMode deleteMode = ImageViewUtilities::DeleteUseTrash);

    void rename();

    void assignRating(const QList<QModelIndex>& index, int rating);

    void setFaceMode(bool on);
    void confirmFaces(const QList<QModelIndex>& indexes, int tagId);
    void removeFaces(const QList<QModelIndex>& indexes);

Q_SIGNALS:

    void previewRequested(const ImageInfo& info);
    void fullscreenRequested(const ImageInfo& info);
    void signalShowContextMenu(QContextMenuEvent* event,
                               const QList<QAction*>& actions = QList<QAction*>());

    void signalShowContextMenuOnInfo(QContextMenuEvent* event, const ImageInfo& info,
                                     const QList<QAction*>& actions,
                                     ImageFilterModel* filterModel);

    void signalShowGroupContextMenu(QContextMenuEvent* event,
                                    const QList<ImageInfo>& selectedInfos,
                                    ImageFilterModel* filterModel);

protected Q_SLOTS:

    void groupIndicatorClicked(const QModelIndex& index);
    void showGroupContextMenu(const QModelIndex& index, QContextMenuEvent* event);

protected:

    void addRejectionOverlay(ImageDelegate* delegate = 0);
    void addAssignNameOverlay(ImageDelegate* delegate = 0);

    virtual void activated(const ImageInfo& info, Qt::KeyboardModifiers modifiers);
    virtual void showContextMenuOnInfo(QContextMenuEvent* event, const ImageInfo& info);
    virtual void showContextMenu(QContextMenuEvent* event);
    virtual void slotSetupChanged();

private Q_SLOTS:

    void slotRotateLeft(const QList<QModelIndex>&);
    void slotRotateRight(const QList<QModelIndex>&);
    void slotFullscreen(const QList<QModelIndex>&);
    void slotInitProgressIndicator();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IMAGE_VIEW_H
