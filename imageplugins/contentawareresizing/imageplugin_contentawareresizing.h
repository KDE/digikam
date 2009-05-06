/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-01
 * Description : Plugin for Digikam using Liquid Rescale Library.
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at ulp dot u-strasbg dot fr>
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

#ifndef IMAGEPLUGIN_CONTENTAWARERESIZING_H
#define IMAGEPLUGIN_CONTENTAWARERESIZING_H

// Qt includes

#include <QVariant>

// Local includes

#include "imageplugin.h"

class KAction;

class ImagePlugin_ContentAwareResizing : public Digikam::ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_ContentAwareResizing(QObject *parent, const QVariantList &args);
    ~ImagePlugin_ContentAwareResizing();

    void setEnabledActions(bool enable);

private Q_SLOTS:

    void slotContentAwareResizing();

private:

    KAction *m_contentAwareResizingAction;
};

#endif/*IMAGEPLUGIN_CONTENTAWARERESIZING_H*/
