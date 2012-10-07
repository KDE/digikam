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

#include <kdialog.h>

namespace Digikam
{

class ScriptIface : public KDialog
{
    Q_OBJECT

public:

    explicit ScriptIface(QWidget* parent = 0);
    ~ScriptIface();

private Q_SLOTS:

    void slotEvaluate();

private:

    void ImportQtBindings();

private:

    class ScriptIfacePriv;
    ScriptIfacePriv* const d;
};

} // namespace Digikam

#endif // SCRIPTIFACE_H
