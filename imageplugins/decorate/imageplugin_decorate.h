/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a plugin to insert a text over an image.
 *
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPLUGIN_DECORATE_H
#define IMAGEPLUGIN_DECORATE_H

// Qt includes

#include <QVariant>

// Local includes

#include "imageplugin.h"
#include "digikam_export.h"

using namespace Digikam;

namespace DigikamDecorateImagePlugin
{

class ImagePlugin_Decorate : public ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_Decorate(QObject* const parent, const QVariantList& args);
    ~ImagePlugin_Decorate();

    void setEnabledActions(bool b);

private Q_SLOTS:

    void slotInsertText();
    void slotBorder();
    void slotTexture();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamDecorateImagePlugin

#endif /* IMAGEPLUGIN_DECORATE_H */
