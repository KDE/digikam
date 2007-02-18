/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Joern Ahrens <joern.ahrens@kdemail.net>
 * Date   : 2005-02-06
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>, Joern Ahrens <joern.ahrens@kdemail.net>
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

// Local includes.

#include "ddebug.h"
#include "dimginterface.h"
#include "undoaction.h"

namespace Digikam
{

UndoAction::UndoAction(DImgInterface* iface)
    : m_iface(iface)
{
    m_title = i18n("unknown");
}

UndoAction::~UndoAction()
{
}

QString UndoAction::getTitle() const
{
    return m_title;
}

UndoActionRotate::UndoActionRotate(DImgInterface* iface,
                                   UndoActionRotate::Angle angle)
                : UndoAction(iface), m_angle(angle)
{
    switch(m_angle)
    {
        case(R90):
            m_title = i18n("Rotate 90 Degrees");
            break;
        case(R180):
            m_title = i18n("Rotate 180 Degrees");
            break;
        case(R270):
            m_title = i18n("Rotate 270 Degrees");
            break;
    }
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
            DWarning() << "Unknown rotate angle specified" << endl;
    }
}

void UndoActionRotate::execute()
{
    switch(m_angle)
    {
        case R90:
            m_iface->rotate90(false);
            return;
        case R180:
            m_iface->rotate180(false);
            return;
        case R270:
            m_iface->rotate270(false);
            return;
        default:
            DWarning() << "Unknown rotate angle specified" << endl;
    }
}

UndoActionFlip::UndoActionFlip(DImgInterface* iface,
                               UndoActionFlip::Direction dir)
              : UndoAction(iface), m_dir(dir)
{
    if(m_dir == Horizontal)
        m_title = i18n("Flip Horizontal");
    else if(m_dir == Vertical)
        m_title = i18n("Flip Vertical");
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
            DWarning() << "Unknown flip direction specified" << endl;
    }
}

void UndoActionFlip::execute()
{
    rollBack();
}

UndoActionBCG::UndoActionBCG(DImgInterface* iface,
                             double oldGamma, double oldBrightness,
                             double oldContrast, double newGamma,
                             double newBrightness, double newContrast)
             : UndoAction(iface), m_oldGamma(oldGamma), m_oldBrightness(oldBrightness),
               m_oldContrast(oldContrast), m_newGamma(newGamma), m_newBrightness(newBrightness),
               m_newContrast(newContrast)
{
    m_title = i18n("Brightness,Contrast,Gamma");    
}

UndoActionBCG::~UndoActionBCG()
{
}

void UndoActionBCG::rollBack()
{
    m_iface->changeBCG(m_oldGamma, m_oldBrightness, m_oldContrast);
}

void UndoActionBCG::execute()
{
    m_iface->changeBCG(m_newGamma, m_newBrightness, m_newContrast);
}

UndoActionIrreversible::UndoActionIrreversible(DImgInterface* iface,
                                               const QString &title)
    : UndoAction(iface)
{
    m_title = title;
}

UndoActionIrreversible::~UndoActionIrreversible()
{
}

void UndoActionIrreversible::rollBack()
{
}

void UndoActionIrreversible::execute()
{
}

}  // namespace Digikam
