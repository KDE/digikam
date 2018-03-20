/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-30
 * Description : a widget to display an image histogram and its control widgets
 *
 * Copyright (C) 2008-2009 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "histogrambox.h"

// Qt includes

#include <QButtonGroup>
#include <QColor>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QPair>
#include <QString>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QComboBox>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "colorgradientwidget.h"
#include "histogramwidget.h"
#include "digikam_globals.h"

namespace Digikam
{

class HistogramBox::Private
{

public:

    Private()
    {
        scaleBG         = 0;
        linHistoButton  = 0;
        logHistoButton  = 0;
        channelCB       = 0;
        hGradient       = 0;
        histogramWidget = 0;
        histoBox        = 0;
    }

    QButtonGroup*        scaleBG;

    QToolButton*         linHistoButton;
    QToolButton*         logHistoButton;

    QWidget*             histoBox;
    QComboBox*           channelCB;

    ColorGradientWidget* hGradient;
    HistogramWidget*     histogramWidget;
};

HistogramBox::HistogramBox(QWidget* const parent, HistogramBoxType type, bool selectMode)
    : QWidget(parent), d(new Private)
{
    d->channelCB               = new QComboBox(this);
    QLabel* const channelLabel = new QLabel(i18n("Channel:"), this);
    channelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QWidget* const scaleBox = new QWidget(this);
    QHBoxLayout* const hlay = new QHBoxLayout(scaleBox);
    d->scaleBG              = new QButtonGroup(scaleBox);
    scaleBox->setWhatsThis(i18n("<p>Select the histogram scale.</p>"
                                "<p>If the image's maximal counts are small, you can use the <b>linear</b> scale.</p>"
                                "<p><b>Logarithmic</b> scale can be used when the maximal counts are big; "
                                "if it is used, all values (small and large) will be visible on the graph.</p>"));

    d->linHistoButton = new QToolButton(scaleBox);
    d->linHistoButton->setToolTip(i18nc("linear histogram scaling mode", "Linear"));
    d->linHistoButton->setIcon(QIcon::fromTheme(QLatin1String("view-object-histogram-linear")));
    d->linHistoButton->setCheckable(true);
    d->scaleBG->addButton(d->linHistoButton, LinScaleHistogram);

    d->logHistoButton = new QToolButton(scaleBox);
    d->logHistoButton->setToolTip(i18nc("logarithmic histogram scaling mode", "Logarithmic"));
    d->logHistoButton->setIcon(QIcon::fromTheme(QLatin1String("view-object-histogram-logarithmic")));
    d->logHistoButton->setCheckable(true);
    d->scaleBG->addButton(d->logHistoButton, LogScaleHistogram);

    hlay->setSpacing(0);
    hlay->setContentsMargins(QMargins());
    hlay->addWidget(d->linHistoButton);
    hlay->addWidget(d->logHistoButton);

    d->scaleBG->setExclusive(true);
    d->logHistoButton->setChecked(true);

    d->histoBox                       = new QWidget;
    QVBoxLayout* const histoBoxLayout = new QVBoxLayout;

    d->histogramWidget = new HistogramWidget(256, 140, d->histoBox, selectMode, true, true);
    d->histogramWidget->setWhatsThis(i18n("Here you can see the target preview image histogram drawing "
                                          "of the selected image channel. This one is re-computed at any "
                                          "settings changes."));

    d->hGradient = new ColorGradientWidget(Qt::Horizontal, 10, d->histoBox);
    d->hGradient->setColors(QColor("black"), QColor("white"));

    histoBoxLayout->addWidget(d->histogramWidget);
    histoBoxLayout->addWidget(d->hGradient);
    histoBoxLayout->setContentsMargins(QMargins());
    histoBoxLayout->setSpacing(1);
    d->histoBox->setLayout(histoBoxLayout);

    QGridLayout* const mainLayout = new QGridLayout;
    mainLayout->addWidget(channelLabel,   0, 0, 1, 1);
    mainLayout->addWidget(d->channelCB,   0, 1, 1, 1);
    mainLayout->addWidget(scaleBox,       0, 3, 1, 2);
    mainLayout->addWidget(d->histoBox,    2, 0, 1, 5);
    mainLayout->setColumnStretch(2, 10);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->setSpacing(5);
    setLayout(mainLayout);

    // ---------------------------------------------------------------

    setHistogramType(type);

    // ---------------------------------------------------------------

    connect(d->channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged()));

    connect(d->scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged()));

    connect(this, SIGNAL(signalChannelChanged(ChannelType)),
            d->histogramWidget, SLOT(setChannelType(ChannelType)));

    connect(this, SIGNAL(signalScaleChanged(HistogramScale)),
            d->histogramWidget, SLOT(setScaleType(HistogramScale)));
}

