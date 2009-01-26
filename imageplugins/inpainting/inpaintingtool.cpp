/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-30
 * Description : a digiKam image editor plugin to inpaint
 *               a photograph
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes.

#include <cstdio>
#include <cmath>
#include <cstring>

// Qt includes.

#include <qbrush.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qfile.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvgroupbox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kprogress.h>
#include <kstandarddirs.h>
#include <kurllabel.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "editortoolsettings.h"
#include "greycstorationsettings.h"
#include "greycstorationwidget.h"
#include "greycstorationiface.h"
#include "inpaintingtool.h"
#include "inpaintingtool.moc"

using namespace Digikam;

namespace DigikamInPaintingImagesPlugin
{

InPaintingTool::InPaintingTool(QObject* parent)
              : EditorToolThreaded(parent)
{
    setName("inpainting");
    setToolName(i18n("Inpainting"));
    setToolIcon(SmallIcon("inpainting"));

    m_isComputed = false;

    m_previewWidget = new ImageWidget("inpainting Tool", 0,
                                      i18n("<p>Here you can see the image selection preview with "
                                           "inpainting applied."),
                                           true, ImageGuideWidget::HVGuideMode, false, true);
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Try|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);
    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage(), 2, 1);
    m_mainTab = new QTabWidget( m_gboxSettings->plainPage() );

    QWidget* firstPage = new QWidget( m_mainTab );
    QGridLayout* grid  = new QGridLayout( firstPage, 2, 2);
    m_mainTab->addTab( firstPage, i18n("Preset") );

    KURLLabel *cimgLogoLabel = new KURLLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setURL("http://cimg.sourceforge.net");
    KGlobal::dirs()->addResourceType("logo-cimg", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("logo-cimg", "logo-cimg.png");
    cimgLogoLabel->setPixmap( QPixmap( directory + "logo-cimg.png" ) );
    QToolTip::add(cimgLogoLabel, i18n("Visit CImg library website"));

    QLabel *typeLabel = new QLabel(i18n("Filtering type:"), firstPage);
    typeLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_inpaintingTypeCB = new QComboBox( false, firstPage );
    m_inpaintingTypeCB->insertItem( i18n("None") );
    m_inpaintingTypeCB->insertItem( i18n("Remove Small Artefact") );
    m_inpaintingTypeCB->insertItem( i18n("Remove Medium Artefact") );
    m_inpaintingTypeCB->insertItem( i18n("Remove Large Artefact") );
    QWhatsThis::add( m_inpaintingTypeCB, i18n("<p>Select the filter preset to use for photograph restoration:<p>"
                                               "<b>None</b>: Most common values. Puts settings to default.<p>"
                                               "<b>Remove Small Artefact</b>: inpaint small image artefact like image glitch.<p>"
                                               "<b>Remove Medium Artefact</b>: inpaint medium image artefact.<p>"
                                               "<b>Remove Large Artefact</b>: inpaint image artefact like unwanted object.<p>"));

    grid->addMultiCellWidget(cimgLogoLabel,      0, 0, 1, 1);
    grid->addMultiCellWidget(typeLabel,          1, 1, 0, 0);
    grid->addMultiCellWidget(m_inpaintingTypeCB, 1, 1, 1, 1);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());
    grid->setRowStretch(1, 10);

    // -------------------------------------------------------------

    m_settingsWidget = new GreycstorationWidget(m_mainTab);

    gridSettings->addMultiCellWidget(m_mainTab,                               0, 0, 1, 1);
    gridSettings->addMultiCellWidget(new QLabel(m_gboxSettings->plainPage()), 1, 1, 1, 1);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(m_gboxSettings->spacingHint());
    gridSettings->setRowStretch(1, 10);

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processCImgURL(const QString&)));

    connect(m_inpaintingTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotResetValues(int)));

    // -------------------------------------------------------------

    GreycstorationSettings defaults;
    defaults.setInpaintingDefaultSettings();
    m_settingsWidget->setDefaultSettings(defaults);
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
    KConfig* config = kapp->config();
    config->setGroup("inpainting Tool");

    GreycstorationSettings settings;
    GreycstorationSettings defaults;
    defaults.setInpaintingDefaultSettings();

    settings.fastApprox = config->readBoolEntry("FastApprox", defaults.fastApprox);
    settings.interp     = config->readNumEntry("Interpolation", defaults.interp);
    settings.amplitude  = config->readDoubleNumEntry("Amplitude", defaults.amplitude);
    settings.sharpness  = config->readDoubleNumEntry("Sharpness", defaults.sharpness);
    settings.anisotropy = config->readDoubleNumEntry("Anisotropy", defaults.anisotropy);
    settings.alpha      = config->readDoubleNumEntry("Alpha", defaults.alpha);
    settings.sigma      = config->readDoubleNumEntry("Sigma", defaults.sigma);
    settings.gaussPrec  = config->readDoubleNumEntry("GaussPrec", defaults.gaussPrec);
    settings.dl         = config->readDoubleNumEntry("Dl", defaults.dl);
    settings.da         = config->readDoubleNumEntry("Da", defaults.da);
    settings.nbIter     = config->readNumEntry("Iteration", defaults.nbIter);
    settings.tile       = config->readNumEntry("Tile", defaults.tile);
    settings.btile      = config->readNumEntry("BTile", defaults.btile);
    m_settingsWidget->setSettings(settings);

    int p = config->readNumEntry("Preset", NoPreset);
    m_inpaintingTypeCB->setCurrentItem(p);
    if (p == NoPreset)
        m_settingsWidget->setEnabled(true);
    else
        m_settingsWidget->setEnabled(false);
}

