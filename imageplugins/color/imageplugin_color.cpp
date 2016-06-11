/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : digiKam image editor plugin to correct color
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_color.h"

// Qt includes

#include <QApplication>
#include <QKeySequence>
#include <QAction>

// KDE includes

#include <kactioncollection.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "dmessagebox.h"
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

    QAction*               BCGAction;
    QAction*               HSLAction;
    QAction*               CBAction;
    QAction*               autoCorrectionAction;
    QAction*               invertAction;
    QAction*               BWAction;
    QAction*               convertTo8Bits;
    QAction*               convertTo16Bits;
    QAction*               whitebalanceAction;
    QAction*               channelMixerAction;
    QAction*               curvesAction;
    QAction*               levelsAction;
    QAction*               filmAction;

    IccProfilesMenuAction* profileMenuAction;
};

ImagePlugin_Color::ImagePlugin_Color(QObject* const parent, const QVariantList&)
    : ImagePlugin(parent, QLatin1String("ImagePlugin_Color")),
      d(new Private)
{
    // to load the rc file from digikam's installation path
    setComponentName(QLatin1String("digikam"), i18nc("to be displayed in shortcuts dialog", "Color adjustment plugins"));

    //-------------------------------
    // Colors menu actions

    KActionCollection* const ac = actionCollection();

    d->BCGAction = new QAction(QIcon::fromTheme(QLatin1String("contrast")), i18n("Brightness/Contrast/Gamma..."), this);
    ac->addAction(QLatin1String("imageplugin_bcg"), d->BCGAction);
    connect(d->BCGAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBCG()));

    // NOTE: Photoshop 7 use CTRL+U.
    d->HSLAction = new QAction(QIcon::fromTheme(QLatin1String("adjusthsl")), i18n("Hue/Saturation/Lightness..."), this);
    ac->addAction(QLatin1String("imageplugin_hsl"), d->HSLAction);
    ac->setDefaultShortcut(d->HSLAction, Qt::CTRL+Qt::Key_U);
    connect(d->HSLAction, SIGNAL(triggered(bool)),
            this, SLOT(slotHSL()));

    // NOTE: Photoshop 7 use CTRL+B.
    d->CBAction = new QAction(QIcon::fromTheme(QLatin1String("adjustrgb")), i18n("Color Balance..."), this);
    ac->addAction(QLatin1String("imageplugin_rgb"), d->CBAction);
    ac->setDefaultShortcut(d->CBAction, Qt::CTRL+Qt::Key_B);
    connect(d->CBAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCB()));

    // NOTE: Photoshop 7 use CTRL+SHIFT+B with
    d->autoCorrectionAction = new QAction(QIcon::fromTheme(QLatin1String("autocorrection")), i18n("Auto-Correction..."), this);
    ac->addAction(QLatin1String("imageplugin_autocorrection"), d->autoCorrectionAction);
    ac->setDefaultShortcut(d->autoCorrectionAction, Qt::CTRL+Qt::SHIFT+Qt::Key_B);
    connect(d->autoCorrectionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAutoCorrection()));

    // NOTE: Photoshop 7 use CTRL+I.
    d->invertAction = new QAction(QIcon::fromTheme(QLatin1String("edit-select-invert")), i18n("Invert"), this);
    ac->addAction(QLatin1String("imageplugin_invert"), d->invertAction);
    ac->setDefaultShortcut(d->invertAction, Qt::CTRL+Qt::Key_I);
    connect(d->invertAction, SIGNAL(triggered(bool)),
            this, SLOT(slotInvert()));

    d->convertTo8Bits = new QAction(QIcon::fromTheme(QLatin1String("depth16to8")), i18n("8 bits"), this);
    ac->addAction(QLatin1String("imageplugin_convertto8bits"), d->convertTo8Bits);
    connect(d->convertTo8Bits, SIGNAL(triggered(bool)),
            this, SLOT(slotConvertTo8Bits()));

    d->convertTo16Bits = new QAction(QIcon::fromTheme(QLatin1String("depth8to16")), i18n("16 bits"), this);
    ac->addAction(QLatin1String("imageplugin_convertto16bits"), d->convertTo16Bits);
    connect(d->convertTo16Bits, SIGNAL(triggered(bool)),
            this, SLOT(slotConvertTo16Bits()));

    d->profileMenuAction = new IccProfilesMenuAction(QIcon::fromTheme(QLatin1String("preferences-desktop-display-color")), i18n("Color Space Conversion"), this);
    ac->addAction(QLatin1String("imageplugin_colormanagement"), d->profileMenuAction->menuAction());
    connect(d->profileMenuAction, SIGNAL(triggered(IccProfile)),
            this, SLOT(slotConvertToColorSpace(IccProfile)));

    connect(IccSettings::instance(), SIGNAL(settingsChanged()),
            this, SLOT(slotUpdateColorSpaceMenu()));

    slotUpdateColorSpaceMenu();

    d->BWAction = new QAction(QIcon::fromTheme(QLatin1String("bwtonal")), i18n("Black && White..."), this);
    ac->addAction(QLatin1String("imageplugin_blackwhite"), d->BWAction);
    connect(d->BWAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBW()));

    d->whitebalanceAction = new QAction(QIcon::fromTheme(QLatin1String("bordertool")), i18n("White Balance..."), this);
    ac->addAction(QLatin1String("imageplugin_whitebalance"), d->whitebalanceAction);
    ac->setDefaultShortcut(d->whitebalanceAction, Qt::CTRL+Qt::SHIFT+Qt::Key_W);
    connect(d->whitebalanceAction, SIGNAL(triggered(bool)),
            this, SLOT(slotWhiteBalance()));

    d->channelMixerAction = new QAction(QIcon::fromTheme(QLatin1String("channelmixer")), i18n("Channel Mixer..."), this);
    ac->addAction(QLatin1String("imageplugin_channelmixer"), d->channelMixerAction);
    ac->setDefaultShortcut(d->channelMixerAction, Qt::CTRL+Qt::Key_H);
    connect(d->channelMixerAction, SIGNAL(triggered(bool)),
            this, SLOT(slotChannelMixer()));

    d->curvesAction = new QAction(QIcon::fromTheme(QLatin1String("adjustcurves")), i18n("Curves Adjust..."), this);
    // NOTE: Photoshop 7 use CTRL+M (but it's used in KDE to toogle menu bar).
    ac->addAction(QLatin1String("imageplugin_adjustcurves"), d->curvesAction);
    ac->setDefaultShortcut(d->curvesAction, Qt::CTRL+Qt::SHIFT+Qt::Key_C);
    connect(d->curvesAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCurvesAdjust()));

    d->levelsAction  = new QAction(QIcon::fromTheme(QLatin1String("adjustlevels")), i18n("Levels Adjust..."), this);
    ac->addAction(QLatin1String("imageplugin_adjustlevels"), d->levelsAction);
    ac->setDefaultShortcut(d->levelsAction, Qt::CTRL+Qt::Key_L);
    connect(d->levelsAction, SIGNAL(triggered(bool)),
            this, SLOT(slotLevelsAdjust()));

    d->filmAction = new QAction(QIcon::fromTheme(QLatin1String("colorneg")), i18n("Color Negative..."), this);
    ac->addAction(QLatin1String("imageplugin_film"), d->filmAction);
    ac->setDefaultShortcut(d->filmAction, Qt::CTRL+Qt::SHIFT+Qt::Key_I);
    connect(d->filmAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFilm()));

    setActionCategory(i18n("Colors"));
    setXMLFile(QLatin1String("digikamimageplugin_color_ui.rc"));

    qCDebug(DIGIKAM_IMAGEPLUGINS_LOG) << "ImagePlugin_Color plugin loaded";
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
    qApp->setOverrideCursor(Qt::WaitCursor);

    ImageIface iface;
    InvertFilter invert(iface.original(), 0L);
    invert.startFilterDirectly();
    iface.setOriginal(i18n("Invert"), invert.filterAction(), invert.getTargetImage());

    qApp->restoreOverrideCursor();
}

