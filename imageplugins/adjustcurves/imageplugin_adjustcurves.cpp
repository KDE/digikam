/* ============================================================
 * File  : imageplugin_adjustcurves.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-12-01
 * Description : 
 * 
 * Copyright 2004-2006 by Gilles Caulier
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

// KDE includes.
  
#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kcursor.h>
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "bannerwidget.h"
#include "adjustcurves.h"
#include "imageplugin_adjustcurves.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_adjustcurves,
                            KGenericFactory<ImagePlugin_AdjustCurves>("digikamimageplugin_adjustcurves"));

ImagePlugin_AdjustCurves::ImagePlugin_AdjustCurves(QObject *parent, const char*,
                                                   const QStringList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_AdjustCurves")
{
    m_curvesAction = new KAction(i18n("Curves Adjust..."), "adjustcurves", 0, 
                         this, SLOT(slotCurvesAdjust()),
                         actionCollection(), "imageplugin_adjustcurves");

    setXMLFile("digikamimageplugin_adjustcurves_ui.rc");

    kdDebug() << "ImagePlugin_AdjustCurves plugin loaded" << endl;
}

ImagePlugin_AdjustCurves::~ImagePlugin_AdjustCurves()
{
}

void ImagePlugin_AdjustCurves::setEnabledActions(bool enable)
{
    m_curvesAction->setEnabled(enable);
}

void ImagePlugin_AdjustCurves::slotCurvesAdjust()
{
    QString title = i18n("Adjust Color Curves");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamAdjustCurvesImagesPlugin::AdjustCurveDialog dlg(parentWidget(),
                                title, headerFrame);
    dlg.exec();
    delete headerFrame;
}


#include "imageplugin_adjustcurves.moc"
