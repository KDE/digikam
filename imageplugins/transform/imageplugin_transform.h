/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a plugin to transform image geometry.
 *
 * Copyright (C) 2005-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPLUGIN_TRANSFORM_H
#define IMAGEPLUGIN_TRANSFORM_H

// Qt includes

#include <QVariant>

// Local includes

#include "imageplugin.h"
#include "digikam_export.h"

using namespace Digikam;

namespace DigikamTransformImagePlugin
{

class ImagePlugin_Transform : public ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_Transform(QObject* const parent, const QVariantList& args);
    ~ImagePlugin_Transform();

    void setEnabledActions(bool b);

Q_SIGNALS:

    void signalPoint1Action();
    void signalPoint2Action();
    void signalAutoAdjustAction();

private Q_SLOTS:

    void slotPerspective();
    void slotFreeRotation();
    void slotShearTool();
    void slotContentAwareResizing();
    void slotResize();
    void slotRatioCrop();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamTransformImagePlugin

#endif /* IMAGEPLUGIN_TRANSFORM_H */