void ImagePlugin_Color::slotConvertTo8Bits()
{
    ImageIface iface;

    if (!iface.originalSixteenBit())
    {
        QMessageBox::critical(qApp->activeWindow(),
                              qApp->applicationName(),
                              i18n("This image is already using a depth of 8 bits / color / pixel."));
        return;
    }
    else
    {
        if (DMessageBox::showContinueCancel(QMessageBox::Warning,
                                            qApp->activeWindow(),
                                            qApp->applicationName(),
                                            i18n("Performing this operation will reduce image color quality. "
                                            "Do you want to continue?"),
                                            QLatin1String("ImagePluginColor16To8Bits"))
            == QMessageBox::Cancel)
        {
            return;
        }
    }

    qApp->setOverrideCursor(Qt::WaitCursor);
    iface.convertOriginalColorDepth(32);
    qApp->restoreOverrideCursor();
}

void ImagePlugin_Color::slotConvertTo16Bits()
{
    ImageIface iface;

    if (iface.originalSixteenBit())
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("This image is already using a depth of 16 bits / color / pixel."));
        return;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);
    iface.convertOriginalColorDepth(64);
    qApp->restoreOverrideCursor();
}

void ImagePlugin_Color::slotConvertToColorSpace(const IccProfile& profile)
{
    ImageIface iface;

    if (iface.originalIccProfile().isNull())
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("This image is not color managed."));
        return;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);
    ProfileConversionTool::fastConversion(profile);
    qApp->restoreOverrideCursor();
}

void ImagePlugin_Color::slotUpdateColorSpaceMenu()
{
    d->profileMenuAction->clear();

    if (!IccSettings::instance()->isEnabled())
    {
        QAction* const action = new QAction(i18n("Color Management is disabled..."), this);
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

    QAction* const moreAction = new QAction(i18n("Other..."), this);
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

#include "imageplugin_color.moc"
