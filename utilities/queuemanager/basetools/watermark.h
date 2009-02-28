/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-28
 * Description : batch tool to add visible watermark.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WATERMARK_H
#define WATERMARK_H

// Local includes.

#include "batchtool.h"

class KComboBox;
class KLineEdit;
class KFontChooser;
class KColorButton;

namespace Digikam
{

class WaterMark : public BatchTool
{
    Q_OBJECT

public:

    WaterMark(QObject* parent=0);
    ~WaterMark();

    BatchToolSettings defaultSettings();

private:

    void assignSettings2Widget();
    bool toolOperations();

private Q_SLOTS:

    void slotSettingsChanged();

private:

    enum AutoCorrectionType
    {
        TopLeft=0,
        TopRight,
        BottomLeft,
        BottomRight
    };

    KLineEdit    *m_textEdit;

    KFontChooser *m_fontChooserWidget;

    KColorButton *m_fontColorButton;

    KComboBox    *m_comboBox;
};

}  // namespace Digikam

#endif /* AUTO_CORRECTION_H */
