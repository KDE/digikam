/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tools Manager.
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "batchtoolsmanager.moc"

// Local includes

#include <config-digikam.h>
#include "assigntemplate.h"
#include "autocorrection.h"
#include "convert2jp2.h"
#include "convert2jpeg.h"
#include "convert2pgf.h"
#include "convert2png.h"
#include "convert2tiff.h"
#include "flip.h"
#include "resize.h"
#include "restoration.h"
#include "rotate.h"
#include "sharpen.h"
#include "blur.h"
#include "watermark.h"
#include "noisereduction.h"
#include "bcgcorrection.h"
#include "hslcorrection.h"
#include "colorbalance.h"
#include "iccconvert.h"
#include "channelmixer.h"
#include "bwconvert.h"
#include "whitebalance.h"
#include "curvesadjust.h"
#include "filmgrain.h"
#include "localcontrast.h"
#include "antivignetting.h"
#include "invert.h"
#include "convert8to16.h"
#include "convert16to8.h"
#include "border.h"
#include "removemetadata.h"
#include "lensautofix.h"

namespace Digikam
{

class BatchToolsManager::BatchToolsManagerPriv
{

public:

    BatchToolsManagerPriv() {}

    BatchToolsList toolsList;
};

BatchToolsManager::BatchToolsManager(QObject* parent)
    : QObject(parent), d(new BatchToolsManagerPriv)
{
    // Convert
    registerTool(new Convert2JPEG(this));
    registerTool(new Convert2PNG(this));
    registerTool(new Convert2TIFF(this));
    registerTool(new Convert2JP2(this));
    registerTool(new Convert2PGF(this));

    // Transform
    registerTool(new Rotate(this));
    registerTool(new Flip(this));
    registerTool(new Resize(this));

    // Decorate
    registerTool(new WaterMark(this));
    registerTool(new Border(this));

    // Metadata
    registerTool(new AssignTemplate(this));
    registerTool(new RemoveMetadata(this));

    // Enhance
    registerTool(new Blur(this));
    registerTool(new Sharpen(this));
    registerTool(new NoiseReduction(this));
    registerTool(new Restoration(this));
    registerTool(new LocalContrast(this));
    registerTool(new AntiVignetting(this));
#ifdef HAVE_GLIB2
    registerTool(new LensAutoFix(this));
#endif // HAVE_GLIB2

    // Color
    registerTool(new BCGCorrection(this));
    registerTool(new HSLCorrection(this));
    registerTool(new ColorBalance(this));
    registerTool(new AutoCorrection(this));
    registerTool(new IccConvert(this));
    registerTool(new ChannelMixer(this));
    registerTool(new BWConvert(this));
    registerTool(new WhiteBalance(this));
    registerTool(new CurvesAdjust(this));
    registerTool(new Invert(this));
    registerTool(new Convert8to16(this));
    registerTool(new Convert16to8(this));

    // Filters
    registerTool(new FilmGrain(this));
}

BatchToolsManager::~BatchToolsManager()
{
    delete d;
}

BatchToolsList BatchToolsManager::toolsList() const
{
    return d->toolsList;
}

void BatchToolsManager::registerTool(BatchTool* tool)
{
    if (!tool)
    {
        return;
    }

    d->toolsList.append(tool);
}

void BatchToolsManager::unregisterTool(BatchTool* tool)
{
    if (!tool)
    {
        return;
    }

    for (BatchToolsList::iterator it = d->toolsList.begin();
         it != d->toolsList.end(); )
    {
        if (*it == tool)
        {
            delete *it;
            it = d->toolsList.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

BatchTool* BatchToolsManager::findTool(const QString& name, BatchTool::BatchToolGroup group) const
{
    for (BatchToolsList::const_iterator it = d->toolsList.constBegin(); it != d->toolsList.constEnd(); ++it)
    {
        if ((*it)->objectName() == name && (*it)->toolGroup() == group)
        {
            return *it;
        }
    }

    return 0;
}

}  // namespace Digikam
