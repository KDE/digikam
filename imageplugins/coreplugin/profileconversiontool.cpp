/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-19
 * Description : a tool for color space conversion
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#include "profileconversiontool.h"
#include "profileconversiontool.moc"

// Qt includes

#include <QCache>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kurl.h>

// Local includes

#include "editortoolsettings.h"
#include "iccprofileinfodlg.h"
#include "iccprofilescombobox.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "icctransformfilter.h"
#include "imageiface.h"
#include "imagepanelwidget.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

class ProfileConversionToolPriv
{
public:

    ProfileConversionToolPriv()
    {
        profilesBox       = 0;
        previewWidget     = 0;
        gboxSettings      = 0;

        favoriteProfiles.setMaxCost(10);
    }

    IccProfilesComboBox*     profilesBox;

    ImagePanelWidget*        previewWidget;
    EditorToolSettings*      gboxSettings;

    IccProfile               currentProfile;
    QCache<QString, QString> favoriteProfiles;

    IccTransform             transform;

    static IccTransform getTransform(const IccProfile& in, const IccProfile& out);
};

IccTransform ProfileConversionToolPriv::getTransform(const IccProfile& in, const IccProfile& out)
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    IccTransform transform;
    transform.setIntent(settings.renderingIntent);
    transform.setUseBlackPointCompensation(settings.useBPC);

    transform.setInputProfile(in);
    transform.setOutputProfile(out);

    return transform;
}

ProfileConversionTool::ProfileConversionTool(QObject* parent)
                     : EditorToolThreaded(parent),
                       d(new ProfileConversionToolPriv)
{
    setObjectName("profile conversion");
    setToolName(i18n("Color Profile Conversion"));
    setToolIcon(SmallIcon("colormanagement"));
    //TODO setToolHelp("colormanagement.anchor");

    // -------------------------------------------------------------

    ImageIface iface(0, 0);
    d->currentProfile = iface.getOriginalIccProfile();

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(Digikam::LRGBA);

    QGridLayout* grid = new QGridLayout(d->gboxSettings->plainPage());

    // ---

    QVBoxLayout *currentProfVBox = new QVBoxLayout;
    QLabel *currentProfileTitle  = new QLabel(i18n("Current Color Space:"));
    QLabel *currentProfileDesc   = new QLabel(QString("<b>%1</b>").arg(d->currentProfile.description()));
    QPushButton *currentProfInfo = new QPushButton(i18n("Info..."));
    currentProfileDesc->setWordWrap(true);

    currentProfVBox->addWidget(currentProfileTitle);
    currentProfVBox->addWidget(currentProfileDesc);
    currentProfVBox->addWidget(currentProfInfo, 0, Qt::AlignLeft);

    // ---

    QVBoxLayout *newProfVBox = new QVBoxLayout;

    QLabel *newProfileLabel  = new QLabel(i18n("Convert to:"));
    d->profilesBox = new IccProfilesComboBox;
    d->profilesBox->addProfilesSqueezed(IccSettings::instance()->workspaceProfiles());
    d->profilesBox->setWhatsThis( i18n("Select the profile of the color space to convert to."));
    newProfileLabel->setBuddy(d->profilesBox);
    QPushButton *newProfInfo = new QPushButton(i18n("Info..."));

    newProfVBox->addWidget(newProfileLabel);
    newProfVBox->addWidget(d->profilesBox);
    newProfVBox->addWidget(newProfInfo, 0, Qt::AlignLeft);

    // ---

    grid->addLayout(currentProfVBox, 0, 0);
    grid->addLayout(newProfVBox,     1, 0);

    // ---

    setToolSettings(d->gboxSettings);

    /*d->previewWidget = new ImageWidget("Profile Conversion Tool", 0,
                                      i18n("This is the image's color preview. "
                                           "You can pick a spot on the image "
                                           "to see the corresponding level in the histogram."));*/
    d->previewWidget = new ImagePanelWidget(470, 350, "Profile Conversion Tool", d->gboxSettings->panIconView());
    setToolView(d->previewWidget);
    init();

    // ---

    connect(d->previewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotTimer()));

    connect(currentProfInfo, SIGNAL(clicked()),
            this, SLOT(slotCurrentProfInfo()) );

    connect(newProfInfo, SIGNAL(clicked()),
            this, SLOT(slotNewProfInfo()) );

    connect(d->profilesBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotProfileChanged()));
}

