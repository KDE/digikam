/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-08
 * Description : simple image properties side bar used by
 *               camera GUI.
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPROPERTIESSIDEBARCAMGUI_H
#define IMAGEPROPERTIESSIDEBARCAMGUI_H

// KDE includes

#include <kurl.h>

// Local includes

#include "sidebar.h"
#include "digikam_export.h"

class QSplitter;
class QWidget;

namespace Digikam
{

class SidebarSplitter;
class GPItemInfo;
class DMetadata;
class NavigateBarTab;

class ImagePropertiesSideBarCamGui : public Sidebar
{
    Q_OBJECT

public:

    ImagePropertiesSideBarCamGui(QWidget* parent, SidebarSplitter* splitter,
                                 KMultiTabBarPosition side=KMultiTabBar::Left, bool mimimizedDefault=false);

    ~ImagePropertiesSideBarCamGui();

    void applySettings();

    void itemChanged(const GPItemInfo& itemInfo, const DMetadata& meta);

public Q_SLOTS:

    virtual void slotNoCurrentItem();

Q_SIGNALS:

    void signalFirstItem();
    void signalPrevItem();
    void signalNextItem();
    void signalLastItem();

private Q_SLOTS:

    virtual void slotChangedTab(QWidget* tab);

private:

    class ImagePropertiesSideBarCamGuiPriv;
    ImagePropertiesSideBarCamGuiPriv* const d;
};

}  // namespace Digikam

#endif  // IMAGEPROPERTIESSIDEBARCAMGUI_H
