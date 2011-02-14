/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-14
 * Description : pick label widget
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

#include "picklabelwidget.moc"

// Qt includes

#include <QFont>
#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <QLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QWidgetAction>
#include <QFontMetrics>
#include <QFont>
#include <QToolButton>

// KDE includes

#include <kglobalsettings.h>
#include <ksqueezedtextlabel.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmenu.h>
#include <khbox.h>
#include <kapplication.h>
#include <kxmlguiwindow.h>
#include <kactioncollection.h>

namespace Digikam
{

class PickLabelWidget::PickLabelWidgetPriv
{

public:

    PickLabelWidgetPriv()
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
        desc       = 0;
        descBox    = 0;
        shortcut   = 0;
    }

    QButtonGroup*       colorBtns;

    QLabel*             desc;

    QToolButton*        btnNone;
    QToolButton*        btnRed;
    QToolButton*        btnOrange;
    QToolButton*        btnYellow;
    QToolButton*        btnGreen;
    QToolButton*        btnBlue;
    QToolButton*        btnMagenta;
    QToolButton*        btnGray;
    QToolButton*        btnBlack;
    QToolButton*        btnWhite;

    KHBox*              descBox;

    KSqueezedTextLabel* shortcut;
};

PickLabelWidget::PickLabelWidget(QWidget* parent)
    : KVBox(parent), d(new PickLabelWidgetPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);

    KHBox* hbox = new KHBox(this);
    hbox->setMargin(0);
    hbox->setSpacing(0);

    d->btnNone = new QToolButton(hbox);
    d->btnNone->setCheckable(true);
    d->btnNone->setFocusPolicy(Qt::NoFocus);
    d->btnNone->setIcon(buildIcon(NoPickLabel));
    d->btnNone->installEventFilter(this);

    d->btnRed = new QToolButton(hbox);
    d->btnRed->setCheckable(true);
    d->btnRed->setFocusPolicy(Qt::NoFocus);
    d->btnRed->setIcon(buildIcon(RejectedLabel));
    d->btnRed->installEventFilter(this);

    d->btnOrange = new QToolButton(hbox);
    d->btnOrange->setCheckable(true);
    d->btnOrange->setFocusPolicy(Qt::NoFocus);
    d->btnOrange->setIcon(buildIcon(PendingLabel));
    d->btnOrange->installEventFilter(this);

    d->btnYellow = new QToolButton(hbox);
    d->btnYellow->setCheckable(true);
    d->btnYellow->setFocusPolicy(Qt::NoFocus);
    d->btnYellow->setIcon(buildIcon(AcceptedLabel));
    d->btnYellow->installEventFilter(this);

    d->colorBtns = new QButtonGroup(hbox);
    d->colorBtns->addButton(d->btnNone,    NoPickLabel);
    d->colorBtns->addButton(d->btnRed,     RejectedLabel);
    d->colorBtns->addButton(d->btnOrange,  PendingLabel);
    d->colorBtns->addButton(d->btnYellow,  AcceptedLabel);

    d->descBox  = new KHBox(this);
    d->descBox->setMargin(0);
    d->descBox->setSpacing(0);
    d->desc     = new QLabel(d->descBox);
    d->shortcut = new KSqueezedTextLabel(d->descBox);
    QFont fnt = d->shortcut->font();
    fnt.setItalic(true);
    d->shortcut->setFont(fnt);
    d->shortcut->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->shortcut->setWordWrap(false);

    setMargin(0);
    setSpacing(0);
    setPickLabels(QList<PickLabel>() << NoPickLabel);
    setDescriptionBoxVisible(true);
    setButtonsExclusive(true);

    // -------------------------------------------------------------

    connect(d->colorBtns, SIGNAL(buttonReleased(int)),
            this, SIGNAL(signalPickLabelChanged(int)));
}

PickLabelWidget::~PickLabelWidget()
{
    delete d;
}

void PickLabelWidget::setDescriptionBoxVisible(bool b)
{
    d->descBox->setVisible(b);
}

void PickLabelWidget::setButtonsExclusive(bool b)
{
    d->colorBtns->setExclusive(b);
}

void PickLabelWidget::updateDescription(PickLabel label)
{
    d->desc->setText(labelPickName(label));

    KXmlGuiWindow* app = dynamic_cast<KXmlGuiWindow*>(kapp->activeWindow());
    if (app)
    {
        QAction* ac = app->actionCollection()->action(QString("colorlabel-%1").arg(label));
        if (ac)
            d->shortcut->setText(ac->shortcut().toString());
    }
}

bool PickLabelWidget::eventFilter(QObject* obj, QEvent* ev)
{
    if ( obj == d->btnNone)
    {
        if ( ev->type() == QEvent::Enter)
        {
            updateDescription(NoPickLabel);
            return false;
        }
    }
    if ( obj == d->btnRed)
    {
        if ( ev->type() == QEvent::Enter)
        {
            updateDescription(RejectedLabel);
            return false;
        }
    }
    if ( obj == d->btnOrange)
    {
        if ( ev->type() == QEvent::Enter)
        {
            updateDescription(PendingLabel);
            return false;
        }
    }
    if ( obj == d->btnYellow)
    {
        if ( ev->type() == QEvent::Enter)
        {
            updateDescription(AcceptedLabel);
            return false;
        }
    }

    // pass the event on to the parent class
    return QWidget::eventFilter(obj, ev);
}

