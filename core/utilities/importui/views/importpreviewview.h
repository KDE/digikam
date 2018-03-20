/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-14-07
 * Description : An embedded view to show the cam item preview widget.
 *
 * Copyright (C) 2012 by Islam Wazery  <wazery at ubuntu dot com>
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

#ifndef IMPORTPREVIEWVIEW_H
#define IMPORTPREVIEWVIEW_H

// Local includes

#include "graphicsdimgview.h"
#include "camiteminfo.h"

namespace Digikam
{

class ImportPreviewView : public GraphicsDImgView
{
    Q_OBJECT

public:

    enum Mode
    {
        IconViewPreview
    };

public:

    explicit ImportPreviewView(QWidget* const parent, Mode mode = IconViewPreview);
    ~ImportPreviewView();

    void setCamItemInfo(const CamItemInfo& info   = CamItemInfo(),
                        const CamItemInfo& previous = CamItemInfo(),
                        const CamItemInfo& next     = CamItemInfo());

    CamItemInfo getCamItemInfo() const;

    void reload();
    void setCamItemPath(const QString& path = QString());
    void setPreviousNextPaths(const QString& previous, const QString& next);

    void showContextMenu(const CamItemInfo& info, QGraphicsSceneContextMenuEvent* event);

private:

    QString identifyCategoryforMime(QString mime);

Q_SIGNALS:

    void signalNextItem();
    void signalPrevItem();
    void signalDeleteItem();
    void signalEditItem();
    void signalPreviewLoaded(bool success);
    void signalEscapePreview();
    //void signalSlideShow();
    //void signalInsert2LightTable();
    //void signalInsert2QueueMgr();
    //void signalFindSimilar();
    //void signalAddToExistingQueue(int);

    //void signalGotoFolderAndItem(const CamItemInfo&);
    //void signalGotoDateAndItem(const CamItemInfo&);
    //void signalGotoTagAndItem(int);
    //void signalPopupTagsView();
    void signalAssignPickLabel(int);
    void signalAssignColorLabel(int);
    void signalAssignRating(int);


protected:

    bool acceptsMouseClick(QMouseEvent* e);
    void enterEvent(QEvent* e);
    void leaveEvent(QEvent* e);
    void showEvent(QShowEvent* e);

private Q_SLOTS:

    void camItemLoaded();
    void camItemLoadingFailed();

    //TODO: Implement Tags and Labels in Import Tool
    //void slotAssignTag(int tagID);
    //void slotRemoveTag(int tagID);
    //void slotAssignPickLabel(int pickId);
    //void slotAssignColorLabel(int colorId);

    void slotThemeChanged();
    void slotSetupChanged();

    void slotRotateLeft();
    void slotRotateRight();
    void slotDeleteItem();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMPORTPREVIEWVIEW_H
