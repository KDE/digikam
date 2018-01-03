/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-14
 * Description : a curves widget with additional control elements
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "curvesbox.h"

// Qt includes

#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QPushButton>
#include <QTextStream>
#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "colorgradientwidget.h"
#include "curveswidget.h"
#include "editortoolsettings.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "digikam_globals.h"
#include "dimg.h"

namespace Digikam
{

class CurvesBox::Private
{
public:

    Private() :
        sixteenBit(false),
        channel(LuminosityChannel),
        curveFree(0),
        curveSmooth(0),
        pickBlack(0),
        pickGray(0),
        pickWhite(0),
        curveType(0),
        pickerType(0),
        pickerBox(0),
        resetButton(0),
        curvesWidget(0),
        hGradient(0),
        vGradient(0)
    {
    }

    bool                 sixteenBit;
    ChannelType          channel;

    QToolButton*         curveFree;
    QToolButton*         curveSmooth;
    QToolButton*         pickBlack;
    QToolButton*         pickGray;
    QToolButton*         pickWhite;

    QButtonGroup*        curveType;
    QButtonGroup*        pickerType;

    QWidget*             pickerBox;

    QPushButton*         resetButton;

    CurvesWidget*        curvesWidget;
    ColorGradientWidget* hGradient;
    ColorGradientWidget* vGradient;
};

CurvesBox::CurvesBox(int w, int h, QWidget* const parent, bool readOnly)
    : QWidget(parent), d(new Private)
{
    d->curvesWidget = new CurvesWidget(w, h, this, readOnly);
    setup();
}

CurvesBox::CurvesBox(int w, int h, const DImg& img, QWidget* const parent, bool readOnly)
    : QWidget(parent),
      d(new Private)
{
    d->sixteenBit   = img.sixteenBit();
    d->curvesWidget = new CurvesWidget(w, h, this, readOnly);
    d->curvesWidget->updateData(img);
    d->curvesWidget->setChannelType(d->channel);
    setup();
}

void CurvesBox::setup()
{
    QWidget* curveBox = new QWidget();

    d->vGradient = new ColorGradientWidget(Qt::Vertical, 10);
    d->vGradient->setColors(QColor("white"), QColor("black"));

    d->hGradient = new ColorGradientWidget(Qt::Horizontal, 10);
    d->hGradient->setColors(QColor("black"), QColor("white"));

    QGridLayout* curveBoxLayout = new QGridLayout;
    curveBoxLayout->addWidget(d->vGradient,    0, 0, 1, 1);
    curveBoxLayout->addWidget(d->curvesWidget, 0, 2, 1, 1);
    curveBoxLayout->addWidget(d->hGradient,    2, 2, 1, 1);
    curveBoxLayout->setRowMinimumHeight(1, 2);
    curveBoxLayout->setColumnMinimumWidth(1, 2);
    curveBoxLayout->setContentsMargins(QMargins());
    curveBoxLayout->setSpacing(0);
    curveBox->setLayout(curveBoxLayout);

    // -------------------------------------------------------------

    QWidget* typeBox = new QWidget();

    d->curveFree = new QToolButton;
    d->curveFree->setIcon(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/curvefree.png"))));
    d->curveFree->setCheckable(true);
    d->curveFree->setToolTip(i18n("Curve free mode"));
    d->curveFree->setWhatsThis(i18n("With this button, you can draw your curve free-hand "
                                    "with the mouse."));

    d->curveSmooth = new QToolButton;
    d->curveSmooth->setIcon(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/curvemooth.png"))));
    d->curveSmooth->setCheckable(true);
    d->curveSmooth->setToolTip(i18n("Curve smooth mode"));
    d->curveSmooth->setWhatsThis(i18n("With this button, the curve type is constrained to "
                                      "be a smooth line with tension."));

    d->curveType = new QButtonGroup(typeBox);
    d->curveType->addButton(d->curveFree,   FreeDrawing);
    d->curveType->addButton(d->curveSmooth, SmoothDrawing);

    d->curveType->setExclusive(true);
    d->curveSmooth->setChecked(true);

    QHBoxLayout* typeBoxLayout = new QHBoxLayout;
    typeBoxLayout->addWidget(d->curveFree);
    typeBoxLayout->addWidget(d->curveSmooth);
    typeBoxLayout->setContentsMargins(QMargins());
    typeBoxLayout->setSpacing(0);
    typeBox->setLayout(typeBoxLayout);

    // -------------------------------------------------------------

    d->pickerBox = new QWidget();

    d->pickBlack = new QToolButton;
    d->pickBlack->setIcon(QIcon::fromTheme(QLatin1String("color-picker-black")));
    d->pickBlack->setCheckable(true);
    d->pickBlack->setToolTip(i18n("All channels shadow tone color picker"));
    d->pickBlack->setWhatsThis(i18n("With this button, you can pick the color from original "
                                    "image used to set <b>Shadow Tone</b> "
                                    "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    d->pickGray = new QToolButton;
    d->pickGray->setIcon(QIcon::fromTheme(QLatin1String("color-picker-grey")));
    d->pickGray->setCheckable(true);
    d->pickGray->setToolTip(i18n("All channels middle tone color picker"));
    d->pickGray->setWhatsThis(i18n("With this button, you can pick the color from original "
                                   "image used to set <b>Middle Tone</b> "
                                   "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    d->pickWhite = new QToolButton;
    d->pickWhite->setIcon(QIcon::fromTheme(QLatin1String("color-picker-white")));
    d->pickWhite->setCheckable(true);
    d->pickWhite->setToolTip(i18n("All channels highlight tone color picker"));
    d->pickWhite->setWhatsThis(i18n("With this button, you can pick the color from original "
                                    "image used to set <b>Highlight Tone</b> "
                                    "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    d->pickerType = new QButtonGroup(d->pickerBox);
    d->pickerType->addButton(d->pickBlack, BlackTonal);
    d->pickerType->addButton(d->pickGray,  GrayTonal);
    d->pickerType->addButton(d->pickWhite, WhiteTonal);

    QHBoxLayout* const pickerBoxLayout = new QHBoxLayout;
    pickerBoxLayout->addWidget(d->pickBlack);
    pickerBoxLayout->addWidget(d->pickGray);
    pickerBoxLayout->addWidget(d->pickWhite);
    pickerBoxLayout->setContentsMargins(QMargins());
    pickerBoxLayout->setSpacing(0);
    d->pickerBox->setLayout(pickerBoxLayout);

    d->pickerType->setExclusive(true);

    // -------------------------------------------------------------

    d->resetButton = new QPushButton(i18n("&Reset"));
    d->resetButton->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->resetButton->setToolTip(i18n("Reset current channel curves' values."));
    d->resetButton->setWhatsThis(i18n("If you press this button, all curves' values "
                                      "from the currently selected channel "
                                      "will be reset to the default values."));

    QHBoxLayout* const l3 = new QHBoxLayout();
    l3->addWidget(typeBox);
    l3->addWidget(d->pickerBox);
    l3->addStretch(10);
    l3->addWidget(d->resetButton);

    // -------------------------------------------------------------

    QGridLayout* const mainLayout = new QGridLayout();
    mainLayout->addWidget(curveBox, 0, 0, 1, 1);
    mainLayout->addLayout(l3,       1, 0, 1, 1);
    mainLayout->setRowStretch(2, 10);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    setLayout(mainLayout);

    // default: disable all control widgets
    enableHGradient(false);
    enableVGradient(false);
    enableControlWidgets(false);

    // -------------------------------------------------------------

    connect(d->curvesWidget, SIGNAL(signalCurvesChanged()),
            this, SIGNAL(signalCurvesChanged()));

    connect(d->pickerType, SIGNAL(buttonReleased(int)),
            this, SIGNAL(signalPickerChanged(int)));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetChannel()));

    connect(d->curveType, SIGNAL(buttonClicked(int)),
            this, SLOT(slotCurveTypeChanged(int)));
}

CurvesBox::~CurvesBox()
{
    delete d;
}

void CurvesBox::enablePickers(bool enable)
{
    d->pickBlack->setVisible(enable);
    d->pickGray->setVisible(enable);
    d->pickWhite->setVisible(enable);
}

void CurvesBox::enableHGradient(bool enable)
{
    d->hGradient->setVisible(enable);
}

void CurvesBox::enableVGradient(bool enable)
{
    d->vGradient->setVisible(enable);
}

void CurvesBox::enableGradients(bool enable)
{
    enableHGradient(enable);
    enableVGradient(enable);
}

void CurvesBox::enableResetButton(bool enable)
{
    d->resetButton->setVisible(enable);
}

void CurvesBox::enableCurveTypes(bool enable)
{
    d->curveFree->setVisible(enable);
    d->curveSmooth->setVisible(enable);
}

void CurvesBox::enableControlWidgets(bool enable)
{
    enablePickers(enable);
    enableResetButton(enable);
    enableCurveTypes(enable);
}

void CurvesBox::slotCurveTypeChanged(int type)
{
    switch (type)
    {
        case SmoothDrawing:
        {
            d->curvesWidget->curves()->setCurveType(d->channel, ImageCurves::CURVE_SMOOTH);
            d->pickerBox->setEnabled(true);
            break;
        }

        case FreeDrawing:
        {
            d->curvesWidget->curves()->setCurveType(d->channel, ImageCurves::CURVE_FREE);
            d->pickerBox->setEnabled(false);
            break;
        }
    }

    d->curvesWidget->curveTypeChanged();
    emit signalCurveTypeChanged(type);
}

void CurvesBox::setScale(HistogramScale type)
{
    d->curvesWidget->setScaleType(type);
}

void CurvesBox::setChannel(ChannelType channel)
{
    d->channel = channel;
    d->curvesWidget->setChannelType(channel);

    switch (channel)
    {
        case RedChannel:
            d->hGradient->setColors(QColor("black"), QColor("red"));
            d->vGradient->setColors(QColor("red"), QColor("black"));
            break;

        case GreenChannel:
            d->hGradient->setColors(QColor("black"), QColor("green"));
            d->vGradient->setColors(QColor("green"), QColor("black"));
            break;

        case BlueChannel:
            d->hGradient->setColors(QColor("black"), QColor("blue"));
            d->vGradient->setColors(QColor("blue"), QColor("black"));
            break;

        default:
            d->hGradient->setColors(QColor("black"), QColor("white"));
            d->vGradient->setColors(QColor("white"), QColor("black"));
            break;
    }

    d->curveType->button(d->curvesWidget->curves()->getCurveType(channel))->setChecked(true);
}

ChannelType CurvesBox::channel() const
{
    return d->channel;
}

int CurvesBox::picker() const
{
    return d->pickerType->checkedId();
}

void CurvesBox::resetPickers()
{
    d->pickerType->setExclusive(false);
    d->pickBlack->setChecked(false);
    d->pickGray->setChecked(false);
    d->pickWhite->setChecked(false);
    d->pickerType->setExclusive(true);
    emit signalPickerChanged(NoPicker);
}

void CurvesBox::resetChannel(int channel)
{
    d->curvesWidget->curves()->curvesChannelReset(channel);
    d->curvesWidget->repaint();
}

void CurvesBox::slotResetChannel()
{
    resetChannel(d->channel);
    emit signalChannelReset(d->channel);
}

void CurvesBox::slotResetChannels()
{
    resetChannels();
}

void CurvesBox::resetChannels()
{
    for (int channel = 0; channel < ImageCurves::NUM_CHANNELS; ++channel)
    {
        d->curvesWidget->curves()->curvesChannelReset(channel);
    }

    reset();
}

void CurvesBox::reset()
{
    d->curvesWidget->curves()->setCurveType(d->channel, ImageCurves::CURVE_SMOOTH);
    d->curvesWidget->reset();
}

void CurvesBox::readCurveSettings(KConfigGroup& group, const QString& prefix)
{
    d->curvesWidget->restoreCurve(group, prefix);
}

void CurvesBox::writeCurveSettings(KConfigGroup& group, const QString& prefix)
{
    d->curvesWidget->saveCurve(group, prefix);
}

ImageCurves* CurvesBox::curves() const
{
    return d->curvesWidget->curves();
}

void CurvesBox::setCurveGuide(const DColor& color)
{
    d->curvesWidget->setCurveGuide(color);
}

int CurvesBox::curvesLeftOffset() const
{
    // width of spacer column between gradient and curves
    int offset = 2;

    if (!d->vGradient->isHidden())
    {
        offset += d->vGradient->width();
    }

    return offset;
}

} // namespace Digikam
