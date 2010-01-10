/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-10
 * Description : a tool bar for preview mode
 *
 * Copyright (C) 2010 by Gilles Caulier<caulier dot gilles at gmail dot com>
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

#ifndef PREVIEWTOOLBAR_H
#define PREVIEWTOOLBAR_H

// KDE includes

#include <QtGui/QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class PreviewToolBarPriv;

class DIGIKAM_EXPORT PreviewToolBar : public QWidget
{
    Q_OBJECT

public:

    enum PreviewMode
    {
        PreviewOriginalImage      = 0x00000001,      // Original image only.
        PreviewBothImagesHorz     = 0x00000002,      // Horizontal with original and target duplicated.
        PreviewBothImagesVert     = 0x00000004,      // Vertical with original and target duplicated.
        PreviewBothImagesHorzCont = 0x00000008,      // Horizontal with original and target in contiguous.
        PreviewBothImagesVertCont = 0x00000010,      // Vertical with original and target in contiguous.
        PreviewTargetImage        = 0x00000020,      // Target image only.
        PreviewToggleOnMouseOver  = 0x00000040,      // Original image if mouse is over image area, else target image.
        NoPreviewMode             = 0x00000080,      // Target image only without information displayed.
    };

public:

    PreviewToolBar(QWidget* parent=0);
    ~PreviewToolBar();

Q_SIGNALS:

    void signalPreviewModeChanged(int);

private:

    PreviewToolBarPriv* const d;
};

} // namespace Digikam

#endif /* PREVIEWTOOLBAR_H */