HistogramBox::~HistogramBox()
{
    histogram()->stopHistogramComputation();
    delete d;
}

void HistogramBox::setGradientVisible(bool visible)
{
    d->hGradient->setVisible(visible);
}

void HistogramBox::setGradientColors(const QColor& from, const QColor& to)
{
    d->hGradient->setColors(from, to);
}

void HistogramBox::setStatisticsVisible(bool b)
{
    d->histogramWidget->setStatisticsVisible(b);
}

ChannelType HistogramBox::channel() const
{
    int index = d->channelCB->currentIndex();
    return (ChannelType)(d->channelCB->itemData(index).toInt());
}

void HistogramBox::setChannel(ChannelType channel)
{
    int id = d->channelCB->findData(QVariant(channel));
    d->channelCB->setCurrentIndex(id);
    slotChannelChanged();
}

void HistogramBox::setChannelEnabled(bool enabled)
{
    d->channelCB->setEnabled(enabled);
}

HistogramScale HistogramBox::scale() const
{
    return static_cast<HistogramScale>(d->scaleBG->checkedId());
}

void HistogramBox::setScale(HistogramScale scale)
{
    d->scaleBG->button((int)scale)->setChecked(true);
    slotScaleChanged();
}

HistogramWidget* HistogramBox::histogram() const
{
    return d->histogramWidget;
}

void HistogramBox::setHistogramMargin(int margin)
{
    d->histoBox->layout()->setContentsMargins(margin, margin, margin, margin);
}

void HistogramBox::slotChannelChanged()
{
    switch (channel())
    {
        case LuminosityChannel:
            setGradientColors(QColor("black"), QColor("white"));
            break;

        case RedChannel:
            setGradientColors(QColor("black"), QColor("red"));
            break;

        case GreenChannel:
            setGradientColors(QColor("black"), QColor("green"));
            break;

        case BlueChannel:
            setGradientColors(QColor("black"), QColor("blue"));
            break;

        case AlphaChannel:
            setGradientColors(QColor("black"), QColor("white"));
            break;

        case ColorChannels:
            setGradientColors(QColor("black"), QColor("white"));
            break;
    }

    emit(signalChannelChanged(channel()));
}

void HistogramBox::slotScaleChanged()
{
    emit(signalScaleChanged(scale()));
}

