/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Captions, Tags, and Rating properties editor
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEDESCEDITTAB_H
#define IMAGEDESCEDITTAB_H

// Qt includes

#include <QScrollArea>
#include <QPixmap>
#include <QEvent>

// KDE includes

#include <kvbox.h>

// Local includes

#include "digikam_export.h"
#include "searchtextbar.h"
#include "imageinfolist.h"
#include "albummanager.h"
#include "albummodel.h"
#include "metadatahub.h"

namespace Digikam
{
class ImageInfo;
class ImageDescEditTabPriv;

class ImageDescEditTab : public KVBox
{
    Q_OBJECT

public:

    ImageDescEditTab(QWidget *parent);
    ~ImageDescEditTab();

    void assignRating(int rating);
    void setItem(const ImageInfo& info = ImageInfo());
    void setItems(const ImageInfoList& infos);
    void populateTags();

Q_SIGNALS:

    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);
    void signalTagFilterMatch(bool);
    void signalPrevItem();
    void signalNextItem();

protected:

    bool eventFilter(QObject *o, QEvent *e);

private:

    void initializeTags(QModelIndex &parent);
    void setTagState(TAlbum *tag, MetadataHub::TagStatus status);

    void setInfos(const ImageInfoList& infos);
    void focusLastSelectedWidget();

    void updateTagsView();
    void updateComments();
    void updateRating();
    void updateDate();
    void updateTemplate();
    void updateRecentTags();

    bool singleSelection() const;
    void setMetadataWidgetStatus(int status, QWidget *widget);
    void reloadForMetadataChange(qlonglong imageId);

private Q_SLOTS:

    void slotApplyAllChanges();
    void slotRevertAllChanges();
    void slotChangingItems();
    void slotTagsSearchChanged(const SearchTextSettings& settings);
    void slotTagStateChanged(Album *album, Qt::CheckState checkState);
    void slotCommentChanged();
    void slotDateTimeChanged(const QDateTime& dateTime);
    void slotRatingChanged(int rating);
    void slotTemplateSelected();
    void slotModified();
    void slotCreateNewTag();

    void slotImageTagsChanged(qlonglong imageId);
    void slotImagesChanged(int albumId);
    void slotImageRatingChanged(qlonglong imageId);
    void slotImageDateChanged(qlonglong imageId);
    void slotImageCaptionChanged(qlonglong imageId);

    void slotRecentTagsMenuActivated(int);
    void slotAssignedTagsToggled(bool);

    void slotMoreMenu();
    void slotReadFromFileMetadataToDatabase();
    void slotWriteToFileMetadataFromDatabase();

private:

    ImageDescEditTabPriv* const d;
};

}  // namespace Digikam

#endif  // IMAGEDESCEDITTAB_H
