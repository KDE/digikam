/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-21
 * Description : Editor tool settings template box
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2011 by Andi Clemens <andi dot clemens at gmail dot com>
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

// Qt includes

#include <QButtonGroup>
#include <QLabel>
#include <QPixmap>
#include <QLayout>
#include <QMap>
#include <QPair>
#include <QString>
#include <QToolButton>
#include <QVariant>
#include <QScrollBar>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dnuminput.h"
#include "colorgradientwidget.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "digikam_globals.h"
#include "dcolorselector.h"

namespace Digikam
{

class EditorToolSettings::Private
{

public:

    Private() :
        scaleBG(0),
        linHistoButton(0),
        logHistoButton(0),
        settingsArea(0),
        plainPage(0),
        toolName(0),
        toolIcon(0),
        guideBox(0),
        okBtn(0),
        cancelBtn(0),
        tryBtn(0),
        defaultBtn(0),
        saveAsBtn(0),
        loadBtn(0),
        guideColorBt(0),
        hGradient(0),
        histogramBox(0),
        guideSize(0)
    {
    }

    QButtonGroup*        scaleBG;

    QToolButton*         linHistoButton;
    QToolButton*         logHistoButton;

    QWidget*             settingsArea;
    QWidget*             plainPage;

    QLabel*              toolName;
    QLabel*              toolIcon;

    DHBox*               guideBox;

    QPushButton*         okBtn;
    QPushButton*         cancelBtn;
    QPushButton*         tryBtn;
    QPushButton*         defaultBtn;
    QPushButton*         saveAsBtn;
    QPushButton*         loadBtn;

    DColorSelector*      guideColorBt;

    ColorGradientWidget* hGradient;

    HistogramBox*        histogramBox;

    DIntNumInput*        guideSize;
};

EditorToolSettings::EditorToolSettings(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);

    d->settingsArea = new QWidget;

    // ---------------------------------------------------------------

    QGridLayout* const gridSettings = new QGridLayout(d->settingsArea);
    d->plainPage                    = new QWidget;
    d->guideBox                     = new DHBox;
    d->histogramBox                 = new HistogramBox;

    // ---------------------------------------------------------------

    QFrame* const toolDescriptor = new QFrame;
    d->toolName                  = new QLabel();
    d->toolIcon                  = new QLabel();
    QFont font                   = d->toolName->font();
    font.setBold(true);
    d->toolName->setFont(font);

    QString frameStyle = QString::fromLatin1("QFrame {"
                                 "color: %1;"
                                 "border: 1px solid %2;"
                                 "border-radius: 5px;"
                                 "background-color: %3;"
                                 "}")
                         .arg(QApplication::palette().color(QPalette::HighlightedText).name())
                         .arg(QApplication::palette().color(QPalette::HighlightedText).name())
                         .arg(QApplication::palette().color(QPalette::Highlight).name());

    QString noFrameStyle(QLatin1String("QFrame {"
                         "border: none;"
                         "}"));

    toolDescriptor->setStyleSheet(frameStyle);
    d->toolName->setStyleSheet(noFrameStyle);
    d->toolIcon->setStyleSheet(noFrameStyle);

    QGridLayout* const descrLayout = new QGridLayout();
    descrLayout->addWidget(d->toolIcon, 0, 0, 1, 1);
    descrLayout->addWidget(d->toolName, 0, 1, 1, 1);
    descrLayout->setColumnStretch(1, 10);
    toolDescriptor->setLayout(descrLayout);

    // ---------------------------------------------------------------

    new QLabel(i18n("Guide:"), d->guideBox);
    QLabel* const space4 = new QLabel(d->guideBox);
    d->guideColorBt      = new DColorSelector(d->guideBox);
    d->guideColorBt->setColor(QColor(Qt::red));
    d->guideColorBt->setWhatsThis(i18n("Set here the color used to draw dashed guide lines."));
    d->guideSize         = new DIntNumInput(d->guideBox);
    d->guideSize->setSuffix(QLatin1String("px"));
    d->guideSize->setRange(1, 5, 1);
    d->guideSize->setDefaultValue(1);
    d->guideSize->setWhatsThis(i18n("Set here the width in pixels used to draw dashed guide lines."));

    d->guideBox->setStretchFactor(space4, 10);
    d->guideBox->setContentsMargins(QMargins());
    d->guideBox->setSpacing(spacingHint());

    // ---------------------------------------------------------------

    d->defaultBtn = new QPushButton(i18n("Defaults"));
    d->defaultBtn->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->defaultBtn->setToolTip(i18n("Reset all settings to their default values."));

    d->okBtn = new QPushButton(i18n("OK"));
    d->okBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-ok-apply")));
    d->okBtn->setDefault(true);

    d->cancelBtn = new QPushButton(i18n("Cancel"));
    d->cancelBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-cancel")));

    QHBoxLayout* const hbox1 = new QHBoxLayout;
    hbox1->addWidget(d->defaultBtn);
    hbox1->addStretch(1);
    hbox1->addWidget(d->okBtn);
    hbox1->addWidget(d->cancelBtn);

