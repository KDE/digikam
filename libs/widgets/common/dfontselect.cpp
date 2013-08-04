/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-23
 * Description : a widget to select between system font or a custom font.
 *
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dfontselect.moc"

// Qt includes

#include <QLabel>
#include <QEvent>
#include <QPushButton>

// KDE includes

#include <kfontdialog.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kcombobox.h>

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

    KComboBox*            modeCombo;

    DFontSelect::FontMode mode;
};

DFontSelect::DFontSelect(const QString& text, QWidget* const parent)
    : KHBox(parent), d(new Private)
{
    d->label     = new QLabel(this);
    d->label->setText(text);
    d->space     = new QLabel(this);

    if (text.isEmpty())
    {
        d->label->hide();
        d->space->hide();
    }

    d->modeCombo = new KComboBox(this);
    d->modeCombo->addItem(i18n("System Font"));
    d->modeCombo->addItem(i18n("Custom Font"));

    d->chooseFontButton = new QPushButton(i18n("Choose..."), this);

    setSpacing(KDialog::spacingHint());
    setMargin(0);
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
    d->modeCombo->setFont(font());
    d->chooseFontButton->setEnabled(d->mode == CustomFont);
}

DFontSelect::FontMode DFontSelect::mode() const
{
    return d->mode;
}

QFont DFontSelect::font() const
{
    return (d->mode == CustomFont) ? d->font : KGlobalSettings::generalFont();
}

void DFontSelect::setFont(const QFont& font)
{
    d->font = font;

    if (d->font == KGlobalSettings::generalFont())
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

    return KHBox::event(e);
}

void DFontSelect::slotOpenFontDialog()
{
    QFont f          = font();
    const int result = KFontDialog::getFont(f, KFontChooser::NoDisplayFlags, this);

    if (result == KFontDialog::Accepted)
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

}  // namespace Digikam
