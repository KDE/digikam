/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a tool to insert a text over an image.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef INSERTTEXTTOOL_H
#define INSERTTEXTTOOL_H

// Local includes

#include "editortool.h"

class QFont;

namespace Digikam
{

class InsertTextTool : public EditorTool
{
    Q_OBJECT

public:

    explicit InsertTextTool(QObject* const parent);
    virtual ~InsertTextTool();

Q_SIGNALS:

    void signalUpdatePreview();

private Q_SLOTS:

    void slotResetSettings();
    void slotFontPropertiesChanged(const QFont& font);
    void slotUpdatePreview();
    void slotAlignModeChanged(int mode);

private:

    void readSettings();
    void writeSettings();
    void finalRendering();
    void setBackgroundColor(const QColor& bg);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // INSERTTEXTTOOL_H
