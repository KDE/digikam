/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-10-05
 * Description : a tool bar for slideshow
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "toolbar.moc"

// Qt includes

#include <QHBoxLayout>
#include <QPixmap>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{

class ToolBar::ToolBarPriv
{
public:

    ToolBarPriv() :
        canHide(true),
        playBtn(0),
        stopBtn(0),
        nextBtn(0),
        prevBtn(0)
    {
    }

    bool         canHide;

    QToolButton* playBtn;
    QToolButton* stopBtn;
    QToolButton* nextBtn;
    QToolButton* prevBtn;
};

ToolBar::ToolBar(QWidget* parent)
    : QWidget(parent), d(new ToolBarPriv)
{
    QHBoxLayout* lay = new QHBoxLayout();
    setLayout(lay);
    d->playBtn = new QToolButton(this);
    d->prevBtn = new QToolButton(this);
    d->nextBtn = new QToolButton(this);
    d->stopBtn = new QToolButton(this);
    d->playBtn->setCheckable(true);

    KIconLoader* loader = KIconLoader::global();
    d->playBtn->setIcon(loader->loadIcon("media-playback-pause", KIconLoader::NoGroup, 22));
    d->prevBtn->setIcon(loader->loadIcon("media-skip-backward",  KIconLoader::NoGroup, 22));
    d->nextBtn->setIcon(loader->loadIcon("media-skip-forward",   KIconLoader::NoGroup, 22));
    d->stopBtn->setIcon(loader->loadIcon("media-playback-stop",  KIconLoader::NoGroup, 22));

    lay->addWidget(d->playBtn);
    lay->addWidget(d->prevBtn);
    lay->addWidget(d->nextBtn);
    lay->addWidget(d->stopBtn);

    adjustSize();
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(d->playBtn, SIGNAL(toggled(bool)),
            this, SLOT(slotPlayBtnToggled()));

    connect(d->nextBtn, SIGNAL(clicked()),
            this, SLOT(slotNexPrevClicked()));

    connect(d->prevBtn, SIGNAL(clicked()),
            this, SLOT(slotNexPrevClicked()));

    connect(d->nextBtn, SIGNAL(clicked()),
            this, SIGNAL(signalNext()));

    connect(d->prevBtn, SIGNAL(clicked()),
            this, SIGNAL(signalPrev()));

    connect(d->stopBtn, SIGNAL(clicked()),
            this, SIGNAL(signalClose()));
}

ToolBar::~ToolBar()
{
    delete d;
}

bool ToolBar::canHide() const
{
    return d->canHide;
}

bool ToolBar::isPaused() const
{
    return d->playBtn->isChecked();
}

void ToolBar::setPaused(bool val)
{
    if (val == isPaused())
    {
        return;
    }

    d->playBtn->setChecked(val);
    slotPlayBtnToggled();
}

void ToolBar::setEnabledPlay(bool val)
{
    d->playBtn->setEnabled(val);
}

void ToolBar::setEnabledNext(bool val)
{
    d->nextBtn->setEnabled(val);
}

void ToolBar::setEnabledPrev(bool val)
{
    d->prevBtn->setEnabled(val);
}

void ToolBar::slotPlayBtnToggled()
{
    KIconLoader* loader = KIconLoader::global();

    if (d->playBtn->isChecked())
    {
        d->canHide = false;
        d->playBtn->setIcon(loader->loadIcon("media-playback-start", KIconLoader::NoGroup, 22));
        emit signalPause();
    }
    else
    {
        d->canHide = true;
        d->playBtn->setIcon(loader->loadIcon("media-playback-pause", KIconLoader::NoGroup, 22));
        emit signalPlay();
    }
}

void ToolBar::slotNexPrevClicked()
{
    if (!d->playBtn->isChecked())
    {
        d->playBtn->setChecked(true);
        d->canHide = false;
        KIconLoader* loader = KIconLoader::global();
        d->playBtn->setIcon(loader->loadIcon("media-playback-start", KIconLoader::NoGroup, 22));
        emit signalPause();
    }
}

void ToolBar::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case (Qt::Key_Space):
        {
            if (d->playBtn->isEnabled())
            {
                d->playBtn->animateClick();
            }

            break;
        }

        case (Qt::Key_PageUp):
        {
            if (d->prevBtn->isEnabled())
            {
                d->prevBtn->animateClick();
            }

            break;
        }

        case (Qt::Key_PageDown):
        {
            if (d->nextBtn->isEnabled())
            {
                d->nextBtn->animateClick();
            }

            break;
        }

        case (Qt::Key_Escape):
        {
            if (d->stopBtn->isEnabled())
            {
                d->stopBtn->animateClick();
            }

            break;
        }

        default:
            break;
    }

    e->accept();
}

}   // namespace Digikam
