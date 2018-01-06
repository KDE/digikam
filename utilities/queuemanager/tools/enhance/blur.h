/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-03
 * Description : blur image batch tool.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BLUR_H_
#define BLUR_H_

#include "batchtool.h"
#include "dnuminput.h"

namespace Digikam
{

class Blur : public BatchTool
{
    Q_OBJECT

public:

    explicit Blur(QObject* const parent = 0);
    ~Blur();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new Blur(parent); };

    void registerSettingsWidget();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    DIntNumInput* m_radiusInput;
    bool          m_changeSettings;
};

} // namespace Digikam

#endif /* BLUR_H_ */
