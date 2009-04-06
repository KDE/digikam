/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-30
 * Description : a digiKam image editor plugin to inpaint
 *               a photograph
 *
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

#include "inpaintingtool.h"
#include "inpaintingtool.moc"

// C++ includes

#include <cstdio>
#include <cmath>
#include <cstring>

// Qt includes

#include <QBrush>
#include <QCheckBox>
#include <QEvent>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>

// Local includes

#include "version.h"
#include "daboutdata.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "editortoolsettings.h"
#include "greycstorationsettings.h"
#include "greycstorationwidget.h"
#include "greycstorationiface.h"

using namespace Digikam;

namespace DigikamInPaintingImagesPlugin
{

InPaintingTool::InPaintingTool(QObject* parent)
              : EditorToolThreaded(parent)
{
    setObjectName("inpainting");
    setToolName(i18n("In-painting"));
    setToolIcon(SmallIcon("inpainting"));

    m_isComputed    = false;
    m_previewWidget = new ImageWidget("inpainting Tool", 0,
                                      i18n("The image selection preview with in-painting applied "
                                           "is shown here."),
                                           true, ImageGuideWidget::HVGuideMode, false, true);
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Try|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);
    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage());
    m_mainTab                 = new KTabWidget( m_gboxSettings->plainPage());

    QWidget* firstPage = new QWidget(m_mainTab);
    QGridLayout* grid  = new QGridLayout(firstPage);
    m_mainTab->addTab(firstPage, i18n("Preset"));

    KUrlLabel *cimgLogoLabel = new KUrlLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setUrl("http://cimg.sourceforge.net");
    cimgLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-cimg.png")));
    cimgLogoLabel->setToolTip(i18n("Visit CImg library website"));

    QLabel *typeLabel  = new QLabel(i18n("Filtering type:"), firstPage);
    typeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_inpaintingTypeCB = new KComboBox(firstPage);
    m_inpaintingTypeCB->addItem(i18nc("no inpainting type", "None"));
    m_inpaintingTypeCB->addItem(i18n("Remove Small Artifact"));
    m_inpaintingTypeCB->addItem(i18n("Remove Medium Artifact"));
    m_inpaintingTypeCB->addItem(i18n("Remove Large Artifact"));
    m_inpaintingTypeCB->setWhatsThis( i18n("<p>Select the filter preset to use for photograph restoration here:</p>"
                                           "<p><b>None</b>: Most common values. Puts settings to default.<br/>"
                                           "<b>Remove Small Artifact</b>: in-paint small image artifacts, such as image glitches.<br/>"
                                           "<b>Remove Medium Artifact</b>: in-paint medium image artifacts.<br/>"
                                           "<b>Remove Large Artifact</b>: in-paint large image artifacts, such as unwanted objects.</p>"));

    grid->addWidget(cimgLogoLabel,      0, 1, 1, 1);
    grid->addWidget(typeLabel,          1, 0, 1, 1);
    grid->addWidget(m_inpaintingTypeCB, 1, 1, 1, 1);
    grid->setRowStretch(1, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    // -------------------------------------------------------------

    m_settingsWidget = new GreycstorationWidget(m_mainTab);

    gridSettings->addWidget(m_mainTab,                               0, 1, 1, 1);
    gridSettings->addWidget(new QLabel(m_gboxSettings->plainPage()), 1, 1, 1, 1);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(m_gboxSettings->spacingHint());
    gridSettings->setRowStretch(1, 10);

    setToolSettings(m_gboxSettings);

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processCImgUrl(const QString&)));

    connect(m_inpaintingTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotResetValues(int)));

    // -------------------------------------------------------------

    GreycstorationSettings defaults;
    defaults.setInpaintingDefaultSettings();
    m_settingsWidget->setDefaultSettings(defaults);
    init();
}

InPaintingTool::~InPaintingTool()
{
}

void InPaintingTool::renderingFinished()
{
    m_mainTab->setEnabled(true);
}

void InPaintingTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("inpainting Tool");

    GreycstorationSettings settings;
    GreycstorationSettings defaults;
    defaults.setInpaintingDefaultSettings();

    settings.fastApprox = group.readEntry("FastApprox",    defaults.fastApprox);
    settings.interp     = group.readEntry("Interpolation", defaults.interp);
    settings.amplitude  = group.readEntry("Amplitude",     (double)defaults.amplitude);
    settings.sharpness  = group.readEntry("Sharpness",     (double)defaults.sharpness);
    settings.anisotropy = group.readEntry("Anisotropy",    (double)defaults.anisotropy);
    settings.alpha      = group.readEntry("Alpha",         (double)defaults.alpha);
    settings.sigma      = group.readEntry("Sigma",         (double)defaults.sigma);
    settings.gaussPrec  = group.readEntry("GaussPrec",     (double)defaults.gaussPrec);
    settings.dl         = group.readEntry("Dl",            (double)defaults.dl);
    settings.da         = group.readEntry("Da",            (double)defaults.da);
    settings.nbIter     = group.readEntry("Iteration",     defaults.nbIter);
    settings.tile       = group.readEntry("Tile",          defaults.tile);
    settings.btile      = group.readEntry("BTile",         defaults.btile);
    m_settingsWidget->setSettings(settings);

    int p = group.readEntry("Preset", (int)NoPreset);
    m_inpaintingTypeCB->setCurrentIndex(p);
    if (p == NoPreset)
        m_settingsWidget->setEnabled(true);
    else
        m_settingsWidget->setEnabled(false);
}

void InPaintingTool::writeSettings()
{
    GreycstorationSettings settings = m_settingsWidget->getSettings();
    KSharedConfig::Ptr config       = KGlobal::config();
    KConfigGroup group              = config->group("inpainting Tool");
    group.writeEntry("Preset",        m_inpaintingTypeCB->currentIndex());
    group.writeEntry("FastApprox",    settings.fastApprox);
    group.writeEntry("Interpolation", settings.interp);
    group.writeEntry("Amplitude",     (double)settings.amplitude);
    group.writeEntry("Sharpness",     (double)settings.sharpness);
    group.writeEntry("Anisotropy",    (double)settings.anisotropy);
    group.writeEntry("Alpha",         (double)settings.alpha);
    group.writeEntry("Sigma",         (double)settings.sigma);
    group.writeEntry("GaussPrec",     (double)settings.gaussPrec);
    group.writeEntry("Dl",            (double)settings.dl);
    group.writeEntry("Da",            (double)settings.da);
    group.writeEntry("Iteration",     settings.nbIter);
    group.writeEntry("Tile",          settings.tile);
    group.writeEntry("BTile",         settings.btile);
    m_previewWidget->writeSettings();
    config->sync();
}

void InPaintingTool::slotResetValues(int i)
{
    if (i == NoPreset)
        m_settingsWidget->setEnabled(true);
    else
        m_settingsWidget->setEnabled(false);

    slotResetSettings();
}

void InPaintingTool::slotResetSettings()
{
    GreycstorationSettings settings;
    settings.setInpaintingDefaultSettings();

    switch(m_inpaintingTypeCB->currentIndex())
    {
        case RemoveSmallArtefact:
            // We use default settings here.
            break;

        case RemoveMediumArtefact:
        {
            settings.amplitude = 50.0;
            settings.nbIter    = 50;
            break;
        }

        case RemoveLargeArtefact:
        {
            settings.amplitude = 100.0;
            settings.nbIter    = 100;
            break;
        }
    }

    m_settingsWidget->setSettings(settings);
}

