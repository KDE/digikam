/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digikam image editor tool to
 *               simulate charcoal drawing.
 *
 * Copyright (C) 2004-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_EDITOR_CHARCOAL_TOOL_H
#define DIGIKAM_EDITOR_CHARCOAL_TOOL_H

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace EditorDigikamCharcoalToolPlugin
{

class CharcoalTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit CharcoalTool(QObject* const parent);
    ~CharcoalTool();

private Q_SLOTS:

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

} // namespace EditorDigikamCharcoalToolPlugin

#endif // DIGIKAM_EDITOR_CHARCOAL_TOOL_H
