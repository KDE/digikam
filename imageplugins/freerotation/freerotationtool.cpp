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

// C++ includes.

#include <cmath>

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
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
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>

// Local includes

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

FreeRotationTool::FreeRotationTool(QObject* parent)
                : EditorToolThreaded(parent)
{
    setObjectName("freerotation");
    setToolName(i18n("Free Rotation"));
    setToolIcon(SmallIcon("freerotation"));

    m_previewWidget = new ImageWidget("freerotation Tool", 0,
                                      i18n("This is the free rotation operation preview. "
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be drawn "
                                           "to guide you in adjusting the free rotation correction. "
                                           "Release the left mouse button to freeze the dashed "
                                           "line's position."),
                                      false, ImageGuideWidget::HVGuideMode);

    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    QString temp;
    Digikam::ImageIface iface(0, 0);

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::ColorGuide);

    QLabel *label1  = new QLabel(i18n("New width:"));
    m_newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"));
    m_newWidthLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    QLabel *label2   = new QLabel(i18n("New height:"));
    m_newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"));
    m_newHeightLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    QLabel *label3 = new QLabel(i18n("Main angle:"));
    m_angleInput   = new RIntNumInput;
    m_angleInput->setRange(-180, 180, 1);
    m_angleInput->setSliderEnabled(true);
    m_angleInput->setDefaultValue(0);
    m_angleInput->setWhatsThis(i18n("An angle in degrees by which to rotate the image. "
                                    "A positive angle rotates the image clockwise; "
                                    "a negative angle rotates it counter-clockwise."));

    QLabel *label4   = new QLabel(i18n("Fine angle:"));
    m_fineAngleInput = new RDoubleNumInput;
    m_fineAngleInput->input()->setRange(-5.0, 5.0, 0.01, true);
    m_fineAngleInput->setDefaultValue(0);
    m_fineAngleInput->setWhatsThis(i18n("This value in degrees will be added to main angle value "
                                        "to set fine target angle."));

    m_antialiasInput = new QCheckBox(i18n("Anti-Aliasing"));
    m_antialiasInput->setWhatsThis(i18n("Enable this option to apply the anti-aliasing filter "
                                        "to the rotated image. "
                                        "In order to smooth the target image, it will be blurred a little."));

    QLabel *label5 = new QLabel(i18n("Auto-crop:"));
    m_autoCropCB   = new RComboBox;
    m_autoCropCB->addItem(i18nc("no autocrop", "None"));
    m_autoCropCB->addItem(i18n("Widest Area"));
    m_autoCropCB->addItem(i18n("Largest Area"));
    m_autoCropCB->setDefaultIndex(FreeRotation::NoAutoCrop);
    m_autoCropCB->setWhatsThis(i18n("Select the method to process image auto-cropping "
                                    "to remove black frames around a rotated image here."));

    // -------------------------------------------------------------

    QString btnWhatsThis = i18n("Select some point in the preview widget, "
                                "then click this button to set it.");

    QPixmap pm1 = generateBtnPixmap(QString("1"), Qt::black);
    m_autoAdjustPoint1Btn = new QPushButton;
    m_autoAdjustPoint1Btn->setIcon(pm1);
    m_autoAdjustPoint1Btn->setText(i18n("Click to set"));
    m_autoAdjustPoint1Btn->setSizePolicy(QSizePolicy::MinimumExpanding,
                                         QSizePolicy::MinimumExpanding);

    QPixmap pm2 = generateBtnPixmap(QString("2"), Qt::black);
    m_autoAdjustPoint2Btn = new QPushButton;
    m_autoAdjustPoint2Btn->setIcon(pm2);
    m_autoAdjustPoint2Btn->setText(i18n("Click to set"));
    m_autoAdjustPoint2Btn->setSizePolicy(QSizePolicy::MinimumExpanding,
                                         QSizePolicy::MinimumExpanding);

    m_autoAdjustPoint1Btn->setToolTip(btnWhatsThis);
    m_autoAdjustPoint1Btn->setWhatsThis(btnWhatsThis);
    m_autoAdjustPoint2Btn->setToolTip(btnWhatsThis);
    m_autoAdjustPoint2Btn->setWhatsThis(btnWhatsThis);

    // try to determine the maximum text width, to set the button minwidth
    QFont fnt = m_autoAdjustPoint1Btn->font();
    QFontMetrics fm(fnt);
    int minWidth = fm.width(QString("(1234, 1234)")) + pm1.width() * 2 + 5;

    // set new minwidth
    m_autoAdjustPoint1Btn->setMinimumWidth(minWidth);
    m_autoAdjustPoint2Btn->setMinimumWidth(minWidth);

    m_autoAdjustBtn = new QPushButton(i18nc("Automatic Adjustment", "Adjust"));
    m_autoAdjustBtn->setMinimumHeight(50);

    // -------------------------------------------------------------

    m_manualAdjustInput          = new QCheckBox(i18n("Manual Adjustment"));
    m_manualAdjustContainer      = new QWidget;
    QGridLayout *containerLayout = new QGridLayout;
    containerLayout->addWidget(label3,              0, 0, 1, 1);
    containerLayout->addWidget(m_angleInput,        1, 0, 1, 1);
    containerLayout->addWidget(label4,              2, 0, 1, 1);
    containerLayout->addWidget(m_fineAngleInput,    3, 0, 1, 1);
    m_manualAdjustContainer->setLayout(containerLayout);

    // -------------------------------------------------------------

    m_autoAdjustContainer         = new QGroupBox(i18n("Automatic Adjustment"));
    m_autoAdjustContainer->setFlat(true);
    QGridLayout *containerLayout2 = new QGridLayout;
    containerLayout2->addWidget(m_autoAdjustPoint1Btn,   0, 0, 1, 1);
    containerLayout2->addWidget(m_autoAdjustBtn,         0, 2, 2, 1);
    containerLayout2->addWidget(m_autoAdjustPoint2Btn,   1, 0, 1, 1);
    containerLayout2->setColumnStretch(1, 10);
    containerLayout2->setMargin(0);
    m_autoAdjustContainer->setLayout(containerLayout2);

    // -------------------------------------------------------------

    KSeparator *line  = new KSeparator(Qt::Horizontal);
    KSeparator *line2 = new KSeparator(Qt::Horizontal);
    KSeparator *line3 = new KSeparator(Qt::Horizontal);

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,                  0, 0, 1, 1);
    mainLayout->addWidget(m_newWidthLabel,         0, 1, 1, 1);
    mainLayout->addWidget(label2,                  1, 0, 1, 1);
    mainLayout->addWidget(m_newHeightLabel,        1, 1, 1, 1);
    mainLayout->addWidget(line,                    2, 0, 1,-1);
    mainLayout->addWidget(m_autoAdjustContainer,   3, 0, 1,-1);
    mainLayout->addWidget(line2,                   4, 0, 1,-1);
    mainLayout->addWidget(m_antialiasInput,        5, 0, 1,-1);
    mainLayout->addWidget(m_manualAdjustInput,     6, 0, 1, 1);
    mainLayout->addWidget(m_manualAdjustContainer, 7, 0, 1,-1);
    mainLayout->addWidget(line3,                   8, 0, 1,-1);
    mainLayout->addWidget(label5,                  9, 0, 1, 1);
    mainLayout->addWidget(m_autoCropCB,            9, 1, 1, 1);
    mainLayout->setRowStretch(10, 10);
    mainLayout->setMargin(m_gboxSettings->spacingHint());
    mainLayout->setSpacing(m_gboxSettings->spacingHint());
    m_gboxSettings->plainPage()->setLayout(mainLayout);

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_angleInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_fineAngleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(m_antialiasInput, SIGNAL(toggled(bool)),
            this, SLOT(slotEffect()));

    connect(m_autoCropCB, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    connect(m_gboxSettings, SIGNAL(signalColorGuideChanged()),
            this, SLOT(slotColorGuideChanged()));

    connect(m_manualAdjustInput, SIGNAL(toggled(bool)),
            this, SLOT(slotManualAdjustToggled(bool)));

    connect(m_autoAdjustPoint1Btn, SIGNAL(clicked()),
            this, SLOT(slotAutoAdjustP1Clicked()));

    connect(m_autoAdjustPoint2Btn, SIGNAL(clicked()),
            this, SLOT(slotAutoAdjustP2Clicked()));

    connect(m_autoAdjustBtn, SIGNAL(clicked()),
            this, SLOT(slotAutoAdjustClicked()));
}

