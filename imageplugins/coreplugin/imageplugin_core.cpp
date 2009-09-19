/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : digiKam image editor plugin core
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_core.h"
#include "imageplugin_core.moc"

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>

// Local includes

#include "dimg.h"
#include "dimgimagefilters.h"
#include "imageiface.h"
#include "iccprofilescombobox.h"
#include "iccsettings.h"
#include "autocorrectiontool.h"
#include "bcgtool.h"
#include "bwsepiatool.h"
#include "hsltool.h"
#include "profileconversiontool.h"
#include "resizetool.h"
#include "blurtool.h"
#include "ratiocroptool.h"
#include "sharpentool.h"
#include "redeyetool.h"
#include "rgbtool.h"

using namespace DigikamImagesPluginCore;
using namespace Digikam;

K_PLUGIN_FACTORY( CorePluginFactory, registerPlugin<ImagePlugin_Core>(); )
K_EXPORT_PLUGIN ( CorePluginFactory("digikamimageplugin_core") )

class ImagePlugin_CorePriv
{

public:

    ImagePlugin_CorePriv()
    {
        redeyeAction          = 0;
        BCGAction             = 0;
        HSLAction             = 0;
        RGBAction             = 0;
        autoCorrectionAction  = 0;
        invertAction          = 0;
        BWAction              = 0;
        aspectRatioCropAction = 0;
        resizeAction          = 0;
        sharpenAction         = 0;
        blurAction            = 0;
        convertTo8Bits        = 0;
        convertTo16Bits       = 0;
        profileMenuAction     = 0;
    }

    KAction *redeyeAction;
    KAction *BCGAction;
    KAction *HSLAction;
    KAction *RGBAction;
    KAction *autoCorrectionAction;
    KAction *invertAction;
    KAction *BWAction;
    KAction *aspectRatioCropAction;
    KAction *resizeAction;
    KAction *sharpenAction;
    KAction *blurAction;
    KAction *convertTo8Bits;
    KAction *convertTo16Bits;
    IccProfilesMenuAction *profileMenuAction;
};

