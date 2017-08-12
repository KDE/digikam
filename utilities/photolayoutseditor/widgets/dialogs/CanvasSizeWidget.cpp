/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011-2012 by Lukasz Spas <lukasz dot spas at gmail dot com>
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

#include "CanvasSizeWidget.h"

#include <QComboBox>
#include <QPushButton>
#include <klocalizedstring.h>

#include <QDoubleSpinBox>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

#include "CanvasSize.h"

using namespace PhotoLayoutsEditor;

class CanvasSizeWidget::Private
{
    void swapSizes();
    void updateSizeLabel();

    QWidget *   sizeWidget;
    QWidget *   advancedWidget;
    QDoubleSpinBox *  xSize;
    QDoubleSpinBox *  ySize;
    QLabel *    sizeLabel;
    QComboBox * sizeUnitsWidget;
    QDoubleSpinBox *  xResolution;
    QDoubleSpinBox *  yResolution;
    QComboBox * resolutionUnitsWidget;

    static int WIDTH;
    static int HEIGHT;
    static QString currentSizeUnit;

    static qreal WIDTH_RES;
    static qreal HEIGHT_RES;
    static QString currentResolutionUnit;

    friend class CanvasSizeWidget;
};

int CanvasSizeWidget::Private::WIDTH = 800;
int CanvasSizeWidget::Private::HEIGHT = 800;
QString CanvasSizeWidget::Private::currentSizeUnit = QString();
qreal CanvasSizeWidget::Private::WIDTH_RES = 72;
qreal CanvasSizeWidget::Private::HEIGHT_RES = 72;
QString CanvasSizeWidget::Private::currentResolutionUnit = QString();

void CanvasSizeWidget::Private::swapSizes()
{
    // swap dimensions
    qreal temp = WIDTH;
    WIDTH = HEIGHT;
    HEIGHT = temp;
    temp = xSize->value();
    xSize->setValue( ySize->value() );
    ySize->setValue( temp );

    // Swap resolutions
    temp = WIDTH_RES;
    WIDTH_RES = HEIGHT_RES;
    HEIGHT_RES = temp;
    temp = xResolution->value();
    xResolution->setValue(yResolution->value());
    yResolution->setValue(temp);
}

void CanvasSizeWidget::Private::updateSizeLabel()
{
    sizeLabel->setText(QString::number(WIDTH)
                        .append(QLatin1String(" x "))
                        .append(QString::number(HEIGHT)
                        .append(QLatin1String(" px"))));
}

CanvasSizeWidget::CanvasSizeWidget(QWidget *parent) :
    QWidget(parent),
    d(new Private)
{
    setupUI(QSize(d->WIDTH, d->HEIGHT),
            d->currentSizeUnit,
            QSize(d->WIDTH_RES, d->HEIGHT_RES),
            d->currentResolutionUnit);
}

CanvasSizeWidget::Orientation CanvasSizeWidget::orientation() const
{
    if (d->WIDTH < d->HEIGHT)
        return CanvasSizeWidget::Vertical;
    else
        return CanvasSizeWidget::Horizontal;
}

void CanvasSizeWidget::setupUI(const QSizeF & size, const QString & sizeUnits, const QSizeF & resolution, const QString & resolutionUnits)
{
    QString tempSizeUnits = sizeUnits;
    QString tempResolutionUnits = resolutionUnits;
    if (tempSizeUnits.isEmpty() ||
            CanvasSize::sizeUnit(tempSizeUnits) == CanvasSize::UnknownSizeUnit)
        tempSizeUnits = CanvasSize::sizeUnitName(CanvasSize::Pixels);;
    if (tempResolutionUnits.isEmpty() ||
            CanvasSize::resolutionUnit(tempResolutionUnits) == CanvasSize::UnknownResolutionUnit)
        tempResolutionUnits = CanvasSize::resolutionUnitName(CanvasSize::PixelsPerInch);;

    QVBoxLayout * vLayout = new  QVBoxLayout();
    this->setLayout(vLayout);

    /// ----------------------- CANVAS SIZE PART -----------------------
    d->sizeWidget = new QGroupBox(i18n("Canvas size"), this);
    vLayout->addWidget(d->sizeWidget);

    QGridLayout * gridLayout = new QGridLayout(d->sizeWidget);
    d->sizeWidget->setLayout(gridLayout);

    // Width widget
    d->xSize = new QDoubleSpinBox(d->sizeWidget);
    d->xSize->setMinimum(0.00001);
    d->xSize->setMaximum(999999);
    d->xSize->setValue(size.width());
    d->WIDTH = CanvasSize::toPixels(size.width(),
                                    resolution.width(),
                                    CanvasSize::sizeUnit(tempSizeUnits),
                                    CanvasSize::resolutionUnit(tempResolutionUnits));
    gridLayout->addWidget(new QLabel(i18n("Width"), d->sizeWidget),0,0);
    gridLayout->addWidget(d->xSize,0,1);

    // Height widget
    d->ySize = new QDoubleSpinBox(d->sizeWidget);
    d->ySize->setMinimum(0.00001);
    d->ySize->setMaximum(999999);
    d->ySize->setValue(size.height());
    d->HEIGHT = CanvasSize::toPixels(size.height(),
                                     resolution.height(),
                                     CanvasSize::sizeUnit(tempSizeUnits),
                                     CanvasSize::resolutionUnit(tempResolutionUnits));
    gridLayout->addWidget(new QLabel(i18n("Height"), d->sizeWidget),1,0);
    gridLayout->addWidget(d->ySize,1,1);

    // Unit widget
    d->sizeUnitsWidget = new QComboBox(d->sizeWidget);
    d->sizeUnitsWidget->addItems(CanvasSize::sizeUnitsNames());
    d->sizeUnitsWidget->setCurrentText(tempSizeUnits);
    d->currentSizeUnit = tempSizeUnits;
    gridLayout->addWidget(d->sizeUnitsWidget,1,2);

    // Size label
    d->sizeLabel = new QLabel(d->sizeWidget);
    gridLayout->addWidget(d->sizeLabel,2, 2);

    /// ----------------------- ADVANCED PART -----------------------
    d->advancedWidget = new QGroupBox(i18n("Advanced"), this);
    vLayout->addWidget(d->advancedWidget);
    gridLayout = new QGridLayout(d->advancedWidget);
    d->advancedWidget->setLayout(gridLayout);

    // x resolution widget
    d->xResolution = new QDoubleSpinBox(d->advancedWidget);
    d->xResolution->setMinimum(0);
    d->xResolution->setMaximum(999999);
    d->xResolution->setValue(resolution.width());
    d->xResolution->setDecimals(3);
    d->WIDTH_RES = resolution.width() * CanvasSize::resolutionUnitFactor(tempResolutionUnits);
    gridLayout->addWidget(new QLabel(i18n("Resolution X"), d->advancedWidget),0,0);
    gridLayout->addWidget(d->xResolution,0,1);

    // y resolution widget
    d->yResolution = new QDoubleSpinBox(d->advancedWidget);
    d->yResolution->setMinimum(0);
    d->yResolution->setMaximum(999999);
    d->yResolution->setValue(resolution.height());
    d->yResolution->setDecimals(3);
    d->HEIGHT_RES = resolution.height() * CanvasSize::resolutionUnitFactor(tempResolutionUnits);
    gridLayout->addWidget(new QLabel(i18n("Resolution Y"), d->advancedWidget),1,0);
    gridLayout->addWidget(d->yResolution,1,1);

    // Unit widget
    d->resolutionUnitsWidget = new QComboBox(d->sizeWidget);
    d->resolutionUnitsWidget->addItems(CanvasSize::resolutionUnitsNames());
    d->resolutionUnitsWidget->setCurrentText(tempResolutionUnits);
    d->currentResolutionUnit = tempResolutionUnits;
    gridLayout->addWidget(d->resolutionUnitsWidget,1,2);

    this->prepareSignalsConnections();

    d->updateSizeLabel();
}

CanvasSizeWidget::~CanvasSizeWidget()
{
    delete d;
}

void CanvasSizeWidget::prepareSignalsConnections()
{
    connect(d->xSize, SIGNAL(valueChanged(double)), this, SLOT(widthChanged(double)));
    connect(d->ySize, SIGNAL(valueChanged(double)), this, SLOT(heightChanged(double)));
    connect(d->sizeUnitsWidget, SIGNAL(activated(QString)), this, SLOT(sizeUnitsChanged(QString)));
    connect(d->xResolution, SIGNAL(valueChanged(double)), this, SLOT(xResolutionChanged(double)));
    connect(d->yResolution, SIGNAL(valueChanged(double)), this, SLOT(yResolutionChanged(double)));
    connect(d->resolutionUnitsWidget, SIGNAL(currentIndexChanged(QString)), this, SLOT(resolutionUnitsChanged(QString)));
}

CanvasSize CanvasSizeWidget::canvasSize() const
{
    CanvasSize result(QSizeF(d->xSize->value(), d->ySize->value()),
                      CanvasSize::sizeUnit(d->sizeUnitsWidget->currentText()),
                      QSizeF(d->xResolution->value(), d->yResolution->value()),
                      CanvasSize::resolutionUnit(d->resolutionUnitsWidget->currentText()));
    return result;
}

