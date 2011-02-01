/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-28
 * Description : color label widget
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "colorlabelwidget.moc"

// Qt includes

#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <QLayout>
#include <QButtonGroup>
#include <QWidgetAction>
#include <QFontMetrics>
#include <QFont>
#include <QToolButton>

// KDE includes

#include <kglobalsettings.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmenu.h>

namespace Digikam
{

class ColorLabelWidget::ColorLabelWidgetPriv
{

public:

    ColorLabelWidgetPriv()
    {
        colorBtns  = 0;
        btnNone    = 0;
        btnRed     = 0;
        btnOrange  = 0;
        btnYellow  = 0;
        btnGreen   = 0;
        btnBlue    = 0;
        btnMagenta = 0;
        btnGray    = 0;
        btnBlack   = 0;
        btnWhite   = 0;
    }

    QButtonGroup* colorBtns;

    QToolButton*  btnNone;
    QToolButton*  btnRed;
    QToolButton*  btnOrange;
    QToolButton*  btnYellow;
    QToolButton*  btnGreen;
    QToolButton*  btnBlue;
    QToolButton*  btnMagenta;
    QToolButton*  btnGray;
    QToolButton*  btnBlack;
    QToolButton*  btnWhite;
};

ColorLabelWidget::ColorLabelWidget(QWidget* parent)
    : KHBox(parent), d(new ColorLabelWidgetPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);

    d->btnNone = new QToolButton(this);
    d->btnNone->setCheckable(true);
    d->btnNone->setFocusPolicy(Qt::NoFocus);
    d->btnNone->setIcon(buildIcon(NoneLabel));

    d->btnRed = new QToolButton(this);
    d->btnRed->setCheckable(true);
    d->btnRed->setFocusPolicy(Qt::NoFocus);
    d->btnRed->setIcon(buildIcon(RedLabel));

    d->btnOrange = new QToolButton(this);
    d->btnOrange->setCheckable(true);
    d->btnOrange->setFocusPolicy(Qt::NoFocus);
    d->btnOrange->setIcon(buildIcon(OrangeLabel));

    d->btnYellow = new QToolButton(this);
    d->btnYellow->setCheckable(true);
    d->btnYellow->setFocusPolicy(Qt::NoFocus);
    d->btnYellow->setIcon(buildIcon(YellowLabel));

    d->btnGreen = new QToolButton(this);
    d->btnGreen->setCheckable(true);
    d->btnGreen->setFocusPolicy(Qt::NoFocus);
    d->btnGreen->setIcon(buildIcon(GreenLabel));

    d->btnBlue = new QToolButton(this);
    d->btnBlue->setCheckable(true);
    d->btnBlue->setFocusPolicy(Qt::NoFocus);
    d->btnBlue->setIcon(buildIcon(BlueLabel));

    d->btnMagenta = new QToolButton(this);
    d->btnMagenta->setCheckable(true);
    d->btnMagenta->setFocusPolicy(Qt::NoFocus);
    d->btnMagenta->setIcon(buildIcon(MagentaLabel));

    d->btnGray = new QToolButton(this);
    d->btnGray->setCheckable(true);
    d->btnGray->setFocusPolicy(Qt::NoFocus);
    d->btnGray->setIcon(buildIcon(GrayLabel));

    d->btnBlack = new QToolButton(this);
    d->btnBlack->setCheckable(true);
    d->btnBlack->setFocusPolicy(Qt::NoFocus);
    d->btnBlack->setIcon(buildIcon(BlackLabel));

    d->btnWhite = new QToolButton(this);
    d->btnWhite->setCheckable(true);
    d->btnWhite->setFocusPolicy(Qt::NoFocus);
    d->btnWhite->setIcon(buildIcon(WhiteLabel));

    d->colorBtns = new QButtonGroup(this);
    d->colorBtns->setExclusive(true);
    d->colorBtns->addButton(d->btnNone,    NoneLabel);
    d->colorBtns->addButton(d->btnRed,     RedLabel);
    d->colorBtns->addButton(d->btnOrange,  OrangeLabel);
    d->colorBtns->addButton(d->btnYellow,  YellowLabel);
    d->colorBtns->addButton(d->btnGreen,   GreenLabel);
    d->colorBtns->addButton(d->btnBlue,    BlueLabel);
    d->colorBtns->addButton(d->btnMagenta, MagentaLabel);
    d->colorBtns->addButton(d->btnGray,    GrayLabel);
    d->colorBtns->addButton(d->btnBlack,   BlackLabel);
    d->colorBtns->addButton(d->btnWhite,   WhiteLabel);

    setMargin(0);
    setSpacing(0);
    setColorLabel(NoneLabel);

    // -------------------------------------------------------------

    connect(d->colorBtns, SIGNAL(buttonReleased(int)),
            this, SIGNAL(signalColorLabelChanged(int)));
}

ColorLabelWidget::~ColorLabelWidget()
{
    delete d;
}

