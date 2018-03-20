/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-12-28
 * Description : crop image batch tool.
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "crop.h"

// Qt includes

#include <QLabel>
#include <QRect>
#include <QWidget>
#include <QGridLayout>
#include <QCheckBox>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "autocrop.h"
#include "dimg.h"
#include "dimgbuiltinfilter.h"

namespace Digikam
{

class Crop::Private
{
public:

    Private() :
        autoCrop(0),
        heightInput(0),
        widthInput(0),
        xInput(0),
        yInput(0),
        changeSettings(true)
    {
    }

public:

    QCheckBox*    autoCrop;

    DIntNumInput* heightInput;
    DIntNumInput* widthInput;
    DIntNumInput* xInput;
    DIntNumInput* yInput;

    bool          changeSettings;
};

Crop::Crop(QObject* const parent)
    : BatchTool(QLatin1String("Crop"), TransformTool, parent),
      d(new Private)
{
    setToolTitle(i18n("Crop"));
    setToolDescription(i18n("Crop images to a region."));
    setToolIconName(QLatin1String("transform-crop"));
}

Crop::~Crop()
{
    delete d;
}

void Crop::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    // -------------------------------------------------------------

    d->autoCrop = new QCheckBox(i18n("Auto-Crop"), m_settingsWidget);
    d->autoCrop->setWhatsThis(i18n("Compute automatically crop area settings by a parse of "
                                   "black hole image borders."));

    QLabel* const positionLabel = new QLabel(i18n("Position:"), m_settingsWidget);
    positionLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

    d->xInput = new DIntNumInput(m_settingsWidget);
    d->xInput->setWhatsThis( i18n("Set here the top left selection corner position for cropping."));
    d->xInput->setRange(0, 99999, 1);
    d->xInput->setDefaultValue(50);

    d->yInput = new DIntNumInput(m_settingsWidget);
    d->yInput->setWhatsThis( i18n("Set here the top left selection corner position for cropping."));
    d->yInput->setRange(0, 99999, 1);
    d->yInput->setDefaultValue(50);

    // -------------------------------------------------------------

    QLabel* const sizeLabel = new QLabel(i18n("Size:"), m_settingsWidget);
    sizeLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

    d->widthInput = new DIntNumInput(m_settingsWidget);
    d->widthInput->setWhatsThis( i18n("Set here the width selection for cropping."));
    d->widthInput->setRange(0, 99999, 1);
    d->widthInput->setDefaultValue(800);

    d->heightInput = new DIntNumInput(m_settingsWidget);
    d->heightInput->setWhatsThis( i18n("Set here the height selection for cropping."));
    d->heightInput->setRange(0, 99999, 1);
    d->heightInput->setDefaultValue(600);

    // -------------------------------------------------------------

    QGridLayout* const mainLayout = new QGridLayout(m_settingsWidget);
    mainLayout->addWidget(d->autoCrop,                  0, 0, 1, 1);
    mainLayout->addWidget(positionLabel,                1, 0, 1, 1);
    mainLayout->addWidget(d->xInput,                    1, 1, 1, 3);
    mainLayout->addWidget(d->yInput,                    2, 1, 1, 3);
    mainLayout->addWidget(sizeLabel,                    3, 0, 1, 1);
    mainLayout->addWidget(d->widthInput,                3, 1, 1, 3);
    mainLayout->addWidget(d->heightInput,               4, 1, 1, 3);
    mainLayout->addWidget(new QLabel(m_settingsWidget), 5, 1, 1, 3);
    mainLayout->setRowStretch(5, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->autoCrop, SIGNAL(clicked(bool)),
            this, SLOT(slotDisableParameters(bool)));

    connect(this, SIGNAL(signalAutoCrop()),
            this, SLOT(slotSettingsChanged()));

    connect(d->xInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->yInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->widthInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->heightInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));;

    BatchTool::registerSettingsWidget();
}

void Crop::slotDisableParameters(bool b)
{
    d->xInput->setDisabled(b);
    d->yInput->setDisabled(b);
    d->widthInput->setDisabled(b);
    d->heightInput->setDisabled(b);

    if (b)
        emit signalAutoCrop();
}

BatchToolSettings Crop::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("xInput"),      50);
    settings.insert(QLatin1String("yInput"),      50);
    settings.insert(QLatin1String("widthInput"),  800);
    settings.insert(QLatin1String("heightInput"), 600);
    settings.insert(QLatin1String("AutoCrop"),    false);
    return settings;
}

void Crop::slotAssignSettings2Widget()
{
    d->changeSettings = false;
    d->xInput->setValue(settings()[QLatin1String("xInput")].toInt());
    d->yInput->setValue(settings()[QLatin1String("yInput")].toInt());
    d->widthInput->setValue(settings()[QLatin1String("widthInput")].toInt());
    d->heightInput->setValue(settings()[QLatin1String("heightInput")].toInt());
    d->autoCrop->setChecked(settings()[QLatin1String("AutoCrop")].toBool());
    d->changeSettings = true;
}

void Crop::slotSettingsChanged()
{
    if (d->changeSettings)
    {
        BatchToolSettings settings;
        settings.insert(QLatin1String("xInput"),      d->xInput->value());
        settings.insert(QLatin1String("yInput"),      d->yInput->value());
        settings.insert(QLatin1String("widthInput"),  d->widthInput->value());
        settings.insert(QLatin1String("heightInput"), d->heightInput->value());
        settings.insert(QLatin1String("AutoCrop"),    d->autoCrop->isChecked());
        BatchTool::slotSettingsChanged(settings);
    }
}

bool Crop::toolOperations()
{
    int xInput      = settings()[QLatin1String("xInput")].toInt();
    int yInput      = settings()[QLatin1String("yInput")].toInt();
    int widthInput  = settings()[QLatin1String("widthInput")].toInt();
    int heightInput = settings()[QLatin1String("heightInput")].toInt();
    bool autoCrop   = settings()[QLatin1String("AutoCrop")].toBool();

    if (!loadToDImg())
    {
        return false;
    }

    QRect rect(xInput, yInput, widthInput, heightInput);

    if (autoCrop)
    {
        AutoCrop ac(&image());
        ac.startFilterDirectly();
        rect = ac.autoInnerCrop();
    }

    if (!rect.isValid())
    {
        return false;
    }

    DImgBuiltinFilter filter(DImgBuiltinFilter::Crop, rect);
    applyFilter(&filter);

    return (savefromDImg());
}

}  // namespace Digikam
