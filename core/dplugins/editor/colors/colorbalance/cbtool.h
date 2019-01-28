/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-11
 * Description : digiKam image editor Color Balance tool.
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

#ifndef DIGIKAM_EDITOR_CB_TOOL_H
#define DIGIKAM_EDITOR_CB_TOOL_H

#include <QObject>

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorColorBalanceToolPlugin
{

class CBTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit CBTool(QObject* const parent);
    ~CBTool();

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

} // namespace DigikamEditorColorBalanceToolPlugin

#endif // DIGIKAM_EDITOR_CB_TOOL_H
