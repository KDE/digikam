/* ============================================================
 * File  : imeventfilter.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-14
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <qevent.h>
#include <qkeysequence.h>

#include "imageview.h"
#include "imeventfilter.h"

IMEventFilter::IMEventFilter(QObject* parent)
    : QObject(parent)
{
    view_ = static_cast<ImageView *>(parent);
}

IMEventFilter::~IMEventFilter()
{
    
}

bool IMEventFilter::eventFilter(QObject *, QEvent *e)
{
    if (e->type() != QEvent::KeyPress)
        return false;

    QKeyEvent *k = (QKeyEvent *)e;

    int key = k->key();
    
    if ( k->state() & ShiftButton )
        key |= SHIFT;
    if ( k->state() & ControlButton )
        key |= CTRL;
    if ( k->state() & AltButton )
        key |= ALT;
    
    emit signalKeyPress(key);

    //  Eat keypress event
    return true;
    
}

#include "imeventfilter.moc"
