/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-28
 * Description : a digiKam image editor plugin to process image
 *               free rotation.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmx dot net>
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


#include "freerotationtool.h"
#include "freerotationtool.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QPushButton>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>

// Local includes

#include "rexpanderbox.h"
#include "daboutdata.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "freerotation.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "version.h"

using namespace Digikam;
using namespace KDcrawIface;

namespace DigikamFreeRotationImagesPlugin
{

class FreeRotationToolPriv
{
public:

    FreeRotationToolPriv()
    {
        antialiasInput      = 0;
        newHeightLabel      = 0;
        newWidthLabel       = 0;
        autoAdjustBtn       = 0;
        autoAdjustPoint1Btn = 0;
        autoAdjustPoint2Btn = 0;
        expanderBox         = 0;
        gboxSettings        = 0;
        previewWidget       = 0;
        autoCropCB          = 0;
        fineAngleInput      = 0;
        angleInput          = 0;
    }

    QCheckBox*           antialiasInput;

    QLabel*              newHeightLabel;
    QLabel*              newWidthLabel;

    QPoint               autoAdjustPoint1;
    QPoint               autoAdjustPoint2;

    QPushButton*         autoAdjustBtn;
    QPushButton*         autoAdjustPoint1Btn;
    QPushButton*         autoAdjustPoint2Btn;

    RExpanderBox*        expanderBox;
    EditorToolSettings*  gboxSettings;
    ImageWidget*         previewWidget;

