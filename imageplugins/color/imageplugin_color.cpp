/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : digiKam image editor plugin to correct color
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
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

#include "imageplugin_color.moc"

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "invertfilter.h"
#include "imageiface.h"
#include "editortooliface.h"
#include "iccprofilescombobox.h"
#include "iccsettings.h"
#include "autocorrectiontool.h"
#include "bcgtool.h"
#include "bwsepiatool.h"
#include "hsltool.h"
#include "profileconversiontool.h"
#include "cbtool.h"
#include "whitebalancetool.h"
#include "channelmixertool.h"
#include "adjustcurvestool.h"
#include "adjustlevelstool.h"
#include "filmtool.h"

namespace DigikamColorImagePlugin
{

K_PLUGIN_FACTORY( ColorPluginFactory, registerPlugin<ImagePlugin_Color>(); )
K_EXPORT_PLUGIN ( ColorPluginFactory("digikamimageplugin_color") )

class ImagePlugin_Color::Private
{

public:

    Private() :
        BCGAction(0),
        HSLAction(0),
        CBAction(0),
        autoCorrectionAction(0),
        invertAction(0),
        BWAction(0),
        convertTo8Bits(0),
        convertTo16Bits(0),
        whitebalanceAction(0),
        channelMixerAction(0),
        curvesAction(0),
        levelsAction(0),
        filmAction(0),
        profileMenuAction(0)
    {}

    KAction*               BCGAction;
    KAction*               HSLAction;
    KAction*               CBAction;
    KAction*               autoCorrectionAction;
    KAction*               invertAction;
    KAction*               BWAction;
    KAction*               convertTo8Bits;
    KAction*               convertTo16Bits;
    KAction*               whitebalanceAction;
    KAction*               channelMixerAction;
    KAction*               curvesAction;
    KAction*               levelsAction;
    KAction*               filmAction;

    IccProfilesMenuAction* profileMenuAction;
};

ImagePlugin_Color::ImagePlugin_Color(QObject* const parent, const QVariantList&)
    : ImagePlugin(parent, "ImagePlugin_Color"),
      d(new Private)
{
    //-------------------------------
    // Colors menu actions

    d->BCGAction = new KAction(KIcon("contrast"), i18n("Brightness/Contrast/Gamma..."), this);
    actionCollection()->addAction("imageplugin_bcg", d->BCGAction );
    connect(d->BCGAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBCG()));

