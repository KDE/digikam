/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-08
 * Description : simple image properties side bar used by
 *               camera GUI.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef IMAGE_PROPERTIES_SIDEBAR_CAMGUI_H
#define IMAGE_PROPERTIES_SIDEBAR_CAMGUI_H

// Qt includes

#include <QUrl>
#include <QWidget>

// Local includes

#include "sidebar.h"
#include "digikam_export.h"

namespace Digikam
{

class SidebarSplitter;
class CamItemInfo;
class DMetadata;

class DIGIKAM_EXPORT ImagePropertiesSideBarCamGui : public Sidebar
{
    Q_OBJECT

public:

    explicit ImagePropertiesSideBarCamGui(QWidget* const parent,
                                          SidebarSplitter* const splitter,
                                          Qt::Edge side=Qt::LeftEdge,
                                          bool mimimizedDefault=false);
    ~ImagePropertiesSideBarCamGui();

    void applySettings();

    void itemChanged(const CamItemInfo& itemInfo, const DMetadata& meta);

    QUrl url() const;

public Q_SLOTS:

    virtual void slotNoCurrentItem();

Q_SIGNALS:

    void signalFirstItem();
    void signalPrevItem();
    void signalNextItem();
    void signalLastItem();

protected:

    /**
     * load the last view state from disk - called by StateSavingObject#loadState()
     */
    void doLoadState();

    /**
     * save the view state to disk - called by StateSavingObject#saveState()
     */
    void doSaveState();

private Q_SLOTS:

    virtual void slotChangedTab(QWidget* tab);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMAGE_PROPERTIES_SIDEBAR_CAMGUI_H
