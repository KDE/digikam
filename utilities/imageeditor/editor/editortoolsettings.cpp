/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-21
 * Description : Editor tool settings template box
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2009 by Andi Clemens <andi dot clemens at gmx dot net>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "editortoolsettings.h"
#include "editortoolsettings.moc"

// Qt includes

#include <QButtonGroup>
#include <QLabel>
#include <QLayout>
#include <QMap>
#include <QPair>
#include <QString>
#include <QToolButton>
#include <QVariant>

// KDE includes

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <khbox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kstandardguiitem.h>
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "colorgradientwidget.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imagepaniconwidget.h"
#include "globals.h"

using namespace KDcrawIface;

namespace Digikam
{

class EditorToolSettingsPriv
{

public:

    EditorToolSettingsPriv()
    {
        okBtn           = 0;
        cancelBtn       = 0;
        tryBtn          = 0;
        defaultBtn      = 0;
        settingsArea    = 0;
        plainPage       = 0;
        btnBox1         = 0;
        btnBox2         = 0;
        saveAsBtn       = 0;
        loadBtn         = 0;
        guideBox        = 0;
        guideColorBt    = 0;
        guideSize       = 0;
        panIconView     = 0;
        channelCB       = 0;
        colorsCB        = 0;
        scaleBG         = 0;
        histogramBox    = 0;
        hGradient       = 0;
        panWidgetFrame  = 0;
    }

    QButtonGroup        *scaleBG;

    QFrame              *panWidgetFrame;

    QToolButton         *linHistoButton;
    QToolButton         *logHistoButton;

    QWidget             *settingsArea;
    QWidget             *plainPage;

    KHBox               *btnBox1;
    KHBox               *btnBox2;
    KHBox               *guideBox;

    KComboBox           *channelCB;
    KComboBox           *colorsCB;

    KPushButton         *okBtn;
    KPushButton         *cancelBtn;
    KPushButton         *tryBtn;
    KPushButton         *defaultBtn;
    KPushButton         *saveAsBtn;
    KPushButton         *loadBtn;

    KColorButton        *guideColorBt;

    ColorGradientWidget *hGradient;

    ImagePanIconWidget  *panIconView;

    HistogramBox        *histogramBox;