FreeRotationTool::~FreeRotationTool()
{
}

void FreeRotationTool::slotColorGuideChanged()
{
    m_previewWidget->slotChangeGuideColor(m_gboxSettings->guideColor());
    m_previewWidget->slotChangeGuideSize(m_gboxSettings->guideSize());
}

void FreeRotationTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("freerotation Tool");
//    m_angleInput->setValue(group.readEntry("Main Angle", m_angleInput->defaultValue()));
//    m_fineAngleInput->setValue(group.readEntry("Fine Angle", m_fineAngleInput->defaultValue()));
    m_autoCropCB->setCurrentIndex(group.readEntry("Auto Crop Type", m_autoCropCB->defaultIndex()));
    m_antialiasInput->setChecked(group.readEntry("Anti Aliasing", true));
    m_manualAdjustInput->setChecked(group.readEntry("Manual Adjust", false));

    m_angleInput->blockSignals(true);
    m_fineAngleInput->blockSignals(true);
    m_angleInput->slotReset();
    m_fineAngleInput->slotReset();
    m_angleInput->blockSignals(false);
    m_fineAngleInput->blockSignals(false);

    resetPoints();
    slotManualAdjustToggled(m_manualAdjustInput->isChecked());

    slotColorGuideChanged();
    slotEffect();
}

void FreeRotationTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("freerotation Tool");
//    group.writeEntry("Main Angle", m_angleInput->value());
//    group.writeEntry("Fine Angle", m_fineAngleInput->value());
    group.writeEntry("Auto Crop Type", m_autoCropCB->currentIndex());
    group.writeEntry("Anti Aliasing", m_antialiasInput->isChecked());
    group.writeEntry("Manual Adjust", m_manualAdjustInput->isChecked());
    m_previewWidget->writeSettings();
    group.sync();
}

void FreeRotationTool::slotResetSettings()
{
    m_angleInput->blockSignals(true);
    m_antialiasInput->blockSignals(true);
    m_autoCropCB->blockSignals(true);

    m_angleInput->slotReset();
    m_fineAngleInput->slotReset();
    m_antialiasInput->setChecked(true);
    m_autoCropCB->slotReset();

    m_angleInput->blockSignals(false);
    m_antialiasInput->blockSignals(false);
    m_autoCropCB->blockSignals(false);

    resetPoints();
    slotEffect();
}

void FreeRotationTool::prepareEffect()
{
    kapp->setOverrideCursor(Qt::WaitCursor);
    m_angleInput->setEnabled(false);
    m_fineAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);
    m_autoCropCB->setEnabled(false);
    m_manualAdjustInput->setEnabled(false);
    m_autoAdjustContainer->setEnabled(false);
    m_manualAdjustContainer->setEnabled(false);

    double angle      = m_angleInput->value() + m_fineAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();
    int autocrop      = m_autoCropCB->currentIndex();
    QColor background = toolView()->backgroundRole();

    ImageIface* iface = m_previewWidget->imageIface();
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
    m_angleInput->setEnabled(false);
    m_fineAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);
    m_autoCropCB->setEnabled(false);
    m_manualAdjustInput->setEnabled(false);
    m_autoAdjustContainer->setEnabled(false);
    m_manualAdjustContainer->setEnabled(false);

    double angle      = m_angleInput->value() + m_fineAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();
    int autocrop      = m_autoCropCB->currentIndex();
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
    ImageIface* iface = m_previewWidget->imageIface();
    int w = iface->previewWidth();
    int h = iface->previewHeight();

    DImg imTemp = filter()->getTargetImage().smoothScale(w, h, Qt::ScaleMin);
    DImg imDest( w, h, filter()->getTargetImage().sixteenBit(),
                                filter()->getTargetImage().hasAlpha() );

    QColor background = toolView()->backgroundRole();
    imDest.fill( DColor(background, filter()->getTargetImage().sixteenBit()) );
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                                               iface->previewHeight())).bits());

    m_previewWidget->updatePreview();
    QSize newSize = dynamic_cast<FreeRotation *>(filter())->getNewSize();
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
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
    m_angleInput->setEnabled(true);
    m_fineAngleInput->setEnabled(true);
    m_antialiasInput->setEnabled(true);
    m_autoCropCB->setEnabled(true);
    m_manualAdjustInput->setEnabled(true);
    m_autoAdjustContainer->setEnabled(true);
    m_manualAdjustContainer->setEnabled(true);

    bool autoHorizon = m_manualAdjustInput->isChecked();
    slotManualAdjustToggled(autoHorizon);

    kapp->restoreOverrideCursor();
}

