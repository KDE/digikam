/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-02
 * Description : sharpen image batch tool.
 *
 * Copyright (C) 2009      by Matthias Welwarsky <matze at welwarsky dot de>
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

#ifndef SHARPEN_H_
#define SHARPEN_H_

#include "batchtool.h"

namespace Digikam
{

class SharpSettings;

class Sharpen : public BatchTool
{
    Q_OBJECT

public:

    explicit Sharpen(QObject* const parent = 0);
    virtual ~Sharpen();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new Sharpen(parent); };

    void registerSettingsWidget();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    SharpSettings* m_settingsView;
};

} // namespace Digikam

#endif // SHARPEN_H_