    RIntNumInput        *guideSize;
};

//EditorToolSettings::EditorToolSettings(int buttonMask, int toolMask, int histogramType, QWidget *parent)
EditorToolSettings::EditorToolSettings(QWidget *parent)
                  : QScrollArea(parent), d(new EditorToolSettingsPriv)
{
    setFrameStyle( QFrame::NoFrame );
    setWidgetResizable(true);

    d->settingsArea = new QWidget;

    // ---------------------------------------------------------------

    QGridLayout* gridSettings = new QGridLayout(d->settingsArea);

    d->plainPage    = new QWidget(d->settingsArea);
    d->guideBox     = new KHBox(d->settingsArea);
    d->btnBox1      = new KHBox(d->settingsArea);
    d->btnBox2      = new KHBox(d->settingsArea);
    d->histogramBox = new HistogramBox(d->settingsArea);

    // ---------------------------------------------------------------

    d->panWidgetFrame = new QFrame(d->settingsArea);
    d->panWidgetFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* vlay = new QVBoxLayout(d->panWidgetFrame);
    d->panIconView    = new ImagePanIconWidget(360, 240, d->panWidgetFrame);
    d->panIconView->setWhatsThis(i18n("<p>Here you can see the original image panel "
                                      "which can help you to select the clip preview.</p>"
                                      "<p>Click and drag the mouse cursor in the "
                                      "red rectangle to change the clip focus.</p>"));
    vlay->addWidget(d->panIconView, 0, Qt::AlignCenter);
    vlay->setSpacing(0);
    vlay->setMargin(5);

    // ---------------------------------------------------------------

    new QLabel(i18n("Guide:"), d->guideBox);
    QLabel *space4  = new QLabel(d->guideBox);
    d->guideColorBt = new KColorButton(QColor(Qt::red), d->guideBox);
    d->guideColorBt->setWhatsThis(i18n("Set here the color used to draw dashed guide lines."));
    d->guideSize    = new RIntNumInput(d->guideBox);
    d->guideSize->input()->setSuffix(QString("px"));
    d->guideSize->setRange(1, 5, 1);
    d->guideSize->setSliderEnabled(true);
    d->guideSize->setDefaultValue(1);
    d->guideSize->setWhatsThis(i18n("Set here the width in pixels used to draw dashed guide lines."));

    d->guideBox->setStretchFactor(space4, 10);
    d->guideBox->setSpacing(spacingHint());
    d->guideBox->setMargin(0);

    // ---------------------------------------------------------------

    d->defaultBtn = new KPushButton(d->btnBox1);
    d->defaultBtn->setGuiItem(KStandardGuiItem::defaults());
    d->defaultBtn->setIcon(KIcon(SmallIcon("document-revert")));
    d->defaultBtn->setToolTip(i18n("Reset all settings to their default values."));

    QLabel *space2 = new QLabel(d->btnBox1);

    d->okBtn = new KPushButton(d->btnBox1);
    d->okBtn->setGuiItem(KStandardGuiItem::ok());

    d->cancelBtn = new KPushButton(d->btnBox1);
    d->cancelBtn->setGuiItem(KStandardGuiItem::cancel());

    d->btnBox1->setStretchFactor(space2, 10);
    d->btnBox1->setSpacing(spacingHint());
    d->btnBox1->setMargin(0);

    // ---------------------------------------------------------------

    d->loadBtn = new KPushButton(d->btnBox2);
    d->loadBtn->setGuiItem(KStandardGuiItem::open());
    d->loadBtn->setText(i18n("Load..."));
    d->loadBtn->setToolTip(i18n("Load all parameters from settings text file."));

    d->saveAsBtn = new KPushButton(d->btnBox2);
    d->saveAsBtn->setGuiItem(KStandardGuiItem::saveAs());
    d->saveAsBtn->setToolTip(i18n("Save all parameters to settings text file."));


    QLabel *space3 = new QLabel(d->btnBox2);

    d->tryBtn = new KPushButton(d->btnBox2);
    d->tryBtn->setGuiItem(KStandardGuiItem::apply());
    d->tryBtn->setText(i18n("Try"));
    d->tryBtn->setToolTip(i18n("Try all settings."));

    d->btnBox2->setStretchFactor(space3, 10);
    d->btnBox2->setSpacing(spacingHint());
    d->btnBox2->setMargin(0);

    // ---------------------------------------------------------------

    gridSettings->addWidget(d->histogramBox,   0, 0, 2, 2);
    gridSettings->addWidget(d->panWidgetFrame, 2, 0, 1, 2);
    gridSettings->addWidget(d->plainPage,      3, 0, 1, 2);
    gridSettings->addWidget(d->guideBox,       4, 0, 1, 2);
    gridSettings->addWidget(d->btnBox2,        5, 0, 1, 2);
    gridSettings->addWidget(d->btnBox1,        6, 0, 1, 2);
    gridSettings->setSpacing(spacingHint());
    gridSettings->setMargin(spacingHint());

    // ---------------------------------------------------------------

    setWidget(d->settingsArea);
    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    d->settingsArea->setAutoFillBackground(false);
    d->plainPage->setAutoFillBackground(false);

    // ---------------------------------------------------------------

    connect(d->okBtn, SIGNAL(clicked()),
            this, SIGNAL(signalOkClicked()));

    connect(d->cancelBtn, SIGNAL(clicked()),
            this, SIGNAL(signalCancelClicked()));

    connect(d->tryBtn, SIGNAL(clicked()),
            this, SIGNAL(signalTryClicked()));

    connect(d->defaultBtn, SIGNAL(clicked()),
            this, SIGNAL(signalDefaultClicked()));

    connect(d->saveAsBtn, SIGNAL(clicked()),
            this, SIGNAL(signalSaveAsClicked()));

    connect(d->loadBtn, SIGNAL(clicked()),
            this, SIGNAL(signalLoadClicked()));

    connect(d->guideColorBt, SIGNAL(changed(const QColor&)),
            this, SIGNAL(signalColorGuideChanged()));

    connect(d->guideSize, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalColorGuideChanged()));

