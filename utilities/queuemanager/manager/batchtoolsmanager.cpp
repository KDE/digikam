/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tools Manager.
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "config-digikam.h"
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
#include "colorfx.h"
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

class BatchToolsManager::Private
{
public:

    /** A container of associated batch tool and settings widget.
     */
    class BatchToolWidgetSet
    {
    public:

        BatchToolWidgetSet()
        {
            tool           = 0;
            settingsWidget = 0;
        };

        BatchTool* tool;
        QWidget*   settingsWidget;
    };

    typedef QList<BatchToolWidgetSet> BatchToolsWidgetList;

public:

    Private()
    {
    }
    
    BatchToolsWidgetList toolsList;
};

BatchToolsManager::BatchToolsManager(QObject* const parent)
    : QObject(parent), d(new Private)
{
    // Convert
    registerTool(new Convert2JPEG(this), 0);
    registerTool(new Convert2PNG(this),  0);
    registerTool(new Convert2TIFF(this), 0);
    registerTool(new Convert2JP2(this),  0);
    registerTool(new Convert2PGF(this),  0);

    // Transform
    registerTool(new Rotate(this), 0);
    registerTool(new Flip(this),   0);
    registerTool(new Resize(this), 0);

    // Decorate
    registerTool(new WaterMark(this), 0);
    registerTool(new Border(this),    0);

    // Metadata
    registerTool(new AssignTemplate(this), 0);
    registerTool(new RemoveMetadata(this), 0);

    // Enhance
    registerTool(new Blur(this),           0);
    registerTool(new Sharpen(this),        0);
    registerTool(new NoiseReduction(this), 0);
    registerTool(new Restoration(this),    0);
    registerTool(new LocalContrast(this),  0);
    registerTool(new AntiVignetting(this), 0);
#ifdef HAVE_GLIB2
    registerTool(new LensAutoFix(this),    0);
#endif // HAVE_GLIB2

    // Color
    registerTool(new BCGCorrection(this),  0);
    registerTool(new HSLCorrection(this),  0);
    registerTool(new ColorBalance(this),   0);
    registerTool(new AutoCorrection(this), 0);
    registerTool(new IccConvert(this),     0);
    registerTool(new ChannelMixer(this),   0);
    registerTool(new BWConvert(this),      0);
    registerTool(new WhiteBalance(this),   0);
    registerTool(new CurvesAdjust(this),   0);
    registerTool(new Invert(this),         0);
    registerTool(new Convert8to16(this),   0);
    registerTool(new Convert16to8(this),   0);

    // Filters
    registerTool(new FilmGrain(this), 0);
    registerTool(new ColorFX(this),   0);
}

BatchToolsManager::~BatchToolsManager()
{
    delete d;
}

BatchToolsList BatchToolsManager::toolsList() const
{
    BatchToolsList list;

    foreach(Private::BatchToolWidgetSet set, d->toolsList)
    {
        list.append(set.tool);
    }
    
    return list;
}

void BatchToolsManager::registerTool(BatchTool* const tool, QWidget* const w)
{
    if (!tool)
    {
        return;
    }

    Private::BatchToolWidgetSet set;
    set.tool           = tool;
    set.settingsWidget = w;
    d->toolsList.append(set);
}

void BatchToolsManager::unregisterTool(BatchTool* const tool)
{
    if (!tool)
    {
        return;
    }

    for (Private::BatchToolsWidgetList::iterator it = d->toolsList.begin(); it != d->toolsList.end();)
    {
        if ((*it).tool == tool)
        {
            delete (*it).tool;
            delete (*it).settingsWidget;
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
    foreach(Private::BatchToolWidgetSet set, d->toolsList)
    {
        if (set.tool->objectName() == name && set.tool->toolGroup() == group)
        {
            return set.tool;
        }
    }

    return 0;
}

QWidget* BatchToolsManager::findSettingsWidget(const QString& name, BatchTool::BatchToolGroup group) const
{
    foreach(Private::BatchToolWidgetSet set, d->toolsList)
    {
        if (set.tool->objectName() == name && set.tool->toolGroup() == group)
        {
            return set.settingsWidget;
        }
    }

    return 0;
}

}  // namespace Digikam
