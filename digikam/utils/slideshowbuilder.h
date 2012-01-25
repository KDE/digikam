/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-24
 * Description : slideshow builder progress indicator
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SLIDESHOWBUILDER_H
#define SLIDESHOWBUILDER_H

// Qt includes

#include <QString>

// Local includes

#include "imageinfo.h"
#include "progressmanager.h"
#include "slideshowsettings.h"

namespace Digikam
{

class SlideShowBuilder : public ProgressItem
{
    Q_OBJECT

public:

    SlideShowBuilder(const ImageInfoList& infoList);
    ~SlideShowBuilder();

Q_SIGNALS:

    void signalComplete(const SlideShowSettings&);

private Q_SLOTS:

    void slotRun();
    void slotCancel();

private:

    bool          m_cancel;
    ImageInfoList m_infoList;
};

} // namespace Digikam

#endif /* SLIDESHOWBUILDER_H */