    connect(d->histogramBox, SIGNAL(signalChannelChanged()),
            this, SIGNAL(signalChannelChanged()));

    connect(d->histogramBox, SIGNAL(signalColorsChanged()),
            this, SIGNAL(signalColorsChanged()));

    connect(d->histogramBox, SIGNAL(signalScaleChanged()),
            this, SIGNAL(signalScaleChanged()));

    // --------------------------------------------------------

    setButtons(Default|Ok|Cancel);
    setTools(NoTool);
}

EditorToolSettings::~EditorToolSettings()
{
    delete d;
}

QSize EditorToolSettings::minimumSizeHint() const
{
    // Editor Tools usually require a larger horizontal space than other widgets in right side bar
    // Set scroll area to a horizontal minimum size sufficient for the settings.
    // Do not touch vertical size hint.
    // Limit to 40% of the desktop width.
    QSize hint = QScrollArea::minimumSizeHint();
    QRect desktopRect = KGlobalSettings::desktopGeometry(d->settingsArea);
    hint.setWidth(qMin(d->settingsArea->minimumSizeHint().width(), desktopRect.width() * 2 / 5));
    return hint;
}

int EditorToolSettings::marginHint()
{
    return KDialog::marginHint();
}

int EditorToolSettings::spacingHint()
{
    return KDialog::spacingHint();
}

QWidget *EditorToolSettings::plainPage() const
{
    return d->plainPage;
}

ImagePanIconWidget* EditorToolSettings::panIconView() const
{
    return d->panIconView;
}

HistogramBox* EditorToolSettings::histogramBox() const
{
    return d->histogramBox;
}

KPushButton* EditorToolSettings::button(int buttonCode) const
{
    if (buttonCode & Default)
        return d->defaultBtn;

    if (buttonCode & Try)
        return d->tryBtn;

    if (buttonCode & Ok)
        return d->okBtn;

    if (buttonCode & Cancel)
        return d->cancelBtn;

    if (buttonCode & Load)
        return d->loadBtn;

    if (buttonCode & SaveAs)
        return d->saveAsBtn;

    return 0;
}

void EditorToolSettings::enableButton(int buttonCode, bool state)
{
    KPushButton *btn = button(buttonCode);
    if (btn) btn->setEnabled(state);
}

QColor EditorToolSettings::guideColor() const
{
    return d->guideColorBt->color();
}

void EditorToolSettings::setGuideColor(const QColor& color)
{
    d->guideColorBt->setColor(color);
}

int EditorToolSettings::guideSize() const
{
    return d->guideSize->value();
}

void EditorToolSettings::setGuideSize(int size)
{
    d->guideSize->setValue(size);
}

void EditorToolSettings::setButtons(Buttons buttonMask)
{
    d->okBtn->setVisible(buttonMask & Ok);
    d->cancelBtn->setVisible(buttonMask & Cancel);
    d->defaultBtn->setVisible(buttonMask & Default);
    d->btnBox1->setVisible((buttonMask & Ok) || (buttonMask & Cancel) || (buttonMask & Default));

    d->loadBtn->setVisible(buttonMask & Load);
    d->saveAsBtn->setVisible(buttonMask & SaveAs);
    d->tryBtn->setVisible(buttonMask & Try);
    d->btnBox2->setVisible((buttonMask & Load) || (buttonMask & SaveAs) || (buttonMask & Try));
}

void EditorToolSettings::setTools(Tools toolMask)
{
    d->panWidgetFrame->setVisible(toolMask & PanIcon);
    d->histogramBox->setVisible(toolMask & Histogram);
    d->guideBox->setVisible(toolMask & ColorGuide);
}

void EditorToolSettings::setHistogramType(HistogramBoxType type)
{
    d->histogramBox->setHistogramType(type);
}

} // namespace Digikam
