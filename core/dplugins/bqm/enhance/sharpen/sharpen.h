/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-02
 * Description : sharpen image batch tool.
 *
 * Copyright (C) 2009      by Matthias Welwarsky <matze at welwarsky dot de>
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_BQM_SHARPEN_H
#define DIGIKAM_BQM_SHARPEN_H

#include "batchtool.h"
#include "sharpsettings.h"

using namespace Digikam;

namespace DigikamBqmSharpenPlugin
{

class Sharpen : public BatchTool
{
    Q_OBJECT

public:

    explicit Sharpen(QObject* const parent = nullptr);
    ~Sharpen();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=nullptr) const { return new Sharpen(parent); };

    void registerSettingsWidget();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    SharpSettings* m_settingsView;
};

} // namespace DigikamBqmSharpenPlugin

#endif // DIGIKAM_BQM_SHARPEN_H
