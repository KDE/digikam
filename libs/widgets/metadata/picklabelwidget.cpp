/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-14
 * Description : pick label widget
 *
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

#include "picklabelwidget.h"

// Qt includes

#include <QPainter>
#include <QIcon>
#include <QLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QWidgetAction>
#include <QFontMetrics>
#include <QFont>
#include <QToolButton>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>

// Local includes

#include "dlayoutbox.h"
#include "dxmlguiwindow.h"
#include "dexpanderbox.h"

namespace Digikam
{

class PickLabelWidget::Private
{

public:

    Private()
    {
        pickBtns = 0;
        btnNone  = 0;
        btnRej   = 0;
        btnPndg  = 0;
        btnAccpt = 0;
        desc     = 0;
        descBox  = 0;
        shortcut = 0;
    }

    QButtonGroup*     pickBtns;

    QLabel*           desc;

    QToolButton*      btnNone;
    QToolButton*      btnRej;
    QToolButton*      btnPndg;
    QToolButton*      btnAccpt;

    DHBox*            descBox;

    DAdjustableLabel* shortcut;
};

PickLabelWidget::PickLabelWidget(QWidget* const parent)
    : DVBox(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);

    DHBox* const hbox = new DHBox(this);
    hbox->setContentsMargins(QMargins());
    hbox->setSpacing(0);

    d->btnNone = new QToolButton(hbox);
    d->btnNone->setCheckable(true);
    d->btnNone->setFocusPolicy(Qt::NoFocus);
    d->btnNone->setIcon(buildIcon(NoPickLabel));
    d->btnNone->installEventFilter(this);

    d->btnRej = new QToolButton(hbox);
    d->btnRej->setCheckable(true);
    d->btnRej->setFocusPolicy(Qt::NoFocus);
    d->btnRej->setIcon(buildIcon(RejectedLabel));
    d->btnRej->installEventFilter(this);

    d->btnPndg = new QToolButton(hbox);
    d->btnPndg->setCheckable(true);
    d->btnPndg->setFocusPolicy(Qt::NoFocus);
    d->btnPndg->setIcon(buildIcon(PendingLabel));
    d->btnPndg->installEventFilter(this);

    d->btnAccpt = new QToolButton(hbox);
    d->btnAccpt->setCheckable(true);
    d->btnAccpt->setFocusPolicy(Qt::NoFocus);
    d->btnAccpt->setIcon(buildIcon(AcceptedLabel));
    d->btnAccpt->installEventFilter(this);

    d->pickBtns = new QButtonGroup(hbox);
    d->pickBtns->addButton(d->btnNone,  NoPickLabel);
    d->pickBtns->addButton(d->btnRej,   RejectedLabel);
    d->pickBtns->addButton(d->btnPndg,  PendingLabel);
    d->pickBtns->addButton(d->btnAccpt, AcceptedLabel);

    d->descBox  = new DHBox(this);
    d->descBox->setContentsMargins(QMargins());
    d->descBox->setSpacing(0);
    d->desc     = new QLabel(d->descBox);
    d->shortcut = new DAdjustableLabel(d->descBox);
    QFont fnt = d->shortcut->font();
    fnt.setItalic(true);
    d->shortcut->setFont(fnt);
    d->shortcut->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->shortcut->setWordWrap(false);

    setSpacing(0);
    setContentsMargins(QMargins());
    setPickLabels(QList<PickLabel>() << NoPickLabel);
    setDescriptionBoxVisible(true);
    setButtonsExclusive(true);

    // -------------------------------------------------------------

    connect(d->pickBtns, SIGNAL(buttonReleased(int)),
            this, SIGNAL(signalPickLabelChanged(int)));
}

PickLabelWidget::~PickLabelWidget()
{
    delete d;
}

void PickLabelWidget::setDescriptionBoxVisible(bool b)
{
    d->descBox->setVisible(b);

    if (!b)
    {
        foreach(QAbstractButton* const btn, d->pickBtns->buttons())
        {
            PickLabel id = (PickLabel)(d->pickBtns->id(btn));
            btn->setToolTip(labelPickName(id));
        }
    }
}

void PickLabelWidget::setButtonsExclusive(bool b)
{
    d->pickBtns->setExclusive(b);
}