void InPaintingTool::writeSettings()
{
    GreycstorationSettings settings = m_settingsWidget->getSettings();
    KConfig* config = kapp->config();
    config->setGroup("inpainting Tool");
    config->writeEntry("Preset", m_inpaintingTypeCB->currentItem());
    config->writeEntry("FastApprox", settings.fastApprox);
    config->writeEntry("Interpolation", settings.interp);
    config->writeEntry("Amplitude", settings.amplitude);
    config->writeEntry("Sharpness", settings.sharpness);
    config->writeEntry("Anisotropy", settings.anisotropy);
    config->writeEntry("Alpha", settings.alpha);
    config->writeEntry("Sigma", settings.sigma);
    config->writeEntry("GaussPrec", settings.gaussPrec);
    config->writeEntry("Dl", settings.dl);
    config->writeEntry("Da", settings.da);
    config->writeEntry("Iteration", settings.nbIter);
    config->writeEntry("Tile", settings.tile);
    config->writeEntry("BTile", settings.btile);
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

    switch(m_inpaintingTypeCB->currentItem())
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

void InPaintingTool::processCImgURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
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
    // neighboor pixels which can be located far from the selected area, we need to ajust the
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

    m_maskImage = inPaintingMask.convertToImage().copy(m_maskRect);
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
//FIXME        setProgressVisible(true);
        prepareEffect();
    }
    else
    {
        putFinalData();
        kapp->restoreOverrideCursor();
//FIXME        accept();
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

    iface.putOriginalImage(i18n("InPainting"), m_originalImage.bits());
}

void InPaintingTool::slotLoadSettings()
{
    KURL loadInpaintingFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Photograph Inpainting Settings File to Load")) );
    if( loadInpaintingFile.isEmpty() )
       return;

    QFile file(loadInpaintingFile.path());

    if ( file.open(IO_ReadOnly) )
    {
        if (!m_settingsWidget->loadSettings(file, QString("# Photograph Inpainting Configuration File V2")))
        {
           KMessageBox::error(kapp->activeWindow(),
                        i18n("\"%1\" is not a Photograph Inpainting settings text file.")
                        .arg(loadInpaintingFile.fileName()));
           file.close();
           return;
        }
    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph Inpainting text file."));

    file.close();
    m_inpaintingTypeCB->blockSignals(true);
    m_inpaintingTypeCB->setCurrentItem(NoPreset);
    m_inpaintingTypeCB->blockSignals(false);
    m_settingsWidget->setEnabled(true);
}

void InPaintingTool::slotSaveAsSettings()
{
    KURL saveRestorationFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Photograph Inpainting Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());

    if ( file.open(IO_WriteOnly) )
        m_settingsWidget->saveSettings(file, QString("# Photograph Inpainting Configuration File V2"));
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Inpainting text file."));

    file.close();
}

}  // NameSpace DigikamInPaintingImagesPlugin
