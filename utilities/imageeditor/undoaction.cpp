/* ============================================================
 * File  : undoaction.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-02-06
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include <kdebug.h>

#include "imlibinterface.h"
#include "undoaction.h"

UndoAction::UndoAction(Digikam::ImlibInterface* iface)
    : m_iface(iface)
{
}

UndoAction::~UndoAction()
{
}

UndoActionRotate::UndoActionRotate(Digikam::ImlibInterface* iface,
                                   UndoActionRotate::Angle angle)
    : UndoAction(iface), m_angle(angle)
{
}

UndoActionRotate::~UndoActionRotate()
{
}

void UndoActionRotate::rollBack()
{
    switch(m_angle)
    {
    case(R90):
        m_iface->rotate270(false);
        return;
    case(R180):
        m_iface->rotate180(false);
        return;
    case(R270):
        m_iface->rotate90(false);
        return;
    default:
        kdWarning() << "Unknown rotate angle specified" << endl;
    }
}

UndoActionFlip::UndoActionFlip(Digikam::ImlibInterface* iface,
                               UndoActionFlip::Direction dir)
    : UndoAction(iface), m_dir(dir)
{
    
}

UndoActionFlip::~UndoActionFlip()
{
    
}

void UndoActionFlip::rollBack()
{
    switch(m_dir)
    {
    case(Horizontal):
        m_iface->flipHoriz(false);
        return;
    case(Vertical):
        m_iface->flipVert(false);
        return;
    default:
        kdWarning() << "Unknown flip direction specified" << endl;
    }
}

UndoActionBCG::UndoActionBCG(Digikam::ImlibInterface* iface,
                             double gamma, double brightness,
                             double contrast)
    : UndoAction(iface), m_gamma(gamma), m_brightness(brightness),
      m_contrast(contrast)
{
    
}
UndoActionBCG::~UndoActionBCG()
{
    
}

void UndoActionBCG::rollBack()
{
    m_iface->changeBCG(m_gamma, m_brightness, m_contrast);    
}

UndoActionIrreversible::UndoActionIrreversible(Digikam::ImlibInterface* iface)
    : UndoAction(iface)
{
}

UndoActionIrreversible::~UndoActionIrreversible()
{
    
}

void UndoActionIrreversible::rollBack()
{
}