void PickLabelWidget::updateDescription(PickLabel label)
{
    d->desc->setText(labelPickName(label));

    DXmlGuiWindow* const app = dynamic_cast<DXmlGuiWindow*>(qApp->activeWindow());

    if (app)
    {
        QAction* const ac = app->actionCollection()->action(QString::fromLatin1("pickshortcut-%1").arg(label));

        if (ac)
        {
            d->shortcut->setAdjustedText(ac->shortcut().toString());
        }
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

    if ( obj == d->btnRej)
    {
        if ( ev->type() == QEvent::Enter)
        {
            updateDescription(RejectedLabel);
            return false;
        }
    }

    if ( obj == d->btnPndg)
    {
        if ( ev->type() == QEvent::Enter)
        {
            updateDescription(PendingLabel);
            return false;
        }
    }

    if ( obj == d->btnAccpt)
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
    foreach(QAbstractButton* const btn, d->pickBtns->buttons())
    {
        PickLabel id = (PickLabel)(d->pickBtns->id(btn));
        btn->setChecked(list.contains(id));
        updateDescription(id);
    }
}

QList<PickLabel> PickLabelWidget::colorLabels() const
{
    QList<PickLabel> list;

    foreach(QAbstractButton* const btn, d->pickBtns->buttons())
    {
        if (btn && btn->isChecked())
            list.append((PickLabel)(d->pickBtns->id(btn)));
    }

    return list;
}

QIcon PickLabelWidget::buildIcon(PickLabel label)
{
    switch(label)
    {
        case RejectedLabel:
            return QIcon::fromTheme(QLatin1String("flag-red"));
            break;
        case PendingLabel:
            return QIcon::fromTheme(QLatin1String("flag-yellow"));
            break;
        case AcceptedLabel:
            return QIcon::fromTheme(QLatin1String("flag-green"));
            break;
        default:
            break;
    }

    // default : NoPickLabel
    return QIcon::fromTheme(QLatin1String("flag-black"));
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

class PickLabelSelector::Private
{

public:

    Private()
    {
        plw = 0;
    }

    PickLabelWidget* plw;
};

PickLabelSelector::PickLabelSelector(QWidget* const parent)
    : QPushButton(parent),
      d(new Private)
{
    QMenu* const popup = new QMenu(this);
    setMenu(popup);

    QWidgetAction* const action = new QWidgetAction(this);
    d->plw                      = new PickLabelWidget(this);
    action->setDefaultWidget(d->plw);
    popup->addAction(action);
    slotPickLabelChanged(NoPickLabel);

    connect(d->plw, SIGNAL(signalPickLabelChanged(int)),
            this, SLOT(slotPickLabelChanged(int)));
}

PickLabelSelector::~PickLabelSelector()
{
    delete d;
}

PickLabelWidget* PickLabelSelector::pickLabelWidget() const
{
    return d->plw;
}

void PickLabelSelector::setPickLabel(PickLabel label)
{
    d->plw->setPickLabels(QList<PickLabel>() << label);
    slotPickLabelChanged(label);
}

PickLabel PickLabelSelector::colorLabel()
{
    QList<PickLabel> list = d->plw->colorLabels();

    if (!list.isEmpty())
        return list.first();

    return NoPickLabel;
}

void PickLabelSelector::slotPickLabelChanged(int id)
{
    setText(QString());
    setIcon(d->plw->buildIcon((PickLabel)id));
    setToolTip(i18n("Pick Label: %1", d->plw->labelPickName((PickLabel)id)));
    menu()->close();

    emit signalPickLabelChanged(id);
}

// -----------------------------------------------------------------------------

PickLabelMenuAction::PickLabelMenuAction(QMenu* const parent)
    : QMenu(parent)
{
    setTitle(i18n("Pick"));
    QWidgetAction* const wa    = new QWidgetAction(this);
    PickLabelWidget* const plw = new PickLabelWidget(parent);
    wa->setDefaultWidget(plw);
    addAction(wa);

    connect(plw, SIGNAL(signalPickLabelChanged(int)),
            this, SIGNAL(signalPickLabelChanged(int)));

    connect(plw, SIGNAL(signalPickLabelChanged(int)),
            parent, SLOT(close()));
}

PickLabelMenuAction::~PickLabelMenuAction()
{
}

}  // namespace Digikam
