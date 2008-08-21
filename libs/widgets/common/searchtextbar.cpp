/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-25
 * Description : a bar used to search a string.
 * 
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qcolor.h>
#include <qpalette.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtoolbutton.h>

// KDE includes.

#include <kapplication.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <kconfig.h>

// Local includes.

#include "searchtextbar.h"
#include "searchtextbar.moc"

namespace Digikam
{

class DLineEditPriv
{
public:

    DLineEditPriv()
    {
        drawMsg = true;
    }

    bool    drawMsg;

    QString message;
};

DLineEdit::DLineEdit(const QString &msg, QWidget *parent)
         : KLineEdit(parent)
{
    d = new DLineEditPriv;
    setMessage(msg);
}

DLineEdit::~DLineEdit()
{
    delete d;
}

QString DLineEdit::message() const
{
    return d->message;
}

void DLineEdit::setMessage(const QString &msg)
{
    d->message = msg;
    repaint();
}

void DLineEdit::setText(const QString &txt)
{
    d->drawMsg = txt.isEmpty();
    repaint();
    KLineEdit::setText(txt);
}

void DLineEdit::drawContents(QPainter *p)
{
    KLineEdit::drawContents(p);

    if (d->drawMsg && !hasFocus())
    {
        QPen tmp = p->pen();
        p->setPen(palette().color(QPalette::Disabled, QColorGroup::Text));
        QRect cr = contentsRect();

        // Add two pixel margin on the left side
        cr.rLeft() += 3;
        p->drawText(cr, AlignAuto | AlignVCenter, d->message);
        p->setPen( tmp );
    }
}

void DLineEdit::dropEvent(QDropEvent *e)
{
    d->drawMsg = false;
    KLineEdit::dropEvent(e);
}

void DLineEdit::focusInEvent(QFocusEvent *e)
{
    if (d->drawMsg)
    {
        d->drawMsg = false;
        repaint();
    }
    QLineEdit::focusInEvent(e);
}

void DLineEdit::focusOutEvent(QFocusEvent *e)
{
    if (text().isEmpty())
    {
        d->drawMsg = true;
        repaint();
    }
    QLineEdit::focusOutEvent(e);
}

// ---------------------------------------------------------------------

class SearchTextBarPriv
{
public:

    SearchTextBarPriv()
    {
        textQueryCompletion = false;
        searchEdit          = 0;
        clearButton         = 0;
    }

    bool         textQueryCompletion;

    QToolButton *clearButton;

    DLineEdit   *searchEdit;
};

SearchTextBar::SearchTextBar(QWidget *parent, const char* name, const QString &msg)
             : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new SearchTextBarPriv;
    setFocusPolicy(QWidget::NoFocus);
    setName(name);

    QHBoxLayout *hlay = new QHBoxLayout(this);

    d->clearButton = new QToolButton(this);
    d->clearButton->setEnabled(false);
    d->clearButton->setAutoRaise(true);
    d->clearButton->setIconSet(kapp->iconLoader()->loadIcon("clear_left",
                               KIcon::Toolbar, KIcon::SizeSmall));

    d->searchEdit     = new DLineEdit(msg, this);
    KCompletion *kcom = new KCompletion;
    kcom->setOrder(KCompletion::Sorted);
    d->searchEdit->setCompletionObject(kcom, true);
    d->searchEdit->setAutoDeleteCompletionObject(true);
    d->searchEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    hlay->setSpacing(0);
    hlay->setMargin(0);
    hlay->addWidget(d->searchEdit);
    hlay->addWidget(d->clearButton);

    connect(d->clearButton, SIGNAL(clicked()),
            d->searchEdit, SLOT(clear()));

    connect(d->searchEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTextChanged(const QString&)));

    KConfig *config = kapp->config();
    config->setGroup(name + QString(" Search Text Tool"));
    d->searchEdit->setCompletionMode((KGlobalSettings::Completion)config->readNumEntry("AutoCompletionMode", 
                                      (int)KGlobalSettings::CompletionAuto));
}

SearchTextBar::~SearchTextBar()
{
    KConfig *config = kapp->config();
    config->setGroup(name() + QString(" Search Text Tool"));
    config->writeEntry("AutoCompletionMode", (int)d->searchEdit->completionMode());
    config->sync();

    delete d;
}

void SearchTextBar::setEnableTextQueryCompletion(bool b)
{
    d->textQueryCompletion = b;
}

bool SearchTextBar::textQueryCompletion() const
{
    return d->textQueryCompletion;
}

void SearchTextBar::setText(const QString& text)
{
    d->searchEdit->setText(text);
}

QString SearchTextBar::text() const
{
    return d->searchEdit->text();
}

DLineEdit *SearchTextBar::lineEdit() const
{
    return d->searchEdit;
}

void SearchTextBar::slotTextChanged(const QString& text)
{
    if (d->searchEdit->text().isEmpty())
        d->searchEdit->unsetPalette();

    d->clearButton->setEnabled(text.isEmpty() ? false : true);

    emit signalTextChanged(text);
}

void SearchTextBar::slotSearchResult(bool match)
{
    if (d->searchEdit->text().isEmpty())
    {
        d->searchEdit->unsetPalette();
        return;
    }

    QPalette pal = d->searchEdit->palette();
    pal.setColor(QPalette::Active, QColorGroup::Base,
                 match ? QColor(200, 255, 200) :
                 QColor(255, 200, 200));
    pal.setColor(QPalette::Active, QColorGroup::Text, Qt::black);
    d->searchEdit->setPalette(pal);

    // If search result match the text query, we put the text 
    // in auto-completion history.
    if (d->textQueryCompletion && match)
        d->searchEdit->completionObject()->addItem(d->searchEdit->text());
}

}  // namespace Digikam
