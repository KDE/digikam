/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : setup tab for slideshow options.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPSLIDESHOW_H
#define SETUPSLIDESHOW_H

// Qt includes

#include <QScrollArea>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class SetupSlideShow : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupSlideShow(QWidget* const parent = 0);
    ~SetupSlideShow();

    void applySettings();

public Q_SLOTS:

    void slotSetUnchecked(int);

private:

    void readSettings();

private:

    class Private;
    Private* const d;
};

}   // namespace Digikam

#endif /* SETUPSLIDESHOW_H */