ImagePlugin_Core::ImagePlugin_Core(QObject *parent, const QVariantList &)
                : ImagePlugin(parent, "ImagePlugin_Core"),
                  d(new ImagePlugin_CorePriv)
{
    //-------------------------------
    // Fix and Colors menu actions

    d->blurAction = new KAction(KIcon("blurimage"), i18n("Blur..."), this);
    actionCollection()->addAction("implugcore_blur", d->blurAction );
    connect(d->blurAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotBlur()));

    d->sharpenAction = new KAction(KIcon("sharpenimage"), i18n("Sharpen..."), this);
    actionCollection()->addAction("implugcore_sharpen", d->sharpenAction );
    connect(d->sharpenAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotSharpen()));

    d->redeyeAction = new KAction(KIcon("redeyes"), i18n("Red Eye..."), this);
    d->redeyeAction->setWhatsThis( i18n( "This filter can be used to correct red eyes in a photo. "
                                        "Select a region including the eyes to use this option.") );
    actionCollection()->addAction("implugcore_redeye", d->redeyeAction );
    connect(d->redeyeAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotRedEye()));

    d->BCGAction = new KAction(KIcon("contrast"), i18n("Brightness/Contrast/Gamma..."), this);
    actionCollection()->addAction("implugcore_bcg", d->BCGAction );
    connect(d->BCGAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotBCG()));

    // NOTE: Photoshop 7 use CTRL+U.
    d->HSLAction = new KAction(KIcon("adjusthsl"), i18n("Hue/Saturation/Lightness..."), this);
    d->HSLAction->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_U));
    actionCollection()->addAction("implugcore_hsl", d->HSLAction );
    connect(d->HSLAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotHSL()));

    // NOTE: Photoshop 7 use CTRL+B.
    d->RGBAction = new KAction(KIcon("adjustrgb"), i18n("Color Balance..."), this);
    d->RGBAction->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_B));
    actionCollection()->addAction("implugcore_rgb", d->RGBAction );
    connect(d->RGBAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotRGB()));

    // NOTE: Photoshop 7 use CTRL+SHIFT+B with
    d->autoCorrectionAction = new KAction(KIcon("autocorrection"), i18n("Auto-Correction..."), this);
    d->autoCorrectionAction->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_B));
    actionCollection()->addAction("implugcore_autocorrection", d->autoCorrectionAction );
    connect(d->autoCorrectionAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotAutoCorrection()));

    // NOTE: Photoshop 7 use CTRL+I.
    d->invertAction = new KAction(KIcon("invertimage"), i18n("Invert"), this);
    d->invertAction->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_I));
    actionCollection()->addAction("implugcore_invert", d->invertAction );
    connect(d->invertAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotInvert()));

    d->convertTo8Bits = new KAction(KIcon("depth16to8"), i18n("8 bits"), this);
    actionCollection()->addAction("implugcore_convertto8bits", d->convertTo8Bits );
    connect(d->convertTo8Bits, SIGNAL(triggered(bool) ),
            this, SLOT(slotConvertTo8Bits()));

    d->convertTo16Bits = new KAction(KIcon("depth8to16"), i18n("16 bits"), this);
    actionCollection()->addAction("implugcore_convertto16bits", d->convertTo16Bits );
    connect(d->convertTo16Bits, SIGNAL(triggered(bool) ),
            this, SLOT(slotConvertTo16Bits()));

    /*
    d->colorManagementAction = new KAction(KIcon("colormanagement"), i18n("Color Management..."), this);
    actionCollection()->addAction("implugcore_colormanagement", d->colorManagementAction );
    connect(d->colorManagementAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotColorManagement()));
    */
    d->profileMenuAction = new IccProfilesMenuAction(KIcon("colormanagement"), i18n("Color Space Conversion"), this);
    actionCollection()->addAction("implugcore_colormanagement", d->profileMenuAction );
    connect(d->profileMenuAction, SIGNAL(triggered(const IccProfile &)),
            this, SLOT(slotConvertToColorSpace(const IccProfile &)));

    connect(IccSettings::instance(), SIGNAL(settingsChanged()),
            this, SLOT(slotUpdateColorSpaceMenu()));

    slotUpdateColorSpaceMenu();

    //-------------------------------
    // Filters menu actions.

    d->BWAction = new KAction(KIcon("bwtonal"), i18n("Black && White..."), this);
    actionCollection()->addAction("implugcore_blackwhite", d->BWAction );
    connect(d->BWAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotBW()));

    //-------------------------------
    // Transform menu actions.

    d->aspectRatioCropAction = new KAction(KIcon("ratiocrop"), i18n("Aspect Ratio Crop..."), this);
    actionCollection()->addAction("implugcore_ratiocrop", d->aspectRatioCropAction );
    connect(d->aspectRatioCropAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotRatioCrop()));


    d->resizeAction = new KAction(KIcon("transform-scale"), i18n("&Resize..."), this);
    actionCollection()->addAction("implugcore_resize", d->resizeAction);
    connect(d->resizeAction, SIGNAL(triggered()),
            this, SLOT(slotResize()));

    //-------------------------------
    // Init. menu actions.

    setXMLFile("digikamimageplugin_core_ui.rc");

    kDebug(50006) << "ImagePlugin_Core plugin loaded";
}

ImagePlugin_Core::~ImagePlugin_Core()
{
    delete d;
}

void ImagePlugin_Core::setEnabledSelectionActions(bool)
{
}

void ImagePlugin_Core::setEnabledActions(bool b)
{
    d->convertTo8Bits->setEnabled(b);
    d->convertTo16Bits->setEnabled(b);
    d->invertAction->setEnabled(b);
    d->BCGAction->setEnabled(b);
    d->RGBAction->setEnabled(b);
    d->blurAction->setEnabled(b);
    d->redeyeAction->setEnabled(b);
    d->autoCorrectionAction->setEnabled(b);
    d->BWAction->setEnabled(b);
    d->HSLAction->setEnabled(b);
    d->sharpenAction->setEnabled(b);
    d->aspectRatioCropAction->setEnabled(b);
    d->resizeAction->setEnabled(b);
    d->profileMenuAction->setEnabled(b);
}

void ImagePlugin_Core::slotInvert()
{
    kapp->setOverrideCursor(Qt::WaitCursor);

    ImageIface iface(0, 0);

    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();

    DImgImageFilters filter;
    filter.invertImage(data, w, h, sixteenBit);
    iface.putOriginalImage(i18n("Invert"), data);
    delete [] data;

    kapp->restoreOverrideCursor();
}

