/* ============================================================
 * File  : imageplugin_core.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-06-04
 * Description :
 *
 * Copyright 2004 by Renchi Raju and Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
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

// Local includes.

#include <imageiface.h>
#include "histogramviewer.h"
#include "imageeffect_rgb.h"
#include "imageeffect_hsl.h"
#include "imageeffect_bcg.h"
#include "imageeffect_bwsepia.h"
#include "imageeffect_redeye.h"
#include "imageeffect_blur.h"
#include "imageeffect_sharpen.h"
#include "imageeffect_colorsenhance.h"
#include "imageplugin_core.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_core,
                            KGenericFactory<ImagePlugin_Core>("digikam"));

ImagePlugin_Core::ImagePlugin_Core(QObject *parent, const char*,
                                   const QStringList &)
                : Digikam::ImagePlugin(parent, "ImagePlugin_Core")
{
    //-------------------------------
    // Fix/Colors menu actions

    new KAction(i18n("Blur..."), 0,
                this, SLOT(slotBlur()),
                actionCollection(), "implugcore_blur");

    new KAction(i18n("Sharpen..."), 0,
                this, SLOT(slotSharpen()),
                actionCollection(), "implugcore_sharpen");

    m_redeyeAction = new KAction(i18n("Red Eye Reduction..."), 0,
                                 this, SLOT(slotRedEye()),
                                 actionCollection(), "implugcore_redeye");
    m_redeyeAction->setEnabled(false);
    m_redeyeAction->setWhatsThis( i18n( "This filter can be used to correct red eyes in a photo. "
                                        "Select a region including the eyes to enable this action.") );

    new KAction(i18n("Brightness/Contrast/Gamma..."), 0,
                this, SLOT(slotBCG()),
                actionCollection(), "implugcore_bcg");

    new KAction(i18n("Hue/Saturation/Lightness..."), 0,
                this, SLOT(slotHSL()),
                actionCollection(), "implugcore_hsl");

    new KAction(i18n("Color Balance..."), 0,
                this, SLOT(slotRGB()),
                actionCollection(), "implugcore_rgb");

    KAction *normalizeAction = new KAction(i18n("Normalize"), 0,
                this, SLOT(slotNormalize()),
                actionCollection(), "implugcore_normalize");
    normalizeAction->setWhatsThis( i18n( "This option scales brightness values across the active "
                                         "image so that the darkest point becomes black, and the "
                                         "brightest point becomes as bright as possible without "
                                         "altering its hue. This is often a \"magic fix\" for "
                                         "images that are dim or washed out."));                

    KAction *equalizeAction = new KAction(i18n("Equalize"), 0,
                this, SLOT(slotEqualize()),
                actionCollection(), "implugcore_equalize");
    equalizeAction->setWhatsThis( i18n( "This option adjusts the brightness of colors across the "
                                        "active image so that the histogram for the Value channel "
                                        "is as nearly as possible flat, that is, so that each possible "
                                        "brightness value appears at about the same number of pixels "
                                        "as each other value. Sometimes Equalize works wonderfully at "
                                        "enhancing the contrasts in an image. Other times it gives "
                                        "garbage. It is a very powerful operation, which can either work "
                                        "miracles on an image or destroy it.display the current image histogram."));  
                                         
    KAction *autolevelseAction = new KAction(i18n("Auto Levels"), 0,
                this, SLOT(slotAutoLevels()),
                actionCollection(), "implugcore_autolevels");
    autolevelseAction->setWhatsThis( i18n( "This option maximizes the tonal range in the Red, Green, "
                                           "and Blue channels. It search the image shadow and highlight "
                                           "limit values and adjust the Red, Green, and Blue channels "
                                           "to a full histogram range."));  
    //-------------------------------
    // Image menu actions.

    KAction *histogramAction = new KAction(i18n("Histogram..."), 0,
                this, SLOT(slotHistogramViewer()),
                actionCollection(), "implugcore_histogramviewer");
    histogramAction->setWhatsThis( i18n( "This option display the current image histogram. "
                                         "If you have selected a region, you can choose an histogram "
                                         "rendering for the full image or the current image selection."));

    //-------------------------------
    // Filters menu actions.

    new KAction(i18n("Convert to Black-White"), 0,
                this, SLOT(slotBW()),
                actionCollection(), "implugcore_bw");

    new KAction(i18n("Convert to Sepia"), 0,
                this, SLOT(slotSepia()),
                actionCollection(), "implugcore_sepia");

    kdDebug() << "ImagePlugin_Core plugin loaded" << endl;
}

ImagePlugin_Core::~ImagePlugin_Core()
{
}

QStringList ImagePlugin_Core::guiDefinition() const
{
    QStringList guiDef;

    guiDef.append("MenuBar/Menu/&Image/Image/Action/implugcore_histogramviewer/ ");

    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/implugcore_bcg/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/implugcore_hsl/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/implugcore_rgb/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Separator/ /  ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/implugcore_normalize/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/implugcore_equalize/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/implugcore_autolevels/ ");

    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_colors/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Separator/ / ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_blur/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_sharpen/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Separator/ / ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_redeye/ ");

    guiDef.append("MenuBar/Menu/Fi&lters/Generic/Action/implugcore_bw/ ");
    guiDef.append("MenuBar/Menu/Fi&lters/Generic/Action/implugcore_sepia/ ");

    // Enable i18n for the menu options.

    i18n( "&Image" );
    i18n( "Fi&x" );
    i18n( "Fi&lters" );
    i18n( "&Colors" );

    return guiDef;
}

void ImagePlugin_Core::setEnabledSelectionActions(bool enable)
{
    m_redeyeAction->setEnabled(enable);
}

void ImagePlugin_Core::slotBlur()
{
    ImageEffect_Blur dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotSharpen()
{
    ImageEffect_Sharpen dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotBCG()
{
    ImageEffect_BCG dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotRGB()
{
    ImageEffect_RGB dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotHSL()
{
    ImageEffect_HSL dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotNormalize()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    ImageEffect_ColorsEnhance::normalizeImage();
    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotEqualize()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    ImageEffect_ColorsEnhance::equalizeImage();
    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotAutoLevels()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    ImageEffect_ColorsEnhance::autoLevelsCorrectionImage();
    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotHistogramViewer()
{
    Digikam::ImageIface iface(0, 0);

    uint* i_data = iface.getOriginalData();
    int i_w      = iface.originalWidth();
    int i_h      = iface.originalHeight();

    uint* s_data = iface.getSelectedData();
    int s_w      = iface.selectedWidth();
    int s_h      = iface.selectedHeight();
     
    if (!s_data || !s_w || !s_h)
        {
        HistogramViewer dlg(parentWidget(), i_data, i_w, i_h);
        dlg.exec();
        delete [] i_data;
        }
    else 
        {
        HistogramViewer dlg(parentWidget(), i_data, i_w, i_h, 
                            s_data, s_w, s_h);
        dlg.exec();
        delete [] i_data;
        delete [] s_data;
        }
}

void ImagePlugin_Core::slotBW()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    ImageEffect_BWSepia::convertTOBW();
    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotSepia()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    ImageEffect_BWSepia::convertTOSepia();
    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotRedEye()
{
    ImageEffect_RedEye::removeRedEye(parentWidget());
}


#include "imageplugin_core.moc"