    // ---------------------------------------------------------------

    d->loadBtn = new QPushButton(i18n("Load..."));
    d->loadBtn->setIcon(QIcon::fromTheme(QLatin1String("document-open")));
    d->loadBtn->setToolTip(i18n("Load all parameters from settings text file."));

    d->saveAsBtn = new QPushButton(i18n("Save As..."));
    d->saveAsBtn->setIcon(QIcon::fromTheme(QLatin1String("document-save-as")));
    d->saveAsBtn->setToolTip(i18n("Save all parameters to settings text file."));

    d->tryBtn = new QPushButton(i18n("Try"));
    d->tryBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-ok-apply")));
    d->tryBtn->setToolTip(i18n("Try all settings."));

    QHBoxLayout* const hbox2 = new QHBoxLayout;
    hbox2->addWidget(d->loadBtn);
    hbox2->addWidget(d->saveAsBtn);
    hbox2->addStretch(1);
    hbox2->addWidget(d->tryBtn);

    // ---------------------------------------------------------------

    const int spacing = spacingHint();

    gridSettings->addWidget(toolDescriptor,  0, 0, 1, -1);
    gridSettings->addWidget(d->histogramBox, 1, 0, 2, 2);
    gridSettings->addWidget(d->plainPage,    4, 0, 1, 2);
    gridSettings->addWidget(d->guideBox,     5, 0, 1, 2);
    gridSettings->addLayout(hbox2,           6, 0, 1, 2);
    gridSettings->addLayout(hbox1,           7, 0, 1, 2);
    gridSettings->setContentsMargins(spacing, spacing, spacing, spacing);
    gridSettings->setSpacing(spacing);

    // ---------------------------------------------------------------

    setWidget(d->settingsArea);

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

    connect(d->guideColorBt, SIGNAL(signalColorSelected(QColor)),
            this, SIGNAL(signalColorGuideChanged()));

    connect(d->guideSize, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalColorGuideChanged()));

    connect(d->histogramBox, SIGNAL(signalChannelChanged(ChannelType)),
            this, SIGNAL(signalChannelChanged()));

    connect(d->histogramBox, SIGNAL(signalScaleChanged(HistogramScale)),
            this, SIGNAL(signalScaleChanged()));

    // --------------------------------------------------------

    setTabOrder(d->tryBtn,     d->okBtn);
    setTabOrder(d->okBtn,      d->cancelBtn);
    setTabOrder(d->cancelBtn,  d->defaultBtn);
    setTabOrder(d->defaultBtn, d->loadBtn);
    setTabOrder(d->loadBtn,    d->saveAsBtn);

    // ---------------------------------------------------------------

    setButtons(Default | Ok | Cancel);
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
    QSize hint        = QScrollArea::minimumSizeHint();
    QRect desktopRect = QApplication::desktop()->screenGeometry(d->settingsArea);
    int wSB           = verticalScrollBar()->height();
    hint.setWidth(qMin(d->settingsArea->sizeHint().width() + wSB, desktopRect.width() * 2 / 5));
    return hint;
}

int EditorToolSettings::marginHint()
{
    return QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin);
}

int EditorToolSettings::spacingHint()
{
    return QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
}

QWidget* EditorToolSettings::plainPage() const
{
    return d->plainPage;
}

HistogramBox* EditorToolSettings::histogramBox() const
{
    return d->histogramBox;
}

QPushButton* EditorToolSettings::button(int buttonCode) const
{
    if (buttonCode & Default)
    {
        return d->defaultBtn;
    }

    if (buttonCode & Try)
    {
        return d->tryBtn;
    }

    if (buttonCode & Ok)
    {
        return d->okBtn;
    }

    if (buttonCode & Cancel)
    {
        return d->cancelBtn;
    }

    if (buttonCode & Load)
    {
        return d->loadBtn;
    }

    if (buttonCode & SaveAs)
    {
        return d->saveAsBtn;
    }

    return 0;
}

void EditorToolSettings::enableButton(int buttonCode, bool state)
{
    QPushButton* const btn = button(buttonCode);

    if (btn)
    {
        btn->setEnabled(state);
    }
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

    d->loadBtn->setVisible(buttonMask & Load);
    d->saveAsBtn->setVisible(buttonMask & SaveAs);
    d->tryBtn->setVisible(buttonMask & Try);
}

void EditorToolSettings::setTools(Tools toolMask)
{
    d->histogramBox->setVisible(toolMask & Histogram);
    d->guideBox->setVisible(toolMask & ColorGuide);
}

void EditorToolSettings::setHistogramType(HistogramBoxType type)
{
    d->histogramBox->setHistogramType(type);
}

void EditorToolSettings::setToolIcon(const QIcon& icon)
{
    d->toolIcon->setPixmap(icon.pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));
}

void EditorToolSettings::setToolName(const QString& name)
{
    d->toolName->setText(name);
}

} // namespace Digikam