void PickLabelWidget::setPickLabels(const QList<PickLabel>& list)
{
    foreach(QAbstractButton* btn, d->colorBtns->buttons())
    {
        PickLabel id = (PickLabel)(d->colorBtns->id(btn));
        btn->setChecked(list.contains(id));
        updateDescription(id);
    }
}

QList<PickLabel> PickLabelWidget::colorLabels() const
{
    QList<PickLabel> list;
    foreach(QAbstractButton* btn, d->colorBtns->buttons())
    {
        if (btn && btn->isChecked())
            list.append((PickLabel)(d->colorBtns->id(btn)));
    }

    return list;
}

QIcon PickLabelWidget::buildIcon(PickLabel label) const
{
    QPixmap pix(12, 12);
    QPainter p(&pix);
    p.setPen(palette().color(QPalette::Active, QPalette::ButtonText));
/* TODO
    if (label != NoPickLabel)
    {
        p.fillRect(0, 0, pix.width()-1, pix.height()-1, labelColor(label));
    }
    else
    {
        p.fillRect(0, 0, pix.width()-1, pix.height()-1, palette().color(QPalette::Active, QPalette::Button));
        p.drawLine(0, 0, pix.width()-1, pix.height()-1);
        p.drawLine(0, pix.height()-1, pix.width()-1, 0);
    }
*/
    p.drawRect(0, 0, pix.width()-1, pix.height()-1);

    return QIcon(pix);
}

QColor PickLabelWidget::labelColor(PickLabel label)
{
    QColor color;

    switch(label)
    {
        case RejectedLabel:
            color = qRgb(0xDF, 0x6E, 0x5F);
            break;
        case PendingLabel:
            color = qRgb(0xEE, 0xAF, 0x6B);
            break;
        case AcceptedLabel:
            color = qRgb(0xE4, 0xD3, 0x78);
            break;
        default:   // NoPickLabel
            break;
    }

    return color;
}

QString PickLabelWidget::labelPickName(PickLabel label)
{
    QString name;

    switch(label)
    {
        case RejectedLabel:
            name = i18n("Rejected");
            break;
        case PendingLabel:
            name = i18n("Pending");
            break;
        case AcceptedLabel:
            name = i18n("Accepted");
            break;
        default:   // NoPickLabel
            name = i18n("None");
            break;
    }

    return name;
}

// -----------------------------------------------------------------------------

class PickLabelSelector::PickLabelSelectorPriv
{

public:

    PickLabelSelectorPriv()
    {
        clw = 0;
    }

    PickLabelWidget* clw;
};

PickLabelSelector::PickLabelSelector(QWidget* parent)
    : QPushButton(parent), d(new PickLabelSelectorPriv)
{
    KMenu* popup = new KMenu(this);
    setMenu(popup);
    setToolTip(i18n("Pick Label"));

    QWidgetAction* action = new QWidgetAction(this);
    d->clw                = new PickLabelWidget(this);
    action->setDefaultWidget(d->clw);
    popup->addAction(action);
    slotPickLabelChanged(NoPickLabel);

    connect(d->clw, SIGNAL(signalPickLabelChanged(int)),
            this, SLOT(slotPickLabelChanged(int)));
}

PickLabelSelector::~PickLabelSelector()
{
    delete d;
}

void PickLabelSelector::setPickLabel(PickLabel label)
{
    d->clw->setPickLabels(QList<PickLabel>() << label);
    slotPickLabelChanged(label);
}

PickLabel PickLabelSelector::colorLabel()
{
    QList<PickLabel> list = d->clw->colorLabels();
    if (!list.isEmpty())
        return list.first();

    return NoPickLabel;
}

void PickLabelSelector::slotPickLabelChanged(int colorId)
{
    QString none(i18n("None"));

    if (colorId != NoPickLabel)
    {
        setText(QString());
        QFontMetrics fontMt = fontMetrics();
        QRect fntRect(0, 0, fontMt.width(none), fontMt.height());

        QPixmap pix(fntRect.size());
        QPainter p(&pix);
        p.setPen(palette().color(QPalette::Active, QPalette::ButtonText));

        p.fillRect(fntRect, d->clw->labelColor((PickLabel)colorId));
        p.drawRect(0, 0, fntRect.width()-1, fntRect.height()-1);

        QIcon icon(pix);
        setIconSize(pix.size());
        setIcon(icon);
    }
    else
    {
        setIconSize(QSize());
        setIcon(QIcon());
        setText(none);
    }

    menu()->close();

    emit signalPickLabelChanged(colorId);
}

// -----------------------------------------------------------------------------

PickLabelMenuAction::PickLabelMenuAction(QMenu* parent)
    : KActionMenu(parent)
{
    setText(i18n("Assign Pick Label"));
    QWidgetAction* wa     = new QWidgetAction(this);
    PickLabelWidget* clw = new PickLabelWidget(parent);
    wa->setDefaultWidget(clw);
    addAction(wa);

    connect(clw, SIGNAL(signalPickLabelChanged(int)),
            this, SIGNAL(signalPickLabelChanged(int)));

    connect(clw, SIGNAL(signalPickLabelChanged(int)),
            parent, SLOT(close()));
}

PickLabelMenuAction::~PickLabelMenuAction()
{
}

}  // namespace Digikam
