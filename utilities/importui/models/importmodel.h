/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-18-06
 * Description : Camera model for the import interface
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#ifndef IMPORTMODEL_H
#define IMPORTMODEL_H

// Local includes

#include "importthumbnailmodel.h"

namespace Digikam
{

class ImportModel : public ImportThumbnailModel
{
    Q_OBJECT

public:

    ImportModel(QObject* const parent = 0);
    ~ImportModel();

    void setupCameraController(CameraController* const controller);
};

} // namespace Digikam

#endif // IMPORTMODEL_H
