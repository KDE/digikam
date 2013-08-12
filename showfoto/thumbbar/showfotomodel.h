/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 08-08-2013
 * Description : Qt model for Showfoto entries
 *
 * Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#ifndef SHOWFOTOMODEL_H
#define SHOWFOTOMODEL_H

#include "showfotothumbnailmodel.h"

namespace ShowFoto {


class ShowfotoModel : public ShowfotoThumbnailModel
{
    Q_OBJECT

public:
    explicit ShowfotoModel(QObject* const parent = 0);
    ~ShowfotoModel();

    void setThumbnailLoadThread(ThumbnailLoadThread *thread);
};
}
#endif // SHOWFOTOMODEL_H