void InPaintingTool::processCImgUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void InPaintingTool::prepareEffect()
{
    m_mainTab->setEnabled(false);

    ImageIface iface(0, 0);
    uchar *data     = iface.getOriginalImage();
    m_originalImage = DImg(iface.originalWidth(), iface.originalHeight(),
                           iface.originalSixteenBit(), iface.originalHasAlpha(), data);
    delete [] data;

    // Selected area from the image and mask creation:
    //
    // We optimize the computation time to use the current selected area in image editor
    // and to create an inpainting mask with it. Because inpainting is done by interpolation
    // neighbor pixels which can be located far from the selected area, we need to adjust the
    // mask size in according with the parameter algorithms, especially 'amplitude'.
    // Mask size is computed like this :
    //
    // (image_size_x + 2*amplitude , image_size_y + 2*amplitude)


    QRect selectionRect = QRect(iface.selectedXOrg(), iface.selectedYOrg(),
                                iface.selectedWidth(), iface.selectedHeight());

    QPixmap inPaintingMask(iface.originalWidth(), iface.originalHeight());
    inPaintingMask.fill(Qt::black);
    QPainter p(&inPaintingMask);
    p.fillRect( selectionRect, QBrush(Qt::white) );
    p.end();

    GreycstorationSettings settings = m_settingsWidget->getSettings();

    int x1 = (int)(selectionRect.left()   - 2*settings.amplitude);
    int y1 = (int)(selectionRect.top()    - 2*settings.amplitude);
    int x2 = (int)(selectionRect.right()  + 2*settings.amplitude);
    int y2 = (int)(selectionRect.bottom() + 2*settings.amplitude);
    m_maskRect = QRect(x1, y1, x2-x1, y2-y1);

    // Mask area normalization.
    // We need to check if mask area is out of image size else inpainting give strange results.

    if (m_maskRect.left()   < 0) m_maskRect.setLeft(0);
    if (m_maskRect.top()    < 0) m_maskRect.setTop(0);
    if (m_maskRect.right()  > iface.originalWidth())  m_maskRect.setRight(iface.originalWidth());
    if (m_maskRect.bottom() > iface.originalHeight()) m_maskRect.setBottom(iface.originalHeight());

    m_maskImage = inPaintingMask.toImage().copy(m_maskRect);
    m_cropImage = m_originalImage.copy(m_maskRect);

    setFilter(dynamic_cast<DImgThreadedFilter*>(
                       new GreycstorationIface(
                                    &m_cropImage,
                                    settings,
                                    GreycstorationIface::InPainting,
                                    0, 0,
                                    m_maskImage, this)));
}

void InPaintingTool::prepareFinal()
{
    if (!m_isComputed)
    {
        prepareEffect();
    }
    else
    {
        slotFilterFinished(true);
    }
}

void InPaintingTool::putPreviewData()
{
    ImageIface* iface               = m_previewWidget->imageIface();
    GreycstorationSettings settings = m_settingsWidget->getSettings();

    m_cropImage = filter()->getTargetImage();
    QRect cropSel((int)(2*settings.amplitude), (int)(2*settings.amplitude),
                  iface->selectedWidth(), iface->selectedHeight());
    DImg imDest = m_cropImage.copy(cropSel);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                                               iface->previewHeight())).bits());
    m_previewWidget->updatePreview();
    m_isComputed = true;
}

void InPaintingTool::putFinalData()
{
    ImageIface iface(0, 0);

    if (!m_isComputed)
        m_cropImage = filter()->getTargetImage();

    m_originalImage.bitBltImage(&m_cropImage, m_maskRect.left(), m_maskRect.top());

    iface.putOriginalImage(i18n("In-Painting"), m_originalImage.bits());
}

void InPaintingTool::slotLoadSettings()
{
    KUrl loadInpaintingFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Photograph In-Painting Settings File to Load")) );
    if( loadInpaintingFile.isEmpty() )
       return;

    QFile file(loadInpaintingFile.path());

    if ( file.open(QIODevice::ReadOnly) )
    {
        if (!m_settingsWidget->loadSettings(file, QString("# Photograph Inpainting Configuration File V2")))
        {
           KMessageBox::error(kapp->activeWindow(),
                        i18n("\"%1\" is not a Photograph In-Painting settings text file.",
                             loadInpaintingFile.fileName()));
           file.close();
           return;
        }
    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph In-Painting text file."));

    file.close();
    m_inpaintingTypeCB->blockSignals(true);
    m_inpaintingTypeCB->setCurrentIndex(NoPreset);
    m_inpaintingTypeCB->blockSignals(false);
    m_settingsWidget->setEnabled(true);
}

void InPaintingTool::slotSaveAsSettings()
{
    KUrl saveRestorationFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Photograph In-Painting Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());

    if ( file.open(QIODevice::WriteOnly) )
        m_settingsWidget->saveSettings(file, QString("# Photograph Inpainting Configuration File V2"));
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph In-Painting text file."));

    file.close();
}

}  // namespace DigikamInPaintingImagesPlugin
