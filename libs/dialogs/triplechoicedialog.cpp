/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-03
 * Description : dialog which provides at least three choices, plus a cancel button
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "triplechoicedialog.moc"

// Qt includes

#include <QSignalMapper>
#include <QToolBar>
#include <QToolButton>

// KDE includes

#include <kiconloader.h>
#include <kpushbutton.h>

namespace Digikam
{

class TripleChoiceDialog::Private
{
public:

    Private()
        : clicked(KDialog::None),
          iconSize(KIconLoader::SizeMedium),
          toolBar(0),
          secondSeparator(0)
    {
    }

    void checkToolBar()
    {
        if (!toolBar)
        {
            toolBar = new QToolBar;
            toolBar->setOrientation(Qt::Vertical);
            toolBar->setIconSize(QSize(iconSize, iconSize));
            toolBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
            toolBar->addSeparator();
            secondSeparator = toolBar->addSeparator();
        }
    }

public:

    int               clicked;
    QSignalMapper     mapper;
    int               iconSize;
    QToolBar*         toolBar;
    QAction*          secondSeparator;
};

TripleChoiceDialog::TripleChoiceDialog(QWidget* const parent)
    : KDialog(parent),
      d(new Private)
{
    setButtons(Ok | Apply | Cancel);
    showButtonSeparator(false);

    button(Ok)->setVisible(false);
    button(Apply)->setVisible(false);

    connect(&d->mapper, SIGNAL(mapped(int)),
            this, SLOT(slotButtonClicked(int)));
}

TripleChoiceDialog::~TripleChoiceDialog()
{
    delete d;
}

void TripleChoiceDialog::setShowCancelButton(bool show)
{
    button(Cancel)->setVisible(show);
}

void TripleChoiceDialog::setIconSize(int size)
{
    d->iconSize = size;
}

int TripleChoiceDialog::iconSize() const
{
    return d->iconSize;
}

QToolButton* TripleChoiceDialog::addChoiceButton(int key, const QString& iconName, const QString& text)
{
    QToolButton* button = new QToolButton;
    button->setText(text);
    button->setIcon(SmallIcon(iconName, d->iconSize));
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setAutoRaise(true);
    button->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    d->mapper.setMapping(button, key);

    connect(button, SIGNAL(clicked()),
            &d->mapper, SLOT(map()));

    d->checkToolBar();
    d->toolBar->insertWidget(d->secondSeparator, button);

    return button;
}

QToolButton* TripleChoiceDialog::addChoiceButton(int key, const QString& text)
{
    return addChoiceButton(key, QString(), text);
}

QToolButton* TripleChoiceDialog::choiceButton(int key) const
{
    return qobject_cast<QToolButton*>(d->mapper.mapping(key));
}

int TripleChoiceDialog::clickedButton() const
{
    return d->clicked;
}

void TripleChoiceDialog::slotButtonClicked(int button)
{
    d->clicked = button;

    emit buttonClicked(static_cast<KDialog::ButtonCode>(button));

    if (button == Cancel)
    {
        reject();
    }
    else
    {
        accept();
    }
}

QWidget* TripleChoiceDialog::buttonContainer() const
{
    d->checkToolBar();
    return d->toolBar;
}

} // namespace Digikam