    // NOTE: Photoshop 7 use CTRL+U.
    d->HSLAction = new KAction(KIcon("adjusthsl"), i18n("Hue/Saturation/Lightness..."), this);
    d->HSLAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_U));
    actionCollection()->addAction("imageplugin_hsl", d->HSLAction );
    connect(d->HSLAction, SIGNAL(triggered(bool)),
            this, SLOT(slotHSL()));

    // NOTE: Photoshop 7 use CTRL+B.
    d->CBAction = new KAction(KIcon("adjustrgb"), i18n("Color Balance..."), this);
    d->CBAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_B));
    actionCollection()->addAction("imageplugin_rgb", d->CBAction );
    connect(d->CBAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCB()));

    // NOTE: Photoshop 7 use CTRL+SHIFT+B with
    d->autoCorrectionAction = new KAction(KIcon("autocorrection"), i18n("Auto-Correction..."), this);
    d->autoCorrectionAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_B));
    actionCollection()->addAction("imageplugin_autocorrection", d->autoCorrectionAction );
    connect(d->autoCorrectionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAutoCorrection()));

    // NOTE: Photoshop 7 use CTRL+I.
    d->invertAction = new KAction(KIcon("invertimage"), i18n("Invert"), this);
    d->invertAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_I));
    actionCollection()->addAction("imageplugin_invert", d->invertAction );
    connect(d->invertAction, SIGNAL(triggered(bool)),
            this, SLOT(slotInvert()));

    d->convertTo8Bits = new KAction(KIcon("depth16to8"), i18n("8 bits"), this);
    actionCollection()->addAction("imageplugin_convertto8bits", d->convertTo8Bits );
    connect(d->convertTo8Bits, SIGNAL(triggered(bool)),
            this, SLOT(slotConvertTo8Bits()));

    d->convertTo16Bits = new KAction(KIcon("depth8to16"), i18n("16 bits"), this);
    actionCollection()->addAction("imageplugin_convertto16bits", d->convertTo16Bits );
    connect(d->convertTo16Bits, SIGNAL(triggered(bool)),
            this, SLOT(slotConvertTo16Bits()));

    d->profileMenuAction = new IccProfilesMenuAction(KIcon("colormanagement"), i18n("Color Space Conversion"), this);
    actionCollection()->addAction("imageplugin_colormanagement", d->profileMenuAction );
    connect(d->profileMenuAction, SIGNAL(triggered(IccProfile)),
            this, SLOT(slotConvertToColorSpace(IccProfile)));

    connect(IccSettings::instance(), SIGNAL(settingsChanged()),
            this, SLOT(slotUpdateColorSpaceMenu()));

    slotUpdateColorSpaceMenu();

    d->BWAction = new KAction(KIcon("bwtonal"), i18n("Black && White..."), this);
    actionCollection()->addAction("imageplugin_blackwhite", d->BWAction );
    connect(d->BWAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBW()));

    d->whitebalanceAction = new KAction(KIcon("whitebalance"), i18n("White Balance..."), this);
    d->whitebalanceAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_W));
    actionCollection()->addAction("imageplugin_whitebalance", d->whitebalanceAction );
    connect(d->whitebalanceAction, SIGNAL(triggered(bool)),
            this, SLOT(slotWhiteBalance()));

    d->channelMixerAction = new KAction(KIcon("channelmixer"), i18n("Channel Mixer..."), this);
    d->channelMixerAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_H));
    actionCollection()->addAction("imageplugin_channelmixer", d->channelMixerAction );
    connect(d->channelMixerAction, SIGNAL(triggered(bool)),
            this, SLOT(slotChannelMixer()));

    d->curvesAction = new KAction(KIcon("adjustcurves"), i18n("Curves Adjust..."), this);
    // NOTE: Photoshop 7 use CTRL+M (but it's used in KDE to toogle menu bar).
    d->curvesAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_C));
    actionCollection()->addAction("imageplugin_adjustcurves", d->curvesAction);
    connect(d->curvesAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCurvesAdjust()));

    d->levelsAction  = new KAction(KIcon("adjustlevels"), i18n("Levels Adjust..."), this);
    d->levelsAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_L));
    actionCollection()->addAction("imageplugin_adjustlevels", d->levelsAction );
    connect(d->levelsAction, SIGNAL(triggered(bool)),
            this, SLOT(slotLevelsAdjust()));

    d->filmAction = new KAction(KIcon("colorneg"), i18n("Color Negative..."), this);
    d->filmAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_I));
    actionCollection()->addAction("imageplugin_film", d->filmAction);
    connect(d->filmAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFilm()));

    setActionCategory(i18n("Colors"));
    setXMLFile("digikamimageplugin_color_ui.rc");

    kDebug() << "ImagePlugin_Color plugin loaded";
}

ImagePlugin_Color::~ImagePlugin_Color()
{
    delete d;
}

void ImagePlugin_Color::setEnabledSelectionActions(bool)
{
}

void ImagePlugin_Color::setEnabledActions(bool b)
{
    d->convertTo8Bits->setEnabled(b);
    d->convertTo16Bits->setEnabled(b);
    d->invertAction->setEnabled(b);
    d->BCGAction->setEnabled(b);
    d->CBAction->setEnabled(b);
    d->autoCorrectionAction->setEnabled(b);
    d->BWAction->setEnabled(b);
    d->HSLAction->setEnabled(b);
    d->profileMenuAction->setEnabled(b);
    d->whitebalanceAction->setEnabled(b);
    d->channelMixerAction->setEnabled(b);
    d->curvesAction->setEnabled(b);
    d->levelsAction->setEnabled(b);
    d->filmAction->setEnabled(b);
}

void ImagePlugin_Color::slotInvert()
{
    kapp->setOverrideCursor(Qt::WaitCursor);

    ImageIface iface;
    InvertFilter invert(iface.original(), 0L);
    invert.startFilterDirectly();
    iface.setOriginal(i18n("Invert"), invert.filterAction(), invert.getTargetImage());

    kapp->restoreOverrideCursor();
}

