/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image plugin to apply special effects.
 *
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPLUGIN_FXFILTERS_H
#define IMAGEPLUGIN_FXFILTERS_H

// Qt includes

#include <QVariant>

// Local includes

#include "imageplugin.h"
#include "digikam_export.h"

using namespace Digikam;

namespace DigikamFxFiltersImagePlugin
{

class ImagePlugin_FxFilters : public ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_FxFilters(QObject* const parent, const QVariantList& args);
    ~ImagePlugin_FxFilters();

    void setEnabledActions(bool b);

private Q_SLOTS:

    void slotColorEffects();
    void slotCharcoal();
    void slotEmboss();
    void slotOilPaint();
    void slotBlurFX();
    void slotDistortionFX();
    void slotRainDrop();
    void slotFilmGrain();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamFxFiltersImagePlugin

#endif /* IMAGEPLUGIN_FXFILTERS_H */
