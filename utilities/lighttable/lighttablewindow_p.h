/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LIGHTTABLEWINDOWPRIVATE_H
#define LIGHTTABLEWINDOWPRIVATE_H

// Qt includes

#include <QSplitter>

// KDE includes

#include <kaction.h>
#include <kselectaction.h>
#include <ksqueezedtextlabel.h>

// Local includes

#include "globals.h"
#include "imagepropertiessidebardb.h"
#include "statusprogressbar.h"
#include "dzoombar.h"
#include "lighttableview.h"
#include "lighttablethumbbar.h"
#include "thumbbardock.h"

namespace Digikam
{

class LightTableWindow::Private
{

public:

    Private() :
        autoLoadOnRightPanel(true),
        autoSyncPreview(true),
        cancelSlideShow(false),
        setItemLeftAction(0),
        setItemRightAction(0),
        clearListAction(0),
        editItemAction(0),
        removeItemAction(0),
        fileDeleteAction(0),
        fileDeleteFinalAction(0),
        slideShowAction(0),
        leftZoomPlusAction(0),
        leftZoomMinusAction(0),
        leftZoomTo100percents(0),
        leftZoomFitToWindowAction(0),
        rightZoomPlusAction(0),
        rightZoomMinusAction(0),
        rightZoomTo100percents(0),
        rightZoomFitToWindowAction(0),
        forwardAction(0),
        backwardAction(0),
        firstAction(0),
        lastAction(0),
        showBarAction(0),
        viewCMViewAction(0),
        syncPreviewAction(0),
        navigateByPairAction(0),
        showMenuBarAction(0),
        clearOnCloseAction(0),
        leftFileName(0),
        rightFileName(0),
        hSplitter(0),
        barViewDock(0),
        thumbView(0),
        previewView(0),
        leftZoomBar(0),
        rightZoomBar(0),
        statusProgressBar(0),
        leftSideBar(0),
        rightSideBar(0)
    {
    }

    void addPageUpDownActions(LightTableWindow* const q, QWidget* const w);

public:

    bool                      autoLoadOnRightPanel;
    bool                      autoSyncPreview;
    bool                      cancelSlideShow;

    KAction*                  setItemLeftAction;
    KAction*                  setItemRightAction;
    KAction*                  clearListAction;
    KAction*                  editItemAction;
    KAction*                  removeItemAction;
    KAction*                  fileDeleteAction;
    KAction*                  fileDeleteFinalAction;
    KAction*                  slideShowAction;
    KAction*                  leftZoomPlusAction;
    KAction*                  leftZoomMinusAction;
    KAction*                  leftZoomTo100percents;
    KAction*                  leftZoomFitToWindowAction;
    KAction*                  rightZoomPlusAction;
    KAction*                  rightZoomMinusAction;
    KAction*                  rightZoomTo100percents;
    KAction*                  rightZoomFitToWindowAction;

    KAction*                  forwardAction;
    KAction*                  backwardAction;
    KAction*                  firstAction;
    KAction*                  lastAction;

    KToggleAction*            showBarAction;
    KToggleAction*            viewCMViewAction;
    KToggleAction*            syncPreviewAction;
    KToggleAction*            navigateByPairAction;
    KToggleAction*            showMenuBarAction;
    KToggleAction*            clearOnCloseAction;

    KSqueezedTextLabel*       leftFileName;
    KSqueezedTextLabel*       rightFileName;

    SidebarSplitter*          hSplitter;
    ThumbBarDock*             barViewDock;

    LightTableThumbBar*       thumbView;

    LightTableView*           previewView;

    DZoomBar*                 leftZoomBar;
    DZoomBar*                 rightZoomBar;

    StatusProgressBar*        statusProgressBar;

    ImagePropertiesSideBarDB* leftSideBar;
    ImagePropertiesSideBarDB* rightSideBar;
};

void LightTableWindow::Private::addPageUpDownActions(LightTableWindow* const q, QWidget* const w)
{
    defineShortcut(w, Qt::Key_Down,  q, SLOT(slotForward()));
    defineShortcut(w, Qt::Key_Right, q, SLOT(slotForward()));
    defineShortcut(w, Qt::Key_Up,    q, SLOT(slotBackward()));
    defineShortcut(w, Qt::Key_Left,  q, SLOT(slotBackward()));
}

}  // namespace Digikam

#endif /* LIGHTTABLEWINDOWPRIVATE_H */
