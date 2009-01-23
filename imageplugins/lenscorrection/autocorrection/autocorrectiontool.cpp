/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "autocorrectiontool.h"
#include "autocorrectiontool.moc"

// Qt includes.

#include <QBitmap>
#include <QBrush>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QTimer>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// Local includes.

#include "daboutdata.h"
#include "dmetadata.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "klensfun.h"
#include "version.h"

using namespace Digikam;

namespace DigikamAutoCorrectionImagesPlugin
{

AutoCorrectionTool::AutoCorrectionTool(QObject* parent)
                  : EditorToolThreaded(parent)
{
    setObjectName("lensautocorrection");
    setToolName(i18n("Lens Auto-Correction"));
    setToolIcon(SmallIcon("lensdistortion"));

    m_previewWidget = new ImageWidget("antivignetting Tool", 0, QString(),
                                      true, ImageGuideWidget::HVGuideMode, true);

    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);

    QGridLayout *grid = new QGridLayout(m_gboxSettings->plainPage());
    m_cameraSelector  = new KLFDeviceSelector(m_gboxSettings->plainPage());
    KSeparator *line  = new KSeparator(Qt::Horizontal, m_gboxSettings->plainPage());

    // -------------------------------------------------------------

    m_showGrid   = new QCheckBox(i18n("Show grid"), m_gboxSettings->plainPage());
    m_showGrid->setWhatsThis(i18n("Set this option to visualize correction grid to be applied."));

    m_filterCCA  = new QCheckBox(i18n("Chromatic Aberration"), m_gboxSettings->plainPage());
    m_filterCCA->setWhatsThis(i18n("Chromatic aberration is easily recognized as color fringes "
                                   "towards the image corners. CA is due to a varying lens focus "
                                   "for different colors."));
    m_filterVig  = new QCheckBox(i18n("Vignetting"), m_gboxSettings->plainPage());
    m_filterVig->setWhatsThis(i18n("Vignetting refers to an image darkening, mostly in the corners. "
                                   "Optical and natural vignetting can be canceled out with this option, "
                                   "whereas mechanical vignetting will not be cured."));
    m_filterCCI  = new QCheckBox(i18n("Color Correction"), m_gboxSettings->plainPage());
    m_filterCCI->setWhatsThis(i18n("All lenses have a slight color tinge to them, "
                                   "mostly due to the anti-reflective coating. "
                                   "The tinge can be canceled when the respective data is known for the lens."));
    m_filterDist = new QCheckBox(i18n("Distortion"), m_gboxSettings->plainPage());
    m_filterDist->setWhatsThis(i18n("Distortion refers to an image deformation, which is most pronounced "
                                    "towards the corners. These Seidel aberrations are known as pincushion "
                                    "and barrel distortions."));
    m_filterGeom = new QCheckBox(i18n("Geometry"), m_gboxSettings->plainPage());
    m_filterGeom->setWhatsThis(i18n("Four geometries are handled here: Rectilinear (99 percent of all lenses), "
                                    "Fisheye, Cylindrical, Equirectangular."));

    grid->addWidget(m_showGrid,       0, 0, 1, 2);
    grid->addWidget(m_cameraSelector, 1, 0, 1, 2);
    grid->addWidget(line,             2, 0, 1, 2);
    grid->addWidget(m_filterCCA,      3, 0, 1, 2);
    grid->addWidget(m_filterVig,      4, 0, 1, 2);
    grid->addWidget(m_filterCCI,      5, 0, 1, 2);
    grid->addWidget(m_filterDist,     6, 0, 1, 2);
    grid->addWidget(m_filterGeom,     7, 0, 1, 2);
    grid->setRowStretch(8, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_cameraSelector, SIGNAL(signalLensSettingsChanged()),
            this, SLOT(slotLensChanged()));

    connect(m_showGrid, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetFilters()));

    connect(m_filterCCA, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetFilters()));

    connect(m_filterVig, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetFilters()));

    connect(m_filterCCI, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetFilters()));

    connect(m_filterDist, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetFilters()));

    connect(m_filterGeom, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetFilters()));

    QTimer::singleShot(0, this, SLOT(slotResetSettings()));
}

AutoCorrectionTool::~AutoCorrectionTool()
{
}

void AutoCorrectionTool::slotLensChanged()
{
    m_filterCCA->setEnabled(m_cameraSelector->getKLFObject()->supportsCCA());
    m_filterVig->setEnabled(m_cameraSelector->getKLFObject()->supportsVig());
    m_filterCCI->setEnabled(m_cameraSelector->getKLFObject()->supportsVig());
    m_filterDist->setEnabled(m_cameraSelector->getKLFObject()->supportsDistortion());
    m_filterGeom->setEnabled(m_cameraSelector->getKLFObject()->supportsDistortion());
    slotSetFilters();
}