ProfileConversionTool::~ProfileConversionTool()
{
    delete d;
}

QStringList ProfileConversionTool::favoriteProfiles()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Profile Conversion Tool");
    return group.readPathEntry("Recently Used Profiles", QStringList());
}

void ProfileConversionTool::fastConversion(const IccProfile& profile)
{
    ImageIface iface(0, 0);
    IccProfile currentProfile = iface.getOriginalIccProfile();
    IccTransform transform = ProfileConversionToolPriv::getTransform(currentProfile, profile);
    IccTransformFilter filter(iface.getOriginalImg(), 0, transform);
    filter.startFilterDirectly();
    DImg imDest = filter.getTargetImage();
    iface.putOriginalImage(i18n("Color Profile Conversion"), imDest.bits());
    iface.putOriginalIccProfile(imDest.getIccProfile());
}

void ProfileConversionTool::renderingFinished()
{
    d->profilesBox->setEnabled(true);
}

void ProfileConversionTool::slotCurrentProfInfo()
{
    ICCProfileInfoDlg infoDlg(d->gboxSettings, QString(), d->currentProfile);
    infoDlg.exec();
}

void ProfileConversionTool::slotNewProfInfo()
{
    ICCProfileInfoDlg infoDlg(d->gboxSettings , QString(), d->profilesBox->currentProfile());
    infoDlg.exec();
}

void ProfileConversionTool::slotProfileChanged()
{
    d->gboxSettings->enableButton(EditorToolSettings::Ok, !d->profilesBox->currentProfile().isNull());
    updateTransform();
    d->favoriteProfiles.insert(d->profilesBox->currentProfile().filePath(), 0);
    slotTimer();
}

void ProfileConversionTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Profile Conversion Tool");
    d->profilesBox->setCurrentProfile(group.readPathEntry("Profile", d->currentProfile.filePath()));
    QStringList lastProfiles = group.readPathEntry("Recently Used Profiles", QStringList());
    foreach (const QString &path, lastProfiles)
        d->favoriteProfiles.insert(path, 0);
}

void ProfileConversionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Profile Conversion Tool");
    group.writePathEntry("Profile", d->profilesBox->currentProfile().filePath());
    group.writePathEntry("Recently Used Profiles", d->favoriteProfiles.keys());
    d->previewWidget->writeSettings();
    config->sync();
}

void ProfileConversionTool::slotResetSettings()
{
    d->profilesBox->setCurrentIndex(-1);
}

void ProfileConversionTool::updateTransform()
{
    d->transform = d->getTransform(d->currentProfile, d->profilesBox->currentProfile());
}

void ProfileConversionTool::prepareEffect()
{
    DImg img = d->previewWidget->getOriginalRegionImage();
    setFilter(new IccTransformFilter(&img, this, d->transform));
}

void ProfileConversionTool::prepareFinal()
{
    ImageIface iface(0, 0);
    setFilter(new IccTransformFilter(iface.getOriginalImg(), this, d->transform));
}

void ProfileConversionTool::putPreviewData()
{
    DImg imDest = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(imDest);
}

void ProfileConversionTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg imDest = filter()->getTargetImage();

    iface.putOriginalImage(i18n("Color Profile Conversion"), imDest.bits());
    iface.putOriginalIccProfile(imDest.getIccProfile());
}

}  // namespace DigikamImagesPluginCore