void CanvasSizeWidget::sizeUnitsChanged(const QString & unitName)
{
    d->currentSizeUnit = unitName;
    CanvasSize::SizeUnits sizeUnit = CanvasSize::sizeUnit(unitName);
    if (sizeUnit == CanvasSize::Pixels)
    {
        d->xSize->setValue(d->WIDTH);
        d->ySize->setValue(d->HEIGHT);
        d->xSize->setDecimals(0);
        d->ySize->setDecimals(0);
        return;
    }
    d->xSize->setDecimals(5);
    d->ySize->setDecimals(5);
    CanvasSize::ResolutionUnits resolutionUnit = CanvasSize::resolutionUnit(d->resolutionUnitsWidget->currentText());
    qreal WIDTH = CanvasSize::fromPixels(d->WIDTH,
                                         d->xResolution->value(),
                                         sizeUnit,
                                         resolutionUnit);
    qreal HEIGHT = CanvasSize::fromPixels(d->HEIGHT,
                                          d->yResolution->value(),
                                          sizeUnit,
                                          resolutionUnit);
    d->xSize->setValue(WIDTH);
    d->ySize->setValue(HEIGHT);
}

void CanvasSizeWidget::resolutionUnitsChanged(const QString & unitName)
{
    d->currentResolutionUnit = unitName;
    CanvasSize::ResolutionUnits unit = CanvasSize::resolutionUnit(unitName);
    if (unit == CanvasSize::PixelsPerInch)
    {
        d->xResolution->setValue(d->WIDTH_RES);
        d->yResolution->setValue(d->HEIGHT_RES);
        return;
    }
    qreal factor = CanvasSize::resolutionUnitFactor(unit);
    d->xResolution->setValue(d->WIDTH_RES / factor);
    d->yResolution->setValue(d->HEIGHT_RES / factor);
}

void CanvasSizeWidget::setHorizontal(bool isHorizontal)
{
    if (isHorizontal)
    {
        if (d->WIDTH < d->HEIGHT)
        {
            d->swapSizes();
            d->updateSizeLabel();
        }
        emit orientationChanged();
    }
}

void CanvasSizeWidget::setVertical(bool isVertical)
{
    if (isVertical)
    {
        if (d->HEIGHT < d->WIDTH)
        {
            d->swapSizes();
            d->updateSizeLabel();
        }
        emit orientationChanged();
    }
}

void CanvasSizeWidget::widthChanged(double width)
{
    width = CanvasSize::toPixels(width,
                                 d->xResolution->value(),
                                 CanvasSize::sizeUnit(d->sizeUnitsWidget->currentText()),
                                 CanvasSize::resolutionUnit(d->resolutionUnitsWidget->currentText()));
    d->WIDTH = width;
    this->setHorizontal(d->WIDTH > d->HEIGHT);
    this->setVertical(d->WIDTH < d->HEIGHT);
    d->updateSizeLabel();
}

void CanvasSizeWidget::heightChanged(double height)
{
    height = CanvasSize::toPixels(height,
                                  d->yResolution->value(),
                                  CanvasSize::sizeUnit(d->sizeUnitsWidget->currentText()),
                                  CanvasSize::resolutionUnit(d->resolutionUnitsWidget->currentText()));
    d->HEIGHT = height;
    this->setHorizontal(d->WIDTH > d->HEIGHT);
    this->setVertical(d->WIDTH < d->HEIGHT);
    d->updateSizeLabel();
}

void CanvasSizeWidget::xResolutionChanged(double xResolution)
{
    CanvasSize::SizeUnits sizeUnit = CanvasSize::sizeUnit(d->sizeUnitsWidget->currentText());
    if (sizeUnit == CanvasSize::Pixels)
        return;
    CanvasSize::ResolutionUnits resolutionUnit = CanvasSize::resolutionUnit(d->resolutionUnitsWidget->currentText());
    qreal resolutionFactor = CanvasSize::resolutionUnitFactor(resolutionUnit);
    int width = CanvasSize::toPixels(d->xSize->value(),
                                     xResolution,
                                     CanvasSize::sizeUnit(d->sizeUnitsWidget->currentText()),
                                     CanvasSize::resolutionUnit(d->resolutionUnitsWidget->currentText()));
    d->WIDTH = width;
    d->WIDTH_RES = xResolution * resolutionFactor;
    d->updateSizeLabel();
}

void CanvasSizeWidget::yResolutionChanged(double yResolution)
{
    CanvasSize::SizeUnits sizeUnit = CanvasSize::sizeUnit(d->sizeUnitsWidget->currentText());
    if (sizeUnit == CanvasSize::Pixels)
        return;
    CanvasSize::ResolutionUnits resolutionUnit = CanvasSize::resolutionUnit(d->resolutionUnitsWidget->currentText());
    qreal resolutionFactor = CanvasSize::resolutionUnitFactor(resolutionUnit);
    int height = CanvasSize::toPixels(d->ySize->value(),
                                      yResolution,
                                      CanvasSize::sizeUnit(d->sizeUnitsWidget->currentText()),
                                      CanvasSize::resolutionUnit(d->resolutionUnitsWidget->currentText()));
    d->HEIGHT = height;
    d->HEIGHT_RES = yResolution * resolutionFactor;
    d->updateSizeLabel();
}
