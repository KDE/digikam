/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-11
 * Description : script interface for digiKam
 *
 * Copyright (C) 2010 by Kunal Ghosh <kunal dot t2 at gmail dot com>
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SCRIPTIFACE_H
#define SCRIPTIFACE_H

// KDE includes

#include <KDialog>

// Local includes

#include "digikam_export.h"

namespace Ui
{
    class scriptiface;
}

namespace Digikam
{

class DIGIKAM_EXPORT scriptiface : public KDialog
{
    Q_OBJECT

public:

    scriptiface(); //QWidget *parent = 0);
    ~scriptiface();

protected:

    void changeEvent(QEvent* e);

private:

    Ui::scriptiface* ui;
};

} // namespace Digikam

#endif // SCRIPTIFACE_H
