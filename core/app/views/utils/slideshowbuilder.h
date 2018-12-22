/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-24
 * Description : slideshow builder progress indicator
 *
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_SLIDE_SHOW_BUILDER_H
#define DIGIKAM_SLIDE_SHOW_BUILDER_H

// Local includes

#include "iteminfo.h"
#include "progressmanager.h"
#include "slideshowsettings.h"

namespace Digikam
{
class Album;

class SlideShowBuilder : public ProgressItem
{
    Q_OBJECT

public:

    /** Contructor to work on image list
     */
    explicit SlideShowBuilder(const ItemInfoList& infoList);

    /** Contructor to work on recursive mode from album
     */
    explicit SlideShowBuilder(Album* const album);

    ~SlideShowBuilder();

    /**
      * The builder creates the slideshow by reading the internal setup. These functions permit
      * to override the setup.
      *
      * @brief setOverrideStartFromCurrent
      * @param startFromCurrent
      */
    void setOverrideStartFrom(const ItemInfo& info);
    void setAutoPlayEnabled(bool enable);

    void run();

Q_SIGNALS:

    void signalComplete(const SlideShowSettings&);

private Q_SLOTS:

    void slotRun();
    void slotCancel();
    void slotParseItemInfoList(const ItemInfoList& list);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_SLIDE_SHOW_BUILDER_H
