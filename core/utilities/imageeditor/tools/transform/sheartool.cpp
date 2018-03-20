/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-23
 * Description : a tool to shear an image
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dnuminput.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "shearfilter.h"
#include "dexpanderbox.h"

namespace Digikam
{

class ShearTool::Private
{
public:

    Private() :
        newWidthLabel(0),
        newHeightLabel(0),
        antialiasInput(0),
        mainHAngleInput(0),
        mainVAngleInput(0),
        fineHAngleInput(0),
        fineVAngleInput(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configAntiAliasingEntry;
    static const QString configMainHAngleEntry;
    static const QString configMainVAngleEntry;
    static const QString configFineHAngleEntry;
    static const QString configFineVAngleEntry;

    QLabel*              newWidthLabel;
    QLabel*              newHeightLabel;

    QCheckBox*           antialiasInput;

    DIntNumInput*        mainHAngleInput;
    DIntNumInput*        mainVAngleInput;

    DDoubleNumInput*     fineHAngleInput;
    DDoubleNumInput*     fineVAngleInput;

    ImageGuideWidget*    previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString ShearTool::Private::configGroupName(QLatin1String("shear Tool"));
const QString ShearTool::Private::configAntiAliasingEntry(QLatin1String("Anti Aliasing"));
const QString ShearTool::Private::configMainHAngleEntry(QLatin1String("Main HAngle"));
const QString ShearTool::Private::configMainVAngleEntry(QLatin1String("Main VAngle"));
const QString ShearTool::Private::configFineHAngleEntry(QLatin1String("Fine HAngle"));
const QString ShearTool::Private::configFineVAngleEntry(QLatin1String("Fine VAngle"));

// --------------------------------------------------------

ShearTool::ShearTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("sheartool"));
    setToolName(i18n("Shear Tool"));
    setToolIcon(QIcon::fromTheme(QLatin1String("transform-shear-left")));

    d->previewWidget = new ImageGuideWidget(0, true, ImageGuideWidget::HVGuideMode);
    d->previewWidget->setWhatsThis(i18n("This is the shear operation preview. "
                                        "If you move the mouse cursor on this preview, "
                                        "a vertical and horizontal dashed line will be drawn "
                                        "to guide you in adjusting the shear correction. "
                                        "Release the left mouse button to freeze the dashed "
                                        "line's position."));

    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);

    // -------------------------------------------------------------

    QString temp;
    ImageIface iface;

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::ColorGuide);

    // -------------------------------------------------------------

    QLabel* label1   = new QLabel(i18n("New width:"));
    d->newWidthLabel = new QLabel(temp.setNum(iface.originalSize().width()) + i18n(" px"));
    d->newWidthLabel->setAlignment(Qt::AlignBottom | Qt::AlignRight);

    QLabel* label2    = new QLabel(i18n("New height:"));
    d->newHeightLabel = new QLabel(temp.setNum(iface.originalSize().height()) + i18n(" px"));
    d->newHeightLabel->setAlignment(Qt::AlignBottom | Qt::AlignRight);

    QLabel* label3     = new QLabel(i18n("Main horizontal angle:"));
    d->mainHAngleInput = new DIntNumInput;
    d->mainHAngleInput->setRange(-45, 45, 1);
    d->mainHAngleInput->setDefaultValue(0);
    d->mainHAngleInput->setWhatsThis( i18n("The main horizontal shearing angle, in degrees."));

    QLabel* label4     = new QLabel(i18n("Fine horizontal angle:"));
    d->fineHAngleInput = new DDoubleNumInput;
    d->fineHAngleInput->setRange(-1.0, 1.0, 0.01);
    d->fineHAngleInput->setDefaultValue(0);
    d->fineHAngleInput->setWhatsThis( i18n("This value in degrees will be added to main "
                                           "horizontal angle value to set fine adjustments."));
    QLabel* label5     = new QLabel(i18n("Main vertical angle:"));
    d->mainVAngleInput = new DIntNumInput;
    d->mainVAngleInput->setRange(-45, 45, 1);
    d->mainVAngleInput->setDefaultValue(0);
    d->mainVAngleInput->setWhatsThis( i18n("The main vertical shearing angle, in degrees."));

    QLabel* label6     = new QLabel(i18n("Fine vertical angle:"));
    d->fineVAngleInput = new DDoubleNumInput;
    d->fineVAngleInput->setRange(-1.0, 1.0, 0.01);
    d->fineVAngleInput->setDefaultValue(0);
    d->fineVAngleInput->setWhatsThis( i18n("This value in degrees will be added to main vertical "
                                           "angle value to set fine adjustments."));

    d->antialiasInput = new QCheckBox(i18n("Anti-Aliasing"));
    d->antialiasInput->setWhatsThis( i18n("Enable this option to apply the anti-aliasing filter "
                                          "to the sheared image. "
                                          "To smooth the target image, it will be blurred a little."));

    DLineWidget* line = new DLineWidget(Qt::Horizontal);

    // -------------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    QGridLayout* grid = new QGridLayout;
    grid->setSpacing(0);
    grid->addWidget(label1,              0, 0, 1, 1);
    grid->addWidget(d->newWidthLabel,    0, 1, 1, 2);
    grid->addWidget(label2,              1, 0, 1, 1);
    grid->addWidget(d->newHeightLabel,   1, 1, 1, 2);
    grid->addWidget(line,                2, 0, 1, 3);
    grid->addWidget(label3,              3, 0, 1, 3);
    grid->addWidget(d->mainHAngleInput,  4, 0, 1, 3);
    grid->addWidget(label4,              5, 0, 1, 3);
    grid->addWidget(d->fineHAngleInput,  6, 0, 1, 3);
    grid->addWidget(label5,              7, 0, 1, 1);
    grid->addWidget(d->mainVAngleInput,  8, 0, 1, 3);
    grid->addWidget(label6,              9, 0, 1, 3);
    grid->addWidget(d->fineVAngleInput, 10, 0, 1, 3);
    grid->addWidget(d->antialiasInput,  11, 0, 1, 3);
    grid->setRowStretch(12, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(grid);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->mainHAngleInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->fineHAngleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(d->mainVAngleInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->fineVAngleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(d->antialiasInput, SIGNAL(toggled(bool)),
            this, SLOT(slotPreview()));

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
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    //    d->mainHAngleInput->setValue(group.readEntry(d->configMainHAngleEntry, d->mainHAngleInput->defaultValue()));
    //    d->mainVAngleInput->setValue(group.readEntry(d->configMainVAngleEntry, d->mainVAngleInput->defaultValue()));
    //    d->fineHAngleInput->setValue(group.readEntry(d->configFineHAngleEntry, d->fineHAngleInput->defaultValue()));
    //    d->fineVAngleInput->setValue(group.readEntry(d->configFineVAngleEntry, d->fineVAngleInput->defaultValue()));
    d->antialiasInput->setChecked(group.readEntry(d->configAntiAliasingEntry, true));
    slotPreview();
}

void ShearTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    //    group.writeEntry(d->configMainHAngleEntry, d->mainHAngleInput->value());
    //    group.writeEntry(d->configMainVAngleEntry, d->mainVAngleInput->value());
    //    group.writeEntry(d->configFineHAngleEntry, d->fineHAngleInput->value());
    //    group.writeEntry(d->configFineVAngleEntry, d->fineVAngleInput->value());
    group.writeEntry(d->configAntiAliasingEntry, d->antialiasInput->isChecked());

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

    slotPreview();
}

