/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qstring.h>

// KDE includes.

#include <kurl.h>
#include <kmainwindow.h>

// Local includes.

#include "imageinfo.h"

class KAction;

namespace Digikam
{

class SlideShowSettings;
class ThumbBarItem;
class LightTableWindowPriv;

class LightTableWindow : public KMainWindow
{
    Q_OBJECT

public:

    ~LightTableWindow();

    static LightTableWindow *lightTableWindow();
    static bool              lightTableWindowCreated();

    void loadImageInfos(const ImageInfoList &list, ImageInfo *imageInfoCurrent, bool addTo);
    void setLeftRightItems(const ImageInfoList &list, bool addTo);
    void applySettings();
    void refreshView();

signals: 

    void signalFileDeleted(const KURL&);

public slots:

    void slotItemsUpdated(const KURL::List&);

private:

    void closeEvent(QCloseEvent* e);
    void setupActions();
    void setupConnections();
    void setupUserArea();
    void setupStatusBar();
    void setupAccelerators();
    void slideShow(bool startWithCurrent, SlideShowSettings& settings);
    void showToolBars();
    void hideToolBars();
    void plugActionAccel(KAction* action);
    void unplugActionAccel(KAction* action);
    void readSettings();
    void writeSettings();
    void refreshStatusBar();

    LightTableWindow();

private slots:

    void slotBackward();
    void slotForward();
    void slotFirst();
    void slotLast();

    void slotSetItemLeft();
    void slotSetItemRight();
    void slotSetItemOnLeftPanel(ImageInfo*);
    void slotSetItemOnRightPanel(ImageInfo*);
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
    void slotEditItem(ImageInfo*);

    void slotDeleteItem();
    void slotDeleteItem(ImageInfo*);

    void slotRemoveItem();
    void slotRemoveItem(ImageInfo*);

    void slotItemSelected(ImageInfo*);
    void slotClearItemsList();

    void slotThumbbarDroppedItems(const ImageInfoList&);

    void slotZoomTo100Percents();
    void slotFitToWindow();

    void slotProgressBarCancelButtonPressed();
    void slotToggleSlideShow();
    void slotToggleFullScreen();
    void slotEscapePressed();
    void slotDonateMoney();
    void slotContribute();
    void slotRawCameraList();
    void slotEditKeys();
    void slotConfToolbars();
    void slotShowMenuBar();
    void slotNewToolbarConfig();
    void slotSetup();

    void slotThemeChanged();
    void slotChangeTheme(const QString& theme);

private:

    LightTableWindowPriv    *d;

    static LightTableWindow *m_instance;
};

}  // namespace Digikam

#endif /* LIGHTTABLEWINDOW_H */