void FreeRotationTool::slotManualAdjustToggled(bool t)
{
    m_manualAdjustContainer->setEnabled(t);
    resetPoints();
}

QString FreeRotationTool::generatePointLabel(const QPoint &p)
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
    tmp = generatePointLabel(m_autoAdjustPoint1);
    m_autoAdjustPoint1Btn->setText(tmp);

    tmp = generatePointLabel(m_autoAdjustPoint2);
    m_autoAdjustPoint2Btn->setText(tmp);

    // set points in preview widget, don't add invalid points
    QPolygon points;
    if (pointIsValid(m_autoAdjustPoint1))
    {
        points << m_autoAdjustPoint1;
        m_autoAdjustPoint2Btn->setEnabled(true);
    }
    else
    {
        m_autoAdjustPoint2Btn->setEnabled(false);
    }
    if (pointIsValid(m_autoAdjustPoint2))
    {
        points << m_autoAdjustPoint2;
    }
    m_previewWidget->setPoints(points, true);

    // enable / disable adjustment buttons
    bool valid  = pointIsValid(m_autoAdjustPoint1) && pointIsValid(m_autoAdjustPoint2);
    m_autoAdjustBtn->setEnabled(valid);
}

void FreeRotationTool::resetPoints()
{
    setPointInvalid(m_autoAdjustPoint1);
    setPointInvalid(m_autoAdjustPoint2);
    m_previewWidget->resetPoints();
    updatePoints();
}

void FreeRotationTool::slotAutoAdjustP1Clicked()
{
    m_autoAdjustPoint1 = m_previewWidget->getSpotPosition();
    updatePoints();
}

void FreeRotationTool::slotAutoAdjustP2Clicked()
{
    m_autoAdjustPoint2 = m_previewWidget->getSpotPosition();
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
    angle = (double)m_angleInput->value() + m_fineAngleInput->value() + angle;

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
        if (!ok) fineAngle = 0.0;

        m_angleInput->setValue(mainAngle);
        m_fineAngleInput->setValue(fineAngle);
    }

    resetPoints();
}

QPixmap FreeRotationTool::generateBtnPixmap(const QString &label, const QColor &color)
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
    return calculateAngle(m_autoAdjustPoint1, m_autoAdjustPoint2);
}

double FreeRotationTool::calculateAngle(const QPoint &p1, const QPoint &p2)
{
    // check if all points are valid
    if (!pointIsValid(p1) && !pointIsValid(p2))
        return 0.0;

    // check if point are not equal
    if (p1 == p2)
        return 0.0;

    // if y() is equal, no angle needs to be calculated
    if (p1.y() == p2.y())
        return 0.0;

    // if x() is equal, angle equals 90°
    if (p1.y() == p2.y())
        return 90.0;

    // check point layout
    bool reverse = ((p1.x() < p2.x()) && (p2.y() > p1.y())) ||
                   ((p1.x() > p2.x()) && (p2.y() < p1.y()));

    // calculate the angle
    double angle = 0.0;
    double ly = fabs((double)p2.y() - p1.y());
    double lx = fabs((double)p2.x() - p1.x());

    angle = atan2(ly, lx) * 180.0 / M_PI;
    angle = reverse ? -angle : angle;

    return angle;
}

void FreeRotationTool::setPointInvalid(QPoint &p)
{
    p.setX(-1);
    p.setY(-1);
}

bool FreeRotationTool::pointIsValid(const QPoint &p)
{
    bool valid = true;
    if (p.x() == -1 || p.y() == -1)
        valid = false;
    return valid;
}

}  // namespace DigikamFreeRotationImagesPlugin