void ShearTool::preparePreview()
{
    float hAngle            = d->mainHAngleInput->value() + d->fineHAngleInput->value();
    float vAngle            = d->mainVAngleInput->value() + d->fineVAngleInput->value();
    bool antialiasing       = d->antialiasInput->isChecked();
    QColor background       = Qt::black;

    ImageIface* const iface = d->previewWidget->imageIface();
    int orgW                = iface->originalSize().width();
    int orgH                = iface->originalSize().height();
    DImg preview            = iface->preview();
    setFilter(new ShearFilter(&preview, this, hAngle, vAngle, antialiasing, background, orgW, orgH));
}

void ShearTool::prepareFinal()
{
    float hAngle         = d->mainHAngleInput->value() + d->fineHAngleInput->value();
    float vAngle         = d->mainVAngleInput->value() + d->fineVAngleInput->value();
    bool antialiasing    = d->antialiasInput->isChecked();
    QColor background    = Qt::black;

    ImageIface iface;
    int orgW             = iface.originalSize().width();
    int orgH             = iface.originalSize().height();
    DImg* const orgImage = iface.original();
    setFilter(new ShearFilter(orgImage, this, hAngle, vAngle, antialiasing, background, orgW, orgH));
}

void ShearTool::setPreviewImage()
{
    ImageIface* const iface = d->previewWidget->imageIface();
    int w                   = iface->previewSize().width();
    int h                   = iface->previewSize().height();
    DImg imTemp             = filter()->getTargetImage().smoothScale(w, h, Qt::KeepAspectRatio);
    DImg imDest( w, h, filter()->getTargetImage().sixteenBit(), filter()->getTargetImage().hasAlpha() );

    imDest.fill( DColor(d->previewWidget->palette().color(QPalette::Background).rgb(),
                        filter()->getTargetImage().sixteenBit()) );
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->setPreview(imDest.smoothScale(iface->previewSize()));

    d->previewWidget->updatePreview();

    ShearFilter* const tool = dynamic_cast<ShearFilter*>(filter());

    if (tool)
    {
        QSize newSize = tool->getNewSize();
        QString temp;
        d->newWidthLabel->setText(temp.setNum( newSize.width())   + i18n(" px") );
        d->newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
    }
}

void ShearTool::setFinalImage()
{
    ImageIface iface;
    DImg targetImage = filter()->getTargetImage();
    iface.setOriginal(i18n("Shear Tool"), filter()->filterAction(), targetImage);
}

}  // namespace Digikam
