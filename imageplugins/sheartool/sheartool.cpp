/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-23
 * Description : a plugin to shear an image
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "sheartool.h"
#include "sheartool.moc"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QImage>
#include <QLabel>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "daboutdata.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "shear.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamShearToolImagesPlugin
{

class ShearToolPriv
{
public:

    ShearToolPriv()
    {
        newWidthLabel   = 0;
        newHeightLabel  = 0;
        antialiasInput  = 0;
        mainHAngleInput = 0;
        mainVAngleInput = 0;
        fineHAngleInput = 0;
        fineVAngleInput = 0;
        previewWidget   = 0;
        gboxSettings    = 0;
    }

    QLabel*              newWidthLabel;
    QLabel*              newHeightLabel;

    QCheckBox*           antialiasInput;

    RIntNumInput*        mainHAngleInput;
    RIntNumInput*        mainVAngleInput;

    RDoubleNumInput* 	 fineHAngleInput;
    RDoubleNumInput* 	 fineVAngleInput;

    ImageWidget*         previewWidget;
    EditorToolSettings*  gboxSettings;
};

ShearTool::ShearTool(QObject* parent)
         : EditorToolThreaded(parent),
           d(new ShearToolPriv)
{
    setObjectName("sheartool");
    setToolName(i18n("Shear Tool"));
    setToolIcon(SmallIcon("shear"));

    d->previewWidget = new ImageWidget("sheartool Tool", 0,
                                      i18n("This is the shear operation preview. "
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be drawn "
                                           "to guide you in adjusting the shear correction. "
                                           "Release the left mouse button to freeze the dashed "
                                           "line's position."),
                                      false, ImageGuideWidget::HVGuideMode);

    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    QString temp;
    Digikam::ImageIface iface(0, 0);

    d->gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                             EditorToolSettings::Ok|
                                             EditorToolSettings::Cancel,
                                             EditorToolSettings::ColorGuide);

    // -------------------------------------------------------------

    QLabel *label1   = new QLabel(i18n("New width:"));
    d->newWidthLabel = new QLabel(temp.setNum(iface.originalWidth()) + i18n(" px"));
    d->newWidthLabel->setAlignment(Qt::AlignBottom | Qt::AlignRight);

    QLabel *label2    = new QLabel(i18n("New height:"));
    d->newHeightLabel = new QLabel(temp.setNum(iface.originalHeight()) + i18n(" px"));
    d->newHeightLabel->setAlignment(Qt::AlignBottom | Qt::AlignRight);

    QLabel *label3     = new QLabel(i18n("Main horizontal angle:"));
    d->mainHAngleInput = new RIntNumInput;
    d->mainHAngleInput->setRange(-45, 45, 1);
    d->mainHAngleInput->setSliderEnabled(true);
    d->mainHAngleInput->setDefaultValue(0);
    d->mainHAngleInput->setWhatsThis( i18n("The main horizontal shearing angle, in degrees."));

    QLabel *label4     = new QLabel(i18n("Fine horizontal angle:"));
    d->fineHAngleInput = new RDoubleNumInput;
    d->fineHAngleInput->input()->setRange(-1.0, 1.0, 0.01, true);
    d->fineHAngleInput->setDefaultValue(0);
    d->fineHAngleInput->setWhatsThis( i18n("This value in degrees will be added to main "
                                           "horizontal angle value to set fine adjustments."));
    QLabel *label5     = new QLabel(i18n("Main vertical angle:"));
    d->mainVAngleInput = new RIntNumInput;
    d->mainVAngleInput->setRange(-45, 45, 1);
    d->mainVAngleInput->setSliderEnabled(true);
    d->mainVAngleInput->setDefaultValue(0);
    d->mainVAngleInput->setWhatsThis( i18n("The main vertical shearing angle, in degrees."));

    QLabel *label6     = new QLabel(i18n("Fine vertical angle:"));
    d->fineVAngleInput = new RDoubleNumInput;
    d->fineVAngleInput->input()->setRange(-1.0, 1.0, 0.01, true);
    d->fineVAngleInput->setDefaultValue(0);
    d->fineVAngleInput->setWhatsThis( i18n("This value in degrees will be added to main vertical "
                                           "angle value to set fine adjustments."));

    d->antialiasInput = new QCheckBox(i18n("Anti-Aliasing"));
    d->antialiasInput->setWhatsThis( i18n("Enable this option to apply the anti-aliasing filter "
                                          "to the sheared image. "
                                          "To smooth the target image, it will be blurred a little."));

    KSeparator *line = new KSeparator(Qt::Horizontal);

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->setSpacing(0);
    mainLayout->addWidget(label1,              0, 0, 1, 1);
    mainLayout->addWidget(d->newWidthLabel,    0, 1, 1, 2);
    mainLayout->addWidget(label2,              1, 0, 1, 1);
    mainLayout->addWidget(d->newHeightLabel,   1, 1, 1, 2);
    mainLayout->addWidget(line,                2, 0, 1, 3);
    mainLayout->addWidget(label3,              3, 0, 1, 3);
    mainLayout->addWidget(d->mainHAngleInput,  4, 0, 1, 3);
    mainLayout->addWidget(label4,              5, 0, 1, 3);
    mainLayout->addWidget(d->fineHAngleInput,  6, 0, 1, 3);
    mainLayout->addWidget(label5,              7, 0, 1, 1);
    mainLayout->addWidget(d->mainVAngleInput,  8, 0, 1, 3);
    mainLayout->addWidget(label6,              9, 0, 1, 3);
    mainLayout->addWidget(d->fineVAngleInput, 10, 0, 1, 3);
    mainLayout->addWidget(d->antialiasInput,  11, 0, 1, 3);
    mainLayout->setRowStretch(12, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->mainHAngleInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->fineHAngleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(d->mainVAngleInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->fineVAngleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(d->antialiasInput, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));

    connect(d->gboxSettings, SIGNAL(signalColorGuideChanged()),
            this, SLOT(slotColorGuideChanged()));
}

ShearTool::~ShearTool()
{
    delete d;
}

void ShearTool::slotColorGuideChanged()
{
    d->previewWidget->slotChangeGuideColor(d->gboxSettings->guideColor());
    d->previewWidget->slotChangeGuideSize(d->gboxSettings->guideSize());
}

void ShearTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("sheartool Tool");
    d->mainHAngleInput->setValue(group.readEntry("Main HAngle", d->mainHAngleInput->defaultValue()));
    d->mainVAngleInput->setValue(group.readEntry("Main VAngle", d->mainVAngleInput->defaultValue()));
    d->fineHAngleInput->setValue(group.readEntry("Fine HAngle", d->fineHAngleInput->defaultValue()));
    d->fineVAngleInput->setValue(group.readEntry("Fine VAngle", d->fineVAngleInput->defaultValue()));
    d->antialiasInput->setChecked(group.readEntry("Anti Aliasing", true));
    slotEffect();
}

void ShearTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("sheartool Tool");
    group.writeEntry("Main HAngle", d->mainHAngleInput->value());
    group.writeEntry("Main VAngle", d->mainVAngleInput->value());
    group.writeEntry("Fine HAngle", d->fineHAngleInput->value());
    group.writeEntry("Fine VAngle", d->fineVAngleInput->value());
    group.writeEntry("Anti Aliasing", d->antialiasInput->isChecked());
    d->previewWidget->writeSettings();
    config->sync();
}

void ShearTool::slotResetSettings()
{
    d->mainHAngleInput->blockSignals(true);
    d->mainVAngleInput->blockSignals(true);
    d->fineHAngleInput->blockSignals(true);
    d->fineVAngleInput->blockSignals(true);
    d->antialiasInput->blockSignals(true);

    d->mainHAngleInput->slotReset();
    d->mainVAngleInput->slotReset();
    d->fineHAngleInput->slotReset();
    d->fineVAngleInput->slotReset();
    d->antialiasInput->setChecked(true);

    d->mainHAngleInput->blockSignals(false);
    d->mainVAngleInput->blockSignals(false);
    d->fineHAngleInput->blockSignals(false);
    d->fineVAngleInput->blockSignals(false);
    d->antialiasInput->blockSignals(false);

    slotEffect();
}

