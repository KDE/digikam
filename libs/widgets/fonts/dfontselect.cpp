/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-23
 * Description : a widget to select between system font or a custom font.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dfontselect.h"

// Qt includes

#include <QLabel>
#include <QEvent>
#include <QPushButton>
#include <QFontDatabase>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QFontDialog>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class DFontSelect::Private
{
public:

    Private() :
        space(0),
        label(0),
        chooseFontButton(0),
        modeCombo(0),
        mode(DFontSelect::SystemFont)
    {
    }

    QLabel*               space;
    QLabel*               label;

    QFont                 font;

    QPushButton*          chooseFontButton;

    QComboBox*            modeCombo;

    DFontSelect::FontMode mode;
};

DFontSelect::DFontSelect(const QString& text, QWidget* const parent)
    : DHBox(parent),
      d(new Private)
{
    d->label     = new QLabel(this);
    d->label->setText(text);
    d->space     = new QLabel(this);

    if (text.isEmpty())
    {
        d->label->hide();
        d->space->hide();
    }

    d->modeCombo = new QComboBox(this);
    d->modeCombo->addItem(i18n("System Font"));
    d->modeCombo->addItem(i18n("Custom Font"));

    d->chooseFontButton = new QPushButton(i18n("Choose..."), this);

    setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    setContentsMargins(QMargins());
    setStretchFactor(d->space, 10);

    connect(d->modeCombo, SIGNAL(activated(int)),
            this, SLOT(slotChangeMode(int)));

    connect(d->chooseFontButton, SIGNAL(clicked()),
            this, SLOT(slotOpenFontDialog()));

    slotChangeMode(d->modeCombo->currentIndex());
}

DFontSelect::~DFontSelect()
{
    delete d;
}

void DFontSelect::setMode(FontMode mode)
{
    d->mode = mode;
    d->modeCombo->setCurrentIndex(d->mode);
    d->modeCombo->setToolTip(i18n("Current font: %1 - %2", font().family(), font().pointSize()));
    d->chooseFontButton->setEnabled(d->mode == CustomFont);
}

DFontSelect::FontMode DFontSelect::mode() const
{
    return d->mode;
}

QFont DFontSelect::font() const
{
    return (d->mode == CustomFont) ? d->font : QFontDatabase::systemFont(QFontDatabase::GeneralFont);
}

void DFontSelect::setFont(const QFont& font)
{
    d->font = font;

    if (d->font == QFontDatabase::systemFont(QFontDatabase::GeneralFont))
    {
        setMode(SystemFont);
    }
    else
    {
        setMode(CustomFont);
    }
}

bool DFontSelect::event(QEvent* e)
{
    if (e->type() == QEvent::Polish)
    {
        d->modeCombo->setFont(font());
    }

    return DHBox::event(e);
}

void DFontSelect::slotOpenFontDialog()
{
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, font(), this);

    if (ok)
    {
        d->font = f;
        d->modeCombo->setFont(d->font);
        emit signalFontChanged();
    }
}

void DFontSelect::slotChangeMode(int index)
{
    setMode((index == CustomFont) ? CustomFont : SystemFont);
    emit signalFontChanged();
}

} // namespace Digikam