void ImagePlugin_Core::slotConvertTo8Bits()
{
    ImageIface iface(0, 0);

    if (!iface.originalSixteenBit())
    {
       KMessageBox::error(kapp->activeWindow(), i18n("This image is already using a depth of 8 bits / color / pixel."));
       return;
    }
    else
    {
       if (KMessageBox::warningContinueCancel(
                        kapp->activeWindow(),
                        i18n("Performing this operation will reduce image color quality. "
                             "Do you want to continue?"), QString(), 
                        KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                        QString("ImagePluginCore16To8Bits")) == KMessageBox::Cancel)
           return;
    }

    kapp->setOverrideCursor(Qt::WaitCursor);
    iface.convertOriginalColorDepth(32);
    kapp->restoreOverrideCursor();
}

void ImagePlugin_Core::slotConvertTo16Bits()
{
    ImageIface iface(0, 0);

    if (iface.originalSixteenBit())
    {
       KMessageBox::error(kapp->activeWindow(), i18n("This image is already using a depth of 16 bits / color / pixel."));
       return;
    }

    kapp->setOverrideCursor(Qt::WaitCursor);
    iface.convertOriginalColorDepth(64);
    kapp->restoreOverrideCursor();
}

void ImagePlugin_Core::slotBCG()
{
    BCGTool *tool = new BCGTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotRGB()
{
    RGBTool *tool = new RGBTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotBlur()
{
    BlurTool *tool = new BlurTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotAutoCorrection()
{
    AutoCorrectionTool *tool = new AutoCorrectionTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotRedEye()
{
    ImageIface iface(0, 0);

    if (!iface.selectedWidth() || !iface.selectedHeight())
    {
        RedEyePassivePopup* popup = new RedEyePassivePopup(kapp->activeWindow());
        popup->setView(i18n("Red-Eye Correction Tool"),
                       i18n("You need to select a region including the eyes to use "
                            "the red-eye correction tool"));
        popup->setAutoDelete(true);
        popup->setTimeout(2500);
        popup->show();
        return;
    }

    RedEyeTool *tool = new RedEyeTool(this);
    loadTool(tool);
}

/*
void ImagePlugin_Core::slotColorManagement()
{
    ICCProofTool *tool = new ICCProofTool(this);
    loadTool(tool);
}
*/

void ImagePlugin_Core::slotConvertToColorSpace(const IccProfile &profile)
{
    kDebug() << "";
    ImageIface iface(0, 0);

    if (iface.getOriginalIccProfile().isNull())
    {
       KMessageBox::error(kapp->activeWindow(), i18n("This image is not color managed."));
       return;
    }

    kapp->setOverrideCursor(Qt::WaitCursor);
    ProfileConversionTool::fastConversion(profile);
    kapp->restoreOverrideCursor();
}

void ImagePlugin_Core::slotUpdateColorSpaceMenu()
{
    d->profileMenuAction->clear();

    ICCSettingsContainer settings = IccSettings::instance()->settings();
    if (!settings.enableCM)
    {
        d->profileMenuAction->setEnabled(false);
        return;
    }

    QList<IccProfile> standardProfiles, favoriteProfiles;
    QSet<QString> standardProfilePaths, favoriteProfilePaths;
    standardProfiles << IccProfile::sRGB()
                     << IccProfile::adobeRGB()
                     << IccProfile::wideGamutRGB()
                     << IccProfile::proPhotoRGB();

    foreach (IccProfile profile, standardProfiles)
    {
        d->profileMenuAction->addProfile(profile, profile.description());
        standardProfilePaths << profile.filePath();
    }

    d->profileMenuAction->addSeparator();

    favoriteProfilePaths = QSet<QString>::fromList(ProfileConversionTool::favoriteProfiles());
    favoriteProfilePaths -= standardProfilePaths;
    foreach (const QString &path, favoriteProfilePaths)
        favoriteProfiles << path;
    d->profileMenuAction->addProfiles(favoriteProfiles);

    d->profileMenuAction->addSeparator();

    KAction *moreAction = new KAction(i18n("Other..."), this);
    d->profileMenuAction->addAction(moreAction);
    connect(moreAction, SIGNAL(triggered()), this, SLOT(slotProfileConversionTool()));
}

void ImagePlugin_Core::slotProfileConversionTool()
{
    ProfileConversionTool *tool = new ProfileConversionTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotBW()
{
    BWSepiaTool *tool = new BWSepiaTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotHSL()
{
    HSLTool *tool = new HSLTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotSharpen()
{
    SharpenTool *tool = new SharpenTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotRatioCrop()
{
    RatioCropTool *tool = new RatioCropTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotResize()
{
    ResizeTool *tool = new ResizeTool(this);
    loadTool(tool);
}