    RComboBox*           autoCropCB;
    RDoubleNumInput*     fineAngleInput;
    RIntNumInput*        angleInput;
};

FreeRotationTool::FreeRotationTool(QObject* parent)
                : EditorToolThreaded(parent),
                  d(new FreeRotationToolPriv)
{
    setObjectName("freerotation");
    setToolName(i18n("Free Rotation"));
    setToolIcon(SmallIcon("freerotation"));

    d->previewWidget = new ImageWidget("freerotation Tool", 0,
                                       i18n("This is the free rotation operation preview. "
                                            "If you move the mouse cursor on this preview, "
                                            "a vertical and horizontal dashed line will be drawn "
                                            "to guide you in adjusting the free rotation correction. "
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


    QLabel *label1   = new QLabel(i18n("New width:"));
    d->newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"));
    d->newWidthLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    QLabel *label2    = new QLabel(i18n("New height:"));
    d->newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"));
    d->newHeightLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    QLabel *label3  = new QLabel(i18n("Main angle:"));
    d->angleInput   = new RIntNumInput;
    d->angleInput->setRange(-180, 180, 1);
    d->angleInput->setSliderEnabled(true);
    d->angleInput->setDefaultValue(0);
    d->angleInput->setWhatsThis(i18n("An angle in degrees by which to rotate the image. "
                                     "A positive angle rotates the image clockwise; "
                                     "a negative angle rotates it counter-clockwise."));

    QLabel *label4    = new QLabel(i18n("Fine angle:"));
    d->fineAngleInput = new RDoubleNumInput;
    d->fineAngleInput->input()->setRange(-1.0, 1.0, 0.01, true);
    d->fineAngleInput->setDefaultValue(0);
    d->fineAngleInput->setWhatsThis(i18n("This value in degrees will be added to main angle value "
                                         "to set fine target angle."));

    d->antialiasInput = new QCheckBox(i18n("Anti-Aliasing"));
    d->antialiasInput->setWhatsThis(i18n("Enable this option to apply the anti-aliasing filter "
                                         "to the rotated image. "
                                         "In order to smooth the target image, it will be blurred a little."));

    QLabel *label5  = new QLabel(i18n("Auto-crop:"));
    d->autoCropCB   = new RComboBox;
    d->autoCropCB->addItem(i18nc("no autocrop", "None"));
    d->autoCropCB->addItem(i18n("Widest Area"));
    d->autoCropCB->addItem(i18n("Largest Area"));
    d->autoCropCB->setDefaultIndex(FreeRotation::NoAutoCrop);
    d->autoCropCB->setWhatsThis(i18n("Select the method to process image auto-cropping "
                                     "to remove black frames around a rotated image here."));

    // -------------------------------------------------------------

    QString btnWhatsThis = i18n("Select a point in the preview widget, "
                                "then click this button to assign the point for auto-correction.");

    QPixmap pm1 = generateBtnPixmap(QString("1"), Qt::black);
    d->autoAdjustPoint1Btn = new QPushButton;
    d->autoAdjustPoint1Btn->setIcon(pm1);
    d->autoAdjustPoint1Btn->setText(i18n("Click to set"));
    d->autoAdjustPoint1Btn->setSizePolicy(QSizePolicy::MinimumExpanding,
                                          QSizePolicy::MinimumExpanding);

    QPixmap pm2 = generateBtnPixmap(QString("2"), Qt::black);
    d->autoAdjustPoint2Btn = new QPushButton;
    d->autoAdjustPoint2Btn->setIcon(pm2);
    d->autoAdjustPoint2Btn->setText(i18n("Click to set"));
    d->autoAdjustPoint2Btn->setSizePolicy(QSizePolicy::MinimumExpanding,
                                          QSizePolicy::MinimumExpanding);

    d->autoAdjustPoint1Btn->setToolTip(btnWhatsThis);
    d->autoAdjustPoint1Btn->setWhatsThis(btnWhatsThis);
    d->autoAdjustPoint2Btn->setToolTip(btnWhatsThis);
    d->autoAdjustPoint2Btn->setWhatsThis(btnWhatsThis);

    // try to determine the maximum text width, to set the button minwidth
    QFont fnt = d->autoAdjustPoint1Btn->font();
    QFontMetrics fm(fnt);
    int minWidth = fm.width(QString("(1234, 1234)")) + pm1.width() * 2 + 5;

    // set new minwidth
    d->autoAdjustPoint1Btn->setMinimumWidth(minWidth);
    d->autoAdjustPoint2Btn->setMinimumWidth(minWidth);

    d->autoAdjustBtn = new QPushButton(i18nc("Automatic Adjustment", "Adjust"));
    d->autoAdjustBtn->setSizePolicy(QSizePolicy::MinimumExpanding,
                                    QSizePolicy::Expanding);

    // -------------------------------------------------------------

    QWidget* manualAdjustContainer = new QWidget;
    QGridLayout *containerLayout   = new QGridLayout;
    containerLayout->addWidget(label3,              0, 0, 1, 1);
    containerLayout->addWidget(d->angleInput,       1, 0, 1, 1);
    containerLayout->addWidget(label4,              2, 0, 1, 1);
    containerLayout->addWidget(d->fineAngleInput,   3, 0, 1, 1);
    containerLayout->setMargin(KDialog::marginHint());
    manualAdjustContainer->setLayout(containerLayout);

    // -------------------------------------------------------------

    QWidget* autoAdjustContainer  = new QWidget;
    QGridLayout *containerLayout2 = new QGridLayout;
    QLabel *autoDescr             = new QLabel;
    autoDescr->setText(i18n("<p>Correct the rotation of your images automatically by assigning two points in the "
                            "preview widget and clicking <i>Adjust</i>.<br/>"
                            "You can either adjust horizontal or vertical lines.</p>"));
    autoDescr->setAlignment(Qt::AlignJustify);
    autoDescr->setWordWrap(true);
    containerLayout2->addWidget(autoDescr,              0, 0, 1,-1);
    containerLayout2->addWidget(d->autoAdjustPoint1Btn, 1, 0, 1, 1);
    containerLayout2->addWidget(d->autoAdjustBtn,       1, 2, 2, 1);
    containerLayout2->addWidget(d->autoAdjustPoint2Btn, 2, 0, 1, 1);
    containerLayout2->setColumnStretch(1, 10);
    containerLayout2->setMargin(KDialog::marginHint());
    autoAdjustContainer->setLayout(containerLayout2);

    // -------------------------------------------------------------

    QWidget* additionalSettingsContainer = new QWidget;
    QGridLayout* containerLayout3 = new QGridLayout;
    containerLayout3->addWidget(d->antialiasInput, 0, 0, 1,-1);
    containerLayout3->addWidget(label5,            1, 0, 1, 1);
    containerLayout3->addWidget(d->autoCropCB,     1, 1, 1, 1);
    additionalSettingsContainer->setLayout(containerLayout3);

    // -------------------------------------------------------------

    KSeparator *line  = new KSeparator(Qt::Horizontal);

    d->expanderBox = new RExpanderBox;
    d->expanderBox->addItem(autoAdjustContainer, SmallIcon("freerotation"), i18n("Automatic Correction"),
                            QString("AutoAdjustContainer"), true);
    d->expanderBox->addItem(manualAdjustContainer, SmallIcon("freerotation"), i18n("Manual Adjustment"),
                            QString("ManualAdjustContainer"), true);
    d->expanderBox->addItem(additionalSettingsContainer, SmallIcon("freerotation"), i18n("Additional Settings"),
                            QString("SettingsContainer"), true);
    d->expanderBox->addStretch();

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,               0, 0, 1, 1);
    mainLayout->addWidget(d->newWidthLabel,     0, 1, 1, 1);
    mainLayout->addWidget(label2,               1, 0, 1, 1);
    mainLayout->addWidget(d->newHeightLabel,    1, 1, 1, 1);
    mainLayout->addWidget(line,                 2, 0, 1,-1);
    mainLayout->addWidget(d->expanderBox,       3, 0, 1,-1);
    mainLayout->setRowStretch(3, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->angleInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->fineAngleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(d->antialiasInput, SIGNAL(toggled(bool)),
            this, SLOT(slotEffect()));

    connect(d->autoCropCB, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    connect(d->gboxSettings, SIGNAL(signalColorGuideChanged()),
            this, SLOT(slotColorGuideChanged()));

    connect(d->autoAdjustPoint1Btn, SIGNAL(clicked()),
            this, SLOT(slotAutoAdjustP1Clicked()));

    connect(d->autoAdjustPoint2Btn, SIGNAL(clicked()),
            this, SLOT(slotAutoAdjustP2Clicked()));

    connect(d->autoAdjustBtn, SIGNAL(clicked()),
            this, SLOT(slotAutoAdjustClicked()));
}

FreeRotationTool::~FreeRotationTool()
{
    delete d;
}

void FreeRotationTool::slotColorGuideChanged()
{
    d->previewWidget->slotChangeGuideColor(d->gboxSettings->guideColor());
    d->previewWidget->slotChangeGuideSize(d->gboxSettings->guideSize());
}

void FreeRotationTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("freerotation Tool");
    d->autoCropCB->setCurrentIndex(group.readEntry("Auto Crop Type", d->autoCropCB->defaultIndex()));
    d->antialiasInput->setChecked(group.readEntry("Anti Aliasing", true));
    d->expanderBox->readSettings(group);

    d->angleInput->blockSignals(true);
    d->fineAngleInput->blockSignals(true);
    d->angleInput->slotReset();
    d->fineAngleInput->slotReset();
    d->angleInput->blockSignals(false);
    d->fineAngleInput->blockSignals(false);

    resetPoints();
    slotColorGuideChanged();
    slotEffect();
}

void FreeRotationTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("freerotation Tool");
    group.writeEntry("Auto Crop Type", d->autoCropCB->currentIndex());
    group.writeEntry("Anti Aliasing", d->antialiasInput->isChecked());
    d->expanderBox->writeSettings(group);
    d->previewWidget->writeSettings();
    group.sync();
}

void FreeRotationTool::slotResetSettings()
{
    d->angleInput->blockSignals(true);
    d->antialiasInput->blockSignals(true);
    d->autoCropCB->blockSignals(true);

    d->angleInput->slotReset();
    d->fineAngleInput->slotReset();
    d->antialiasInput->setChecked(true);
    d->autoCropCB->slotReset();

    d->angleInput->blockSignals(false);
    d->antialiasInput->blockSignals(false);
    d->autoCropCB->blockSignals(false);

    resetPoints();
    slotEffect();
}

void FreeRotationTool::prepareEffect()
{
    kapp->setOverrideCursor(Qt::WaitCursor);
    d->expanderBox->setEnabled(false);

    double angle      = d->angleInput->value() + d->fineAngleInput->value();
    bool antialiasing = d->antialiasInput->isChecked();
    int autocrop      = d->autoCropCB->currentIndex();
    QColor background = toolView()->backgroundRole();

    ImageIface* iface = d->previewWidget->imageIface();
    int orgW          = iface->originalWidth();
    int orgH          = iface->originalHeight();

    uchar *data = iface->getPreviewImage();
    DImg image(iface->previewWidth(), iface->previewHeight(), iface->previewSixteenBit(),
                        iface->previewHasAlpha(), data);
    delete [] data;

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new FreeRotation(&image, this, angle, antialiasing, autocrop,
                                        background, orgW, orgH)));
}