void ColorLabelWidget::setColorLabel(ColorLabel label)
{
    QAbstractButton* btn = d->colorBtns->button(label);
    if (btn) btn->setChecked(true);
}

ColorLabel ColorLabelWidget::colorLabel()
{
    QAbstractButton* btn = d->colorBtns->checkedButton();
    if (btn) return (ColorLabel)(d->colorBtns->id(btn));

    return NoneLabel;
}

QIcon ColorLabelWidget::buildIcon(ColorLabel label) const
{
    QPixmap pix(12, 12);
    QPainter p(&pix);
    p.setBrush(palette().color(QPalette::Active, QPalette::ButtonText));
    p.fillRect(0, 0, pix.width(), pix.height(), labelColor(label));
    if (label == NoneLabel)
    {
        p.drawLine(0, 0, pix.width(), pix.height());
        p.drawLine(0, pix.height(), pix.width(), 0);
    }

/*
    p.drawRect(0, 0, pix.width(), pix.height());
*/
    return QIcon(pix);
}

QColor ColorLabelWidget::labelColor(ColorLabel label)
{
    QColor color;

    switch(label)
    {
        case RedLabel:
            color = qRgb(0xDF, 0x6E, 0x5F);
            break;
        case OrangeLabel:
            color = qRgb(0xEE, 0xAF, 0x6B);
            break;
        case YellowLabel:
            color = qRgb(0xE4, 0xD3, 0x78);
            break;
        case GreenLabel:
            color = qRgb(0xAF, 0xD8, 0x78);
            break;
        case BlueLabel:
            color = qRgb(0x77, 0xBA, 0xE8);
            break;
        case MagentaLabel:
            color = qRgb(0xCB, 0x98, 0xE1);
            break;
        case GrayLabel:
            color = qRgb(0xB7, 0xB7, 0xB7);
            break;
        case BlackLabel:
            color = qRgb(0x28, 0x28, 0x28);
            break;
        case WhiteLabel:
            color = qRgb(0xF7, 0xFE, 0xFA);
            break;
        default:   // NoneLabel
            break;
    }

    return color;
}

QString ColorLabelWidget::labelColorName(ColorLabel label)
{
    QString name;

    switch(label)
    {
        case RedLabel:
            name = i18n("Red");
            break;
        case OrangeLabel:
            name = i18n("Orange");
            break;
        case YellowLabel:
            name = i18n("Yellow");
            break;
        case GreenLabel:
            name = i18n("Green");
            break;
        case BlueLabel:
            name = i18n("Blue");
            break;
        case MagentaLabel:
            name = i18n("Magenta");
            break;
        case GrayLabel:
            name = i18n("Gray");
            break;
        case BlackLabel:
            name = i18n("Black");
            break;
        case WhiteLabel:
            name = i18n("White");
            break;
        default:   // NoneLabel
            name = i18n("None");
            break;
    }

    return name;
}

// -----------------------------------------------------------------------------

class ColorLabelSelector::ColorLabelSelectorPriv
{

public:

    ColorLabelSelectorPriv()
    {
        clw = 0;
    }

    ColorLabelWidget* clw;
};

ColorLabelSelector::ColorLabelSelector(QWidget* parent)
    : QPushButton(parent), d(new ColorLabelSelectorPriv)
{
    KMenu* popup = new KMenu(this);
    setMenu(popup);
    setToolTip(i18n("Color Label"));

    QWidgetAction* action = new QWidgetAction(this);
    d->clw                = new ColorLabelWidget(this);
    action->setDefaultWidget(d->clw);
    popup->addAction(action);
    slotColorLabelChanged(NoneLabel);

    connect(d->clw, SIGNAL(signalColorLabelChanged(int)),
            this, SLOT(slotColorLabelChanged(int)));
}

ColorLabelSelector::~ColorLabelSelector()
{
    delete d;
}

void ColorLabelSelector::setColorLabel(ColorLabel label)
{
    d->clw->setColorLabel(label);
    slotColorLabelChanged(label);
}

ColorLabel ColorLabelSelector::colorLabel()
{
    return d->clw->colorLabel();
}

void ColorLabelSelector::slotColorLabelChanged(int colorId)
{
    QFontMetrics fontMt = fontMetrics();
    QString none(i18n("None"));
    QRect fntRect(0, 0, fontMt.width(none)+2, fontMt.height()+2);

    QPixmap pix(fntRect.size());
    QPainter p(&pix);
    p.setFont(font());
    p.setBrush(palette().color(QPalette::Active, QPalette::ButtonText));

    p.fillRect(fntRect, d->clw->labelColor((ColorLabel)colorId));

    if (colorId == NoneLabel)
        p.drawText(fntRect, none);

    QIcon icon(pix);
    setIconSize(pix.size());
    setIcon(icon);
    menu()->close();

    emit signalColorLabelChanged(colorId);
}

}  // namespace Digikam
