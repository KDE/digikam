/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

// Qt includes.

#include <qstring.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "editorwindow.h"
#include "imageinfo.h"

class QDragMoveEvent;
class QDropEvent;

namespace Digikam
{

class AlbumIconView;
class ImageWindowPriv;
class SlideShowSettings;

class ImageWindow : public EditorWindow
{
    Q_OBJECT

public:

    ~ImageWindow();

    void loadURL(const KURL::List& urlList, const KURL& urlCurrent,
                 const QString& caption=QString(),
                 bool allowSaving=true);

    void loadImageInfos(const ImageInfoList &imageInfoList,
                        ImageInfo *imageInfoCurrent,
                        const QString& caption, bool allowSaving);

    static ImageWindow* imagewindow();
    static bool         imagewindowCreated();

    void applySettings();
    void refreshView();
    bool setup(bool iccSetupPage=false);

    bool queryClose();

signals:

    void signalFileDeleted(const KURL& url);
    void signalFileAdded(const KURL& url);
    void signalFileModified(const KURL& url);
    void signalURLChanged(const KURL& url);

private:

    void loadCurrentList(const QString& caption, bool allowSaving);
    void closeEvent(QCloseEvent* e);

    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);

    void setupActions();
    void setupConnections();
    void setupUserArea();
    void toggleGUI2FullScreen();

    bool save();
    bool saveAs();

    void saveIsComplete();
    void saveAsIsComplete();
    void setViewToURL(const KURL &url);
    void deleteCurrentItem(bool ask, bool permanently);

    void slideShow(bool startWithCurrent, SlideShowSettings& settings);

    Sidebar* rightSideBar() const;

    ImageWindow();

private slots:

    void slotForward();
    void slotBackward();
    void slotFirst();
    void slotLast();
    void slotFilePrint();

    void slotLoadCurrent();
    void slotDeleteCurrentItem();
    void slotDeleteCurrentItemPermanently();
    void slotDeleteCurrentItemPermanentlyDirectly();
    void slotTrashCurrentItemDirectly();

    void slotChanged();
    void slotUndoStateChanged(bool, bool, bool);
    void slotUpdateItemInfo();

    void slotContextMenu();
    void slotRevert();

    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);

    void slotAssignRatingNoStar();
    void slotAssignRatingOneStar();
    void slotAssignRatingTwoStar();
    void slotAssignRatingThreeStar();
    void slotAssignRatingFourStar();
    void slotAssignRatingFiveStar();
    void slotAssignRating(int rating);

    void slotFileMetadataChanged(const KURL &);
    void slotChangeTheme(const QString& theme);

private:

    ImageWindowPriv    *d;

    static ImageWindow *m_instance;
};

}  // namespace Digikam

#endif /* IMAGEWINDOW_H */