void FreeRotationTool::prepareFinal()
{
    d->expanderBox->setEnabled(false);

    double angle      = d->angleInput->value() + d->fineAngleInput->value();
    bool antialiasing = d->antialiasInput->isChecked();
    int autocrop      = d->autoCropCB->currentIndex();
    QColor background = Qt::black;

    ImageIface iface(0, 0);
    int orgW = iface.originalWidth();
    int orgH = iface.originalHeight();

    uchar *data = iface.getOriginalImage();
    DImg orgImage(orgW, orgH, iface.originalSixteenBit(),
                           iface.originalHasAlpha(), data);
    delete [] data;

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new FreeRotation(&orgImage, this, angle, antialiasing, autocrop,
                                        background, orgW, orgH)));
}

void FreeRotationTool::putPreviewData(void)
{
    ImageIface* iface = d->previewWidget->imageIface();
    int w = iface->previewWidth();
    int h = iface->previewHeight();

    DImg imTemp = filter()->getTargetImage().smoothScale(w, h, Qt::KeepAspectRatio);
    DImg imDest( w, h, filter()->getTargetImage().sixteenBit(),
                                filter()->getTargetImage().hasAlpha() );

    QColor background = toolView()->backgroundRole();
    imDest.fill( DColor(background, filter()->getTargetImage().sixteenBit()) );
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                                               iface->previewHeight())).bits());

    d->previewWidget->updatePreview();
    QSize newSize = dynamic_cast<FreeRotation *>(filter())->getNewSize();
    QString temp;
    d->newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    d->newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
}

