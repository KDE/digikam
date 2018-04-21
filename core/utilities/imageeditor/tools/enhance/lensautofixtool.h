/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008      by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_LENS_AUTO_FIX_TOOL_H
#define DIGIKAM_HEALING_CLONE_TOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{

class LensAutoFixTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit LensAutoFixTool(QObject* const parent);
    ~LensAutoFixTool();

private Q_SLOTS:

    void slotLensChanged();
    void slotResetSettings();

private:

    void readSettings();
    void writeSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_HEALING_CLONE_TOOL_H
