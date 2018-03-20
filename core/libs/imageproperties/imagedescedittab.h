/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Captions, Tags, and Rating properties editor
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGE_DESC_EDIT_TAB_H
#define IMAGE_DESC_EDIT_TAB_H

// Qt includes

#include <QScrollArea>
#include <QPixmap>
#include <QEvent>

// Local includes

#include "dlayoutbox.h"
#include "digikam_export.h"
#include "imageinfolist.h"
#include "albummanager.h"
#include "albummodel.h"
#include "metadatahub.h"
#include "searchtextbar.h"
#include "addtagslineedit.h"
#include "disjointmetadata.h"

namespace Digikam
{
class ImageInfo;
class TaggingAction;

class ImageDescEditTab : public DVBox
{
    Q_OBJECT

public:

    explicit ImageDescEditTab(QWidget* const parent);
    ~ImageDescEditTab();

    void assignPickLabel(int pickId);
    void assignColorLabel(int colorId);
    void assignRating(int rating);
    void setItem(const ImageInfo& info = ImageInfo());
    void setItems(const ImageInfoList& infos);
    void populateTags();
    void setFocusToTagsView();
    void setFocusToNewTagEdit();
    void setFocusToTitlesEdit();
    void setFocusToCommentsEdit();
    void activateAssignedTagsButton();

    AddTagsLineEdit* getNewTagEdit() const;

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalProgressMessageChanged(const QString& actionDescription);
    void signalProgressValueChanged(float percent);
    void signalProgressFinished();

    void signalTagFilterMatch(bool);
    void signalPrevItem();
    void signalNextItem();

protected:

    bool eventFilter(QObject* o, QEvent* e);

private:

    void reset();
    void setTagState(TAlbum* const tag, DisjointMetadata::Status status);

    void setInfos(const ImageInfoList& infos);
    void setFocusToLastSelectedWidget();

    void updateTagsView();
    void updateComments();
    void updatePickLabel();
    void updateColorLabel();
    void updateRating();
    void updateDate();
    void updateTemplate();
    void updateRecentTags();

    bool singleSelection() const;
    void setMetadataWidgetStatus(int status, QWidget* const widget);
    void metadataChange(qlonglong imageId);
    void resetMetadataChangeInfo();
    void initProgressIndicator();

Q_SIGNALS:

    void askToApplyChanges(const QList<ImageInfo>& infos, DisjointMetadata* hub);

private Q_SLOTS:

    void slotApplyAllChanges();
    void slotApplyChangesToAllVersions();
    void slotRevertAllChanges();
    void slotChangingItems();
    void slotTagsSearchChanged(const SearchTextSettings& settings);
    void slotTagStateChanged(Album* album, Qt::CheckState checkState);
    void slotOpenTagsManager();
    void slotCommentChanged();
    void slotTitleChanged();
    void slotDateTimeChanged(const QDateTime& dateTime);
    void slotPickLabelChanged(int pickId);
    void slotColorLabelChanged(int colorId);
    void slotRatingChanged(int rating);
    void slotTemplateSelected();
    void slotModified();
    void slotCreateNewTag();
    void slotTaggingActionActivated(const TaggingAction&);
    void slotReloadForMetadataChange();

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

    void slotAskToApplyChanges(const QList<ImageInfo>& infos, DisjointMetadata* hub);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif  // IMAGE_DESC_EDIT_TAB_H