void FreeRotationTool::putFinalData(void)
{
    ImageIface iface(0, 0);
    DImg targetImage = filter()->getTargetImage();
    iface.putOriginalImage(i18n("Free Rotation"),
                           targetImage.bits(),
                           targetImage.width(), targetImage.height());
}

void FreeRotationTool::renderingFinished()
{
    d->expanderBox->setEnabled(true);
    kapp->restoreOverrideCursor();
}

QString FreeRotationTool::generatePointLabel(const QPoint& p)
{
    if (!pointIsValid(p))
        return QString(i18n("Click to set"));

    QString label = QString("(%1, %2)")
                           .arg(p.x())
                           .arg(p.y());
    return label;
}

void FreeRotationTool::updatePoints()
{
    // set labels
    QString tmp;
    tmp = generatePointLabel(d->autoAdjustPoint1);
    d->autoAdjustPoint1Btn->setText(tmp);

    tmp = generatePointLabel(d->autoAdjustPoint2);
    d->autoAdjustPoint2Btn->setText(tmp);

    // set points in preview widget, don't add invalid points
    QPolygon points;
    if (pointIsValid(d->autoAdjustPoint1))
    {
        points << d->autoAdjustPoint1;
        d->autoAdjustPoint2Btn->setEnabled(true);
    }
    else
    {
        d->autoAdjustPoint2Btn->setEnabled(false);
    }
    if (pointIsValid(d->autoAdjustPoint2))
    {
        points << d->autoAdjustPoint2;
    }
    d->previewWidget->setPoints(points, true);

    // enable / disable adjustment buttons
    bool valid  = (pointIsValid(d->autoAdjustPoint1) && pointIsValid(d->autoAdjustPoint2))
                  && (d->autoAdjustPoint1 != d->autoAdjustPoint2);
    d->autoAdjustBtn->setEnabled(valid);
}

void FreeRotationTool::resetPoints()
{
    setPointInvalid(d->autoAdjustPoint1);
    setPointInvalid(d->autoAdjustPoint2);
    d->previewWidget->resetPoints();
    updatePoints();
}

void FreeRotationTool::slotAutoAdjustP1Clicked()
{
    d->autoAdjustPoint1 = d->previewWidget->getSpotPosition();
    updatePoints();
}

void FreeRotationTool::slotAutoAdjustP2Clicked()
{
    d->autoAdjustPoint2 = d->previewWidget->getSpotPosition();
    updatePoints();
}

void FreeRotationTool::slotAutoAdjustClicked()
{
    double angle = calculateAutoAngle();
    if (fabs(angle) > 45.0)
    {
        if (angle < 0.0)
            angle += 90.0;
        else
            angle -= 90.0;
    }

    // we need to add the calculated angle to the currently set angle
    angle = (double)d->angleInput->value() + d->fineAngleInput->value() + angle;

    // convert the angle to a string so we can easily split it up
    QString angleStr       = QString::number(angle, 'f', 2);
    QStringList anglesList = angleStr.split('.');

    // try to set the angle widgets with the extracted values
    if (anglesList.count() == 2)
    {
        bool ok = false;
        int mainAngle = anglesList[0].toInt(&ok);
        if (!ok) mainAngle = 0;

        double fineAngle = (QString("0.") + anglesList[1]).toDouble(&ok);
        fineAngle = (angle < 0.0) ? -fineAngle : fineAngle;
        if (!ok) fineAngle = 0.0;

        d->angleInput->setValue(mainAngle);
        d->fineAngleInput->setValue(fineAngle);
    }

    resetPoints();
}

QPixmap FreeRotationTool::generateBtnPixmap(const QString& label, const QColor& color)
{
    QPixmap pm(22, 22);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(color);

    p.drawEllipse(1, 1, 20, 20);
    p.drawText(pm.rect(), label, Qt::AlignHCenter | Qt::AlignVCenter);

    p.end();

    return pm;
}

double FreeRotationTool::calculateAutoAngle()
{
    // check if all points are valid
    if (!pointIsValid(d->autoAdjustPoint1) && !pointIsValid(d->autoAdjustPoint2))
        return 0.0;

    return FreeRotation::calculateAngle(d->autoAdjustPoint1, d->autoAdjustPoint2);
}

void FreeRotationTool::setPointInvalid(QPoint& p)
{
    p.setX(-1);
    p.setY(-1);
}

bool FreeRotationTool::pointIsValid(const QPoint& p)
{
    bool valid = true;
    if (p.x() == -1 || p.y() == -1)
        valid = false;
    return valid;
}

}  // namespace DigikamFreeRotationImagesPlugin