void HistogramBox::setHistogramType(HistogramBoxType type)
{
    // all possible channels for histogram widget are defined in this map
    QMap<int, QPair<QString, QString> > channelDescMap;

    // this string will contain the WhatsThis message for the channelCB
    QString channelCBDescr(i18n("<p>Select the histogram channel to display:</p>"));

    // those pairs hold the combobox text and WhatsThis description for each channel item
    typedef QPair<QString, QString> ChannelPair;

    ChannelPair luminosityPair(i18nc("The luminosity channel", "Luminosity"), i18n(
                                   "<b>Luminosity</b>: display the image's luminosity values."));

    ChannelPair redPair(i18nc("The red channel", "Red"), i18n(
                            "<b>Red</b>: display the red image-channel values."));

    ChannelPair greenPair(i18nc("The green channel", "Green"), i18n(
                              "<b>Green</b>: display the green image-channel values."));

    ChannelPair bluePair(i18nc("The blue channel", "Blue"), i18n(
                             "<b>Blue</b>: display the blue image-channel values."));

    ChannelPair colorsPair(i18nc("The colors channel", "Colors"), i18n(
                               "<b>Colors</b>: Display all color channel values at the same time."));

    ChannelPair alphaPair(i18nc("The alpha channel", "Alpha"), i18n(
                              "<b>Alpha</b>: display the alpha image-channel values. "
                              "This channel corresponds to the transparency value and "
                              "is supported by some image formats, such as PNG or TIF."));

    channelDescMap.insert(LuminosityChannel, luminosityPair);
    channelDescMap.insert(RedChannel, redPair);
    channelDescMap.insert(GreenChannel, greenPair);
    channelDescMap.insert(BlueChannel, bluePair);
    channelDescMap.insert(ColorChannels, colorsPair);
    channelDescMap.insert(AlphaChannel, alphaPair);

    switch (type)
    {
        case RGB:
            d->channelCB->clear();
            d->channelCB->addItem(channelDescMap[RedChannel].first, QVariant(RedChannel));
            d->channelCB->addItem(channelDescMap[GreenChannel].first, QVariant(GreenChannel));
            d->channelCB->addItem(channelDescMap[BlueChannel].first, QVariant(BlueChannel));
            channelCBDescr.append(QLatin1String("<p>"));
            channelCBDescr.append(channelDescMap[RedChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[GreenChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[BlueChannel].second);
            channelCBDescr.append(QLatin1String("</p>"));
            break;

        case RGBA:
            d->channelCB->clear();
            d->channelCB->addItem(channelDescMap[RedChannel].first, QVariant(RedChannel));
            d->channelCB->addItem(channelDescMap[GreenChannel].first, QVariant(GreenChannel));
            d->channelCB->addItem(channelDescMap[BlueChannel].first, QVariant(BlueChannel));
            d->channelCB->addItem(channelDescMap[AlphaChannel].first, QVariant(AlphaChannel));
            channelCBDescr.append(QLatin1String("<p>"));
            channelCBDescr.append(channelDescMap[RedChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[GreenChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[BlueChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[AlphaChannel].second);
            channelCBDescr.append(QLatin1String("</p>"));
            break;

        case LRGB:
            d->channelCB->clear();
            d->channelCB->addItem(channelDescMap[LuminosityChannel].first, QVariant(LuminosityChannel));
            d->channelCB->addItem(channelDescMap[RedChannel].first, QVariant(RedChannel));
            d->channelCB->addItem(channelDescMap[GreenChannel].first, QVariant(GreenChannel));
            d->channelCB->addItem(channelDescMap[BlueChannel].first, QVariant(BlueChannel));
            channelCBDescr.append(QLatin1String("<p>"));
            channelCBDescr.append(channelDescMap[LuminosityChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[RedChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[GreenChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[BlueChannel].second);
            channelCBDescr.append(QLatin1String("</p>"));
            break;

        case LRGBA:
            d->channelCB->clear();
            d->channelCB->addItem(channelDescMap[LuminosityChannel].first, QVariant(LuminosityChannel));
            d->channelCB->addItem(channelDescMap[RedChannel].first, QVariant(RedChannel));
            d->channelCB->addItem(channelDescMap[GreenChannel].first, QVariant(GreenChannel));
            d->channelCB->addItem(channelDescMap[BlueChannel].first, QVariant(BlueChannel));
            d->channelCB->addItem(channelDescMap[AlphaChannel].first, QVariant(AlphaChannel));
            channelCBDescr.append(QLatin1String("<p>"));
            channelCBDescr.append(channelDescMap[LuminosityChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[RedChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[GreenChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[BlueChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[AlphaChannel].second);
            channelCBDescr.append(QLatin1String("</p>"));
            break;

        case LRGBC:
            d->channelCB->clear();
            d->channelCB->addItem(channelDescMap[LuminosityChannel].first, QVariant(LuminosityChannel));
            d->channelCB->addItem(channelDescMap[RedChannel].first, QVariant(RedChannel));
            d->channelCB->addItem(channelDescMap[GreenChannel].first, QVariant(GreenChannel));
            d->channelCB->addItem(channelDescMap[BlueChannel].first, QVariant(BlueChannel));
            d->channelCB->addItem(channelDescMap[ColorChannels].first, QVariant(ColorChannels));
            channelCBDescr.append(QLatin1String("<p>"));
            channelCBDescr.append(channelDescMap[LuminosityChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[RedChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[GreenChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[BlueChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[ColorChannels].second);
            channelCBDescr.append(QLatin1String("</p>"));
            break;

        case LRGBAC:
            d->channelCB->clear();
            d->channelCB->addItem(channelDescMap[LuminosityChannel].first, QVariant(LuminosityChannel));
            d->channelCB->addItem(channelDescMap[RedChannel].first, QVariant(RedChannel));
            d->channelCB->addItem(channelDescMap[GreenChannel].first, QVariant(GreenChannel));
            d->channelCB->addItem(channelDescMap[BlueChannel].first, QVariant(BlueChannel));
            d->channelCB->addItem(channelDescMap[AlphaChannel].first, QVariant(AlphaChannel));
            d->channelCB->addItem(channelDescMap[ColorChannels].first, QVariant(ColorChannels));
            channelCBDescr.append(QLatin1String("<p>"));
            channelCBDescr.append(channelDescMap[LuminosityChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[RedChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[GreenChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[BlueChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[AlphaChannel].second).append(QLatin1String("<br/>"));
            channelCBDescr.append(channelDescMap[ColorChannels].second);
            channelCBDescr.append(QLatin1String("</p>"));
            break;

        default:
            break;
    }

    d->channelCB->setWhatsThis(channelCBDescr);
}

} // namespace Digikam
