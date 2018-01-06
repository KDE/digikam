/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-03
 * Description : setup Image Editor interface.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPEDITORIFACE_H
#define SETUPEDITORIFACE_H

// Qt includes

#include <QScrollArea>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class SetupEditorIface : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupEditorIface(QWidget* const parent = 0);
    virtual ~SetupEditorIface();

    void applySettings();

private Q_SLOTS:

    void slotThemeBackgroundColor(bool);
    void slotExpoSettingsChanged();
    void slotShowOverExpoHistogramGuide(double);
    void slotShowUnderExpoHistogramGuide(double);

private:

    void readSettings();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SETUPEDITORIFACE_H
