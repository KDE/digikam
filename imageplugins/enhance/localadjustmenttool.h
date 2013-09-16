/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a plugin to edit portions of a image/
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
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

#ifndef LOCALADJUSTMENTTOOL_H
#define LOCALADJUSTMENTTOOL_H

// Local includes

#include "editortool.h"
#include "localadjustmentfilter.h"

using namespace Digikam;

namespace DigikamEnhanceImagePlugin
{

class LocalAdjustmentTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit LocalAdjustmentTool(QObject* const parent);
    ~LocalAdjustmentTool();

private:

    void readSettings();
    void writeSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();

private Q_SLOTS:

    void slotResetSettings();
//    void slotLoadSettings();
//    void slotSaveAsSettings();

private:

    class Private;
    Private* const d;
};

}  // namespace DigikamEnhanceImagePlugin

#endif /* LOCALADJUSTMENTTOOL_H */

