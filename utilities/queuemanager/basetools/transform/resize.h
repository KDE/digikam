/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-17
 * Description : resize image batch tool.
 *
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RESIZE_H
#define RESIZE_H

// Local includes

#include "batchtool.h"

class QLabel;
class QCheckBox;

class KIntNumInput;
class KComboBox;

namespace Digikam
{

class Resize : public BatchTool
{
    Q_OBJECT

public:

    explicit Resize(QObject* parent = 0);
    ~Resize();

    BatchToolSettings defaultSettings();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    enum WidthPreset
    {
        Tiny = 0,
        Small,
        Medium,
        Big,
        Large,
        Huge
    };

    QLabel*       m_labelPreset;

    QCheckBox*    m_useCustom;

    KIntNumInput* m_customLength;

    KComboBox*    m_comboBox;

private:

    bool toolOperations();
    int  presetLengthValue(WidthPreset preset);
};

}  // namespace Digikam

#endif /* RESIZE_H */