void AutoCorrectionTool::slotSetFilters()
{
    m_cameraSelector->getKLFObject()->setCorrection(
        (m_filterCCA->checkState()  == Qt::Checked && m_filterCCA->isEnabled())  ? true : false,
        (m_filterVig->checkState()  == Qt::Checked && m_filterVig->isEnabled())  ? true : false,
        (m_filterCCI->checkState()  == Qt::Checked && m_filterCCI->isEnabled())  ? true : false,
        (m_filterDist->checkState() == Qt::Checked && m_filterDist->isEnabled()) ? true : false,
        (m_filterGeom->checkState() == Qt::Checked && m_filterGeom->isEnabled()) ? true : false
     );

    slotTimer();
}

void AutoCorrectionTool::readSettings()
{
    m_gboxSettings->blockSignals(true);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Lens Auto-Correction Tool");

    m_filterCCA->setCheckState(group.readEntry("CCA", true)         ? Qt::Checked : Qt::Unchecked);
    m_filterVig->setCheckState(group.readEntry("Vignetting", true)  ? Qt::Checked : Qt::Unchecked);
    m_filterCCI->setCheckState(group.readEntry("CCI", true)         ? Qt::Checked : Qt::Unchecked);
    m_filterDist->setCheckState(group.readEntry("Distortion", true) ? Qt::Checked : Qt::Unchecked);
    m_filterGeom->setCheckState(group.readEntry("Geometry", true)   ? Qt::Checked : Qt::Unchecked);

    m_gboxSettings->blockSignals(false);
    slotSetFilters();
}

void AutoCorrectionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Lens Auto-Correction Tool");
    if ( m_filterCCA->isEnabled() )
        group.writeEntry("CCA",        (m_filterCCA->checkState() == Qt::Checked)  ? true : false);
    if ( m_filterVig->isEnabled() )
        group.writeEntry("Vignetting", (m_filterVig->checkState() == Qt::Checked)  ? true : false);
    if ( m_filterCCI->isEnabled() )
        group.writeEntry("CCI",        (m_filterCCI->checkState() == Qt::Checked)  ? true : false);
    if ( m_filterDist->isEnabled() )
        group.writeEntry("Distortion", (m_filterDist->checkState() == Qt::Checked) ? true : false);
    if ( m_filterGeom->isEnabled() )
        group.writeEntry("Geometry",   (m_filterGeom->checkState() == Qt::Checked) ? true : false);
    group.sync();
}

void AutoCorrectionTool::slotResetSettings()
{
    m_gboxSettings->blockSignals(true);

    // Read Exif information ...
    DImg      *img = m_previewWidget->imageIface()->getOriginalImg();
    DMetadata  meta;
    meta.setExif(img->getExif());
    meta.setIptc(img->getIptc());
    meta.setXmp(img->getXmp());
    m_cameraSelector->findFromMetadata(meta);

    m_gboxSettings->blockSignals(false);
}

void AutoCorrectionTool::prepareEffect()
{
    m_gboxSettings->setEnabled(false);

    ImageIface* iface          = m_previewWidget->imageIface();
    uchar *data                = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool sb                    = iface->previewSixteenBit();

    DImg image(w, h, sb, false, data);

    if (m_showGrid->isChecked())
    {
        QBitmap pattern(9, 9);
        pattern.clear();
        QPainter p1(&pattern);
        p1.setPen(QPen(Qt::black, 1));
        p1.drawLine(5, 0, 5, 9);
        p1.drawLine(0, 5, 9, 5);
        p1.end();

        QPixmap pix(w, h);
        pix.fill(Qt::transparent);
        QPainter p2(&pix);
        p2.setPen(QPen(Qt::gray, 1));
        p2.fillRect(0, 0, pix.width(), pix.height(), QBrush(pattern));
        p2.end();
        DImg grid(pix.toImage());

        DColorComposer *composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);
        DColorComposer::MultiplicationFlags flags = DColorComposer::NoMultiplication;

        // Do alpha blending of template on dest image
        image.bitBlendImage(composer, &grid, 0, 0, w, h, 0, 0, flags);
    }

    setFilter(dynamic_cast<DImgThreadedFilter *>
                       (new KLensFunFilter(&image, this, m_cameraSelector->getKLFObject())));

}

void AutoCorrectionTool::prepareFinal()
{
    m_gboxSettings->setEnabled(false);

    ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    DImg originalImage(iface.originalWidth(), iface.originalHeight(),
                                iface.originalSixteenBit(), iface.originalHasAlpha(), data);

    setFilter(dynamic_cast<DImgThreadedFilter *>
                       (new KLensFunFilter( &originalImage, this, m_cameraSelector->getKLFObject())));

    delete [] data;
}

void AutoCorrectionTool::putPreviewData()
{
    DImg imDest = filter()->getTargetImage();
    m_previewWidget->imageIface()->putPreviewImage(imDest.bits());
    m_previewWidget->updatePreview();
}

void AutoCorrectionTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Lens Auto-Correction"), filter()->getTargetImage().bits());
}

void AutoCorrectionTool::renderingFinished()
{
    m_gboxSettings->setEnabled(true);
}

}  // namespace DigikamAutoCorrectionImagesPlugin