void ImagePlugin_Color::slotConvertTo8Bits()
{
    ImageIface iface;

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
                QString("ImagePluginColor16To8Bits")) == KMessageBox::Cancel)
        {
            return;
        }
    }

    kapp->setOverrideCursor(Qt::WaitCursor);
    iface.convertOriginalColorDepth(32);
    kapp->restoreOverrideCursor();
}

void ImagePlugin_Color::slotConvertTo16Bits()
{
    ImageIface iface;

    if (iface.originalSixteenBit())
    {
        KMessageBox::error(kapp->activeWindow(), i18n("This image is already using a depth of 16 bits / color / pixel."));
        return;
    }

    kapp->setOverrideCursor(Qt::WaitCursor);
    iface.convertOriginalColorDepth(64);
    kapp->restoreOverrideCursor();
}

void ImagePlugin_Color::slotConvertToColorSpace(const IccProfile& profile)
{
    ImageIface iface;

    if (iface.originalIccProfile().isNull())
    {
        KMessageBox::error(kapp->activeWindow(), i18n("This image is not color managed."));
        return;
    }

    kapp->setOverrideCursor(Qt::WaitCursor);
    ProfileConversionTool::fastConversion(profile);
    kapp->restoreOverrideCursor();
}

void ImagePlugin_Color::slotUpdateColorSpaceMenu()
{
    d->profileMenuAction->clear();

    if (!IccSettings::instance()->isEnabled())
    {
        KAction* const action = new KAction(i18n("Color Management is disabled..."), this);
        d->profileMenuAction->addAction(action);

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotSetupICC()));
        return;
    }

    ICCSettingsContainer settings = IccSettings::instance()->settings();

    QList<IccProfile> standardProfiles, favoriteProfiles;
    QSet<QString> standardProfilePaths, favoriteProfilePaths;
    standardProfiles << IccProfile::sRGB()
                     << IccProfile::adobeRGB()
                     << IccProfile::wideGamutRGB()
                     << IccProfile::proPhotoRGB();

    foreach(IccProfile profile, standardProfiles) // krazy:exclude=foreach
    {
        d->profileMenuAction->addProfile(profile, profile.description());
        standardProfilePaths << profile.filePath();
    }

    d->profileMenuAction->addSeparator();

    favoriteProfilePaths = QSet<QString>::fromList(ProfileConversionTool::favoriteProfiles());
    favoriteProfilePaths -= standardProfilePaths;

    foreach(const QString& path, favoriteProfilePaths)
    {
        favoriteProfiles << path;
    }

    d->profileMenuAction->addProfiles(favoriteProfiles);
    d->profileMenuAction->addSeparator();

    KAction* const moreAction = new KAction(i18n("Other..."), this);
    d->profileMenuAction->addAction(moreAction);

    connect(moreAction, SIGNAL(triggered()),
            this, SLOT(slotProfileConversionTool()));
}

void ImagePlugin_Color::slotSetupICC()
{
    EditorToolIface::editorToolIface()->setupICC();
}

void ImagePlugin_Color::slotProfileConversionTool()
{
    ProfileConversionTool* const tool = new ProfileConversionTool(this);

    connect(tool, SIGNAL(okClicked()), 
            this, SLOT(slotUpdateColorSpaceMenu()));

    loadTool(tool);
}

void ImagePlugin_Color::slotBW()
{
    loadTool(new BWSepiaTool(this));
}

void ImagePlugin_Color::slotHSL()
{
    loadTool(new HSLTool(this));
}

void ImagePlugin_Color::slotWhiteBalance()
{
    loadTool(new WhiteBalanceTool(this));
}

void ImagePlugin_Color::slotChannelMixer()
{
    loadTool(new ChannelMixerTool(this));
}

void ImagePlugin_Color::slotCurvesAdjust()
{
    loadTool(new AdjustCurvesTool(this));
}

void ImagePlugin_Color::slotLevelsAdjust()
{
    loadTool(new AdjustLevelsTool(this));
}

void ImagePlugin_Color::slotFilm()
{
    loadTool(new FilmTool(this));
}

void ImagePlugin_Color::slotBCG()
{
    loadTool(new BCGTool(this));
}

void ImagePlugin_Color::slotCB()
{
    loadTool(new CBTool(this));
}

void ImagePlugin_Color::slotAutoCorrection()
{
    loadTool(new AutoCorrectionTool(this));
}

} // namespace DigikamColorImagePlugin
