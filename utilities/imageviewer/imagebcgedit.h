//////////////////////////////////////////////////////////////////////////////
//
//    IMAGEBCGEDIT.H
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef IMAGEBCGEDIT_H
#define IMAGEBCGEDIT_H

// KDE includes.

#include <kdialogbase.h>

class QCheckBox;
class QLabel;

class KIntNumInput;

class ImageBCGEdit : public KDialogBase
{
    Q_OBJECT

public:

    ImageBCGEdit(QWidget* parent);
    ~ImageBCGEdit();

signals:

    void signalGammaValueChanged(int);
    void signalBrightnessValueChanged(int);
    void signalContrastValueChanged(int);
    void signalPreviewEnabled(bool); 
    
protected :

    KIntNumInput *m_gammaValue;
    KIntNumInput *m_brightnessValue;
    KIntNumInput *m_contrastValue;
    
    QLabel       *m_label_gammaValue;
    QLabel       *m_label_brightnessValue;
    QLabel       *m_label_contrastValue;
    
    QCheckBox    *m_preview;
};

#endif  // IMAGEBCGEDIT_H 
