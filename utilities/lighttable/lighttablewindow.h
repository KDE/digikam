/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LIGHTTABLEWINDOW_H
#define LIGHTTABLEWINDOW_H

// Qt includes

#include <QString>
#include <QCloseEvent>

// KDE includes

#include <kurl.h>
#include <kxmlguiwindow.h>
#include <ktoggleaction.h>

// Local includes

#include "imageinfo.h"

class KAction;

namespace Digikam
{

class SlideShowSettings;

class LightTableWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:

    ~LightTableWindow();

    static LightTableWindow* lightTableWindow();
    static bool              lightTableWindowCreated();

    void loadImageInfos(const ImageInfoList& list, const ImageInfo& imageInfoCurrent, bool addTo);
    void setLeftRightItems(const ImageInfoList& list, bool addTo);
    void applySettings();
    void refreshView();
    bool isEmpty() const;

    void toggleTag(int tagID);

Q_SIGNALS:

    void signalFileDeleted(const KUrl&);
    void signalWindowHasMoved();

public Q_SLOTS:

    void slotFileChanged(const QString& filePath);
    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);
    void slotAssignRating(int rating);

protected:

    void moveEvent(QMoveEvent* e);

private:

    void closeEvent(QCloseEvent* e);
    void showEvent(QShowEvent*);
    void setupActions();
    void setupConnections();
    void setupUserArea();
    void setupStatusBar();
    void slideShow(SlideShowSettings& settings);
    void showToolBars();
    void hideToolBars();
    void readSettings();
    void writeSettings();
    void refreshStatusBar();

    void deleteItem(bool permanently);
    void deleteItem(const ImageInfo& info, bool permanently);

    LightTableWindow();

private Q_SLOTS:

    void slotBackward();
    void slotForward();
    void slotFirst();
    void slotLast();

    void slotSetItemLeft();
    void slotSetItemRight();
    void slotSetItemOnLeftPanel(const ImageInfo&);
    void slotSetItemOnRightPanel(const ImageInfo&);
    void slotLeftDroppedItems(const ImageInfoList&);
    void slotRightDroppedItems(const ImageInfoList&);

    void slotLeftPanelLeftButtonClicked();
    void slotRightPanelLeftButtonClicked();

    void slotLeftPreviewLoaded(bool);
    void slotRightPreviewLoaded(bool);

    void slotLeftZoomFactorChanged(double);
    void slotRightZoomFactorChanged(double);

    void slotToggleOnSyncPreview(bool);
    void slotToggleSyncPreview();
    void slotToggleNavigateByPair();

    void slotEditItem();
    void slotEditItem(const ImageInfo&);

    void slotDeleteItem();
    void slotDeleteItem(const ImageInfo&);

    void slotDeleteFinalItem();
    void slotDeleteFinalItem(const ImageInfo&);

    void slotRemoveItem();
    void slotRemoveItem(const ImageInfo&);

    void slotItemSelected(const ImageInfo&);
    void slotClearItemsList();

    void slotThumbbarDroppedItems(const ImageInfoList&);

    void slotProgressBarCancelButtonPressed();
    void slotToggleSlideShow();
    void slotToggleFullScreen();
    void slotEscapePressed();
    void slotEditKeys();
    void slotConfToolbars();
    void slotConfNotifications();
    void slotShowMenuBar();
    void slotNewToolbarConfig();
    void slotSetup();
    void slotComponentsInfo();
    void slotDBStat();

    void slotThemeChanged();
    void slotChangeTheme(const QString& theme);

    void slotSidebarTabTitleStyleChanged();

private:

    class LightTableWindowPriv;
    LightTableWindowPriv* const d;

    static LightTableWindow* m_instance;
};

}  // namespace Digikam

#endif /* LIGHTTABLEWINDOW_H */
