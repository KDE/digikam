/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : a plugin to enhance image with local contrasts (as human eye does).
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at gmail dot com>
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

#ifndef IMAGEPLUGIN_LOCALCONTRAST_H
#define IMAGEPLUGIN_LOCALCONTRAST_H

// Qt includes

#include <QVariant>

// Local includes

#include "imageplugin.h"
#include "digikam_export.h"

class KAction;

class ImagePlugin_LocalContrast : public Digikam::ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_LocalContrast(QObject *parent, const QVariantList& args);
    ~ImagePlugin_LocalContrast();

    void setEnabledActions(bool enable);

private Q_SLOTS:

    void slotLocalContrast();

private:

    KAction *m_localContrastAction;
};

#endif /* IMAGEPLUGIN_LOCALCONTRAST_H */
