/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-10-05
 * Description : a tool bar for slideshow
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "slidetoolbar.moc"

// Qt includes

#include <QPixmap>
#include <QToolButton>
#include <QDesktopWidget>
#include <QActionGroup>

// KDE includes

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kselectaction.h>
#include <kdebug.h>
#include <kmenu.h>

// Local includes

#include "slidehelp.h"

namespace Digikam
{

class SlideToolBar::Private
{
public:

    Private() :
        iconSize(KIconLoader::SizeSmall),
        playBtn(0),
        stopBtn(0),
        nextBtn(0),
        prevBtn(0),
        screenSelectBtn(0),
        desktop(kapp->desktop()),
        loader(KIconLoader::global())
    {
    }

    const int       iconSize;

    QToolButton*    playBtn;
    QToolButton*    stopBtn;
    QToolButton*    nextBtn;
    QToolButton*    prevBtn;
    QToolButton*    screenSelectBtn;

    QDesktopWidget* desktop;

    KIconLoader*    loader;
};

SlideToolBar::SlideToolBar(const SlideShowSettings& settings, QWidget* const parent)
    : KHBox(parent), d(new Private)
{
    setMouseTracking(true);
    setMargin(0);

    d->playBtn = new QToolButton(this);
    d->prevBtn = new QToolButton(this);
    d->nextBtn = new QToolButton(this);
    d->stopBtn = new QToolButton(this);

    d->playBtn->setCheckable(true);
    d->playBtn->setFocusPolicy(Qt::NoFocus);
    d->prevBtn->setFocusPolicy(Qt::NoFocus);
    d->nextBtn->setFocusPolicy(Qt::NoFocus);
    d->stopBtn->setFocusPolicy(Qt::NoFocus);

    d->playBtn->setIcon(d->loader->loadIcon("media-playback-pause", KIconLoader::Toolbar, d->iconSize));
    d->prevBtn->setIcon(d->loader->loadIcon("media-skip-backward",  KIconLoader::Toolbar, d->iconSize));
    d->nextBtn->setIcon(d->loader->loadIcon("media-skip-forward",   KIconLoader::Toolbar, d->iconSize));
    d->stopBtn->setIcon(d->loader->loadIcon("media-playback-stop",  KIconLoader::Toolbar, d->iconSize));

    d->playBtn->setIconSize(QSize(d->iconSize, d->iconSize));
    d->prevBtn->setIconSize(QSize(d->iconSize, d->iconSize));
    d->nextBtn->setIconSize(QSize(d->iconSize, d->iconSize));
    d->stopBtn->setIconSize(QSize(d->iconSize, d->iconSize));

    int num = d->desktop->numScreens();

    if (num > 1)
    {
        d->screenSelectBtn      = new QToolButton(this);
        KMenu* const screenMenu = new KMenu(d->screenSelectBtn);
        d->screenSelectBtn->setToolTip( i18n("Switch Screen"));
        d->screenSelectBtn->setIcon(d->loader->loadIcon("video-display", KIconLoader::Toolbar, d->iconSize));
        d->screenSelectBtn->setIconSize(QSize(d->iconSize, d->iconSize));
        d->screenSelectBtn->setMenu(screenMenu);
        d->screenSelectBtn->setPopupMode(QToolButton::InstantPopup);
        d->screenSelectBtn->setFocusPolicy(Qt::NoFocus);

        QActionGroup* const group = new QActionGroup(screenMenu);
        group->setExclusive(true);

        for (int i = 0 ; i < num ; ++i)
        {
            QAction* const act = screenMenu->addAction(i18nc("%1 is the screen number (0, 1, ...)", "Screen %1", i));
            act->setData(qVariantFromValue(i));
            act->setCheckable(true);
            group->addAction(act);

            if (i == settings.slideScreen)
                act->setChecked(true);
        }

        connect(screenMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotScreenSelected(QAction*)));
    }

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

SlideToolBar::~SlideToolBar()
{
    delete d;
}

bool SlideToolBar::isPaused() const
{
    return d->playBtn->isChecked();
}

void SlideToolBar::pause(bool val)
{
    if (val == isPaused())
    {
        return;
    }

    d->playBtn->setChecked(val);
    slotPlayBtnToggled();
}

void SlideToolBar::setEnabledPlay(bool val)
{
    d->playBtn->setEnabled(val);
}

void SlideToolBar::setEnabledNext(bool val)
{
    d->nextBtn->setEnabled(val);
}

void SlideToolBar::setEnabledPrev(bool val)
{
    d->prevBtn->setEnabled(val);
}

void SlideToolBar::slotPlayBtnToggled()
{
    if (d->playBtn->isChecked())
    {
        d->playBtn->setIcon(d->loader->loadIcon("media-playback-start", KIconLoader::Toolbar, d->iconSize));
        emit signalPause();
    }
    else
    {
        d->playBtn->setIcon(d->loader->loadIcon("media-playback-pause", KIconLoader::Toolbar, d->iconSize));
        emit signalPlay();
    }
}

void SlideToolBar::slotNexPrevClicked()
{
    if (!d->playBtn->isChecked())
    {
        d->playBtn->setChecked(true);
        d->playBtn->setIcon(d->loader->loadIcon("media-playback-start", KIconLoader::Toolbar, d->iconSize));
        emit signalPause();
    }
}

void SlideToolBar::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case (Qt::Key_F1):
        {
            d->playBtn->animateClick();
            SlideHelp* const help = new SlideHelp();
            help->exec();
            d->playBtn->animateClick();
            break;
        }

        case (Qt::Key_Space):
        {
            if (d->playBtn->isEnabled())
            {
                d->playBtn->animateClick();
            }

            break;
        }

        case (Qt::Key_Left):
        case (Qt::Key_Up):
        case (Qt::Key_PageUp):
        {
            if (d->prevBtn->isEnabled())
            {
                d->prevBtn->animateClick();
            }

            break;
        }

        case (Qt::Key_Right):
        case (Qt::Key_Down):
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

void SlideToolBar::slotScreenSelected(QAction* act)
{
    if (!act || act->data().type() != QVariant::Int)
        return;

    emit signalScreenSelected(act->data().toInt());
}

}   // namespace Digikam