void ShearTool::prepareEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    d->mainHAngleInput->setEnabled(false);
    d->mainVAngleInput->setEnabled(false);
    d->fineHAngleInput->setEnabled(false);
    d->fineVAngleInput->setEnabled(false);
    d->antialiasInput->setEnabled(false);

    float hAngle      = d->mainHAngleInput->value() + d->fineHAngleInput->value();
    float vAngle      = d->mainVAngleInput->value() + d->fineVAngleInput->value();
    bool antialiasing = d->antialiasInput->isChecked();
    QColor background = Qt::black;

    ImageIface* iface = d->previewWidget->imageIface();
    int orgW          = iface->originalWidth();
    int orgH          = iface->originalHeight();

    uchar *data       = iface->getPreviewImage();
    DImg image(iface->previewWidth(), iface->previewHeight(), iface->previewSixteenBit(),
                        iface->previewHasAlpha(), data);
    delete [] data;

    setFilter(dynamic_cast<DImgThreadedFilter *>(new Shear(&image, this, hAngle, vAngle, antialiasing,
                                                           background, orgW, orgH)));
}

void ShearTool::prepareFinal()
{
    d->mainHAngleInput->setEnabled(false);
    d->mainVAngleInput->setEnabled(false);
    d->fineHAngleInput->setEnabled(false);
    d->fineVAngleInput->setEnabled(false);
    d->antialiasInput->setEnabled(false);

    float hAngle      = d->mainHAngleInput->value() + d->fineHAngleInput->value();
    float vAngle      = d->mainVAngleInput->value() + d->fineVAngleInput->value();
    bool antialiasing = d->antialiasInput->isChecked();
    QColor background = Qt::black;

    ImageIface iface(0, 0);
    int orgW = iface.originalWidth();
    int orgH = iface.originalHeight();

    uchar *data = iface.getOriginalImage();
    DImg orgImage(orgW, orgH, iface.originalSixteenBit(),
                           iface.originalHasAlpha(), data);
    delete [] data;

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new Shear(&orgImage, this, hAngle, vAngle, antialiasing, background, orgW, orgH)));
}

void ShearTool::putPreviewData()
{
    ImageIface* iface = d->previewWidget->imageIface();
    int w = iface->previewWidth();
    int h = iface->previewHeight();

    DImg imTemp = filter()->getTargetImage().smoothScale(w, h, Qt::KeepAspectRatio);
    DImg imDest( w, h, filter()->getTargetImage().sixteenBit(),
                                filter()->getTargetImage().hasAlpha() );

    imDest.fill( DColor(d->previewWidget->palette().color(QPalette::Background).rgb(),
                                 filter()->getTargetImage().sixteenBit()) );
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                                               iface->previewHeight())).bits());

    d->previewWidget->updatePreview();
    QSize newSize = dynamic_cast<Shear*>(filter())->getNewSize();
    QString temp;
    d->newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    d->newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
}

void ShearTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg targetImage = filter()->getTargetImage();
    iface.putOriginalImage(i18n("Shear Tool"),
                           targetImage.bits(),
                           targetImage.width(), targetImage.height());
}

void ShearTool::renderingFinished()
{
    d->mainHAngleInput->setEnabled(true);
    d->mainVAngleInput->setEnabled(true);
    d->fineHAngleInput->setEnabled(true);
    d->fineVAngleInput->setEnabled(true);
    d->antialiasInput->setEnabled(true);
    kapp->restoreOverrideCursor();
}

}  // namespace DigikamShearToolImagesPlugin
