/* ============================================================
 * File  : imageeffect_inserttext.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-14
 * Description : a digiKam image plugin for insert text  
 *               to an image.
 * 
 * Copyright 2005 by Gilles Caulier
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

#ifndef IMAGEEFFECT_INSERTEXT_H
#define IMAGEEFFECT_INSERTEXT_H

// Qt includes.

#include <qcolor.h>
#include <qimage.h>

// Digikam includes.

#include <digikamheaders.h>

class QLabel;
class QFont;
class QHButtonGroup;
class QComboBox;
class QCheckBox;

class KTextEdit;
class KColorButton;

namespace DigikamInsertTextImagesPlugin
{

class InsertTextWidget;
class FontChooserWidget;

class ImageEffect_InsertText : public Digikam::ImageDlgBase
{
    Q_OBJECT
    
public:

    ImageEffect_InsertText(QWidget *parent, QString title, QFrame* banner);
    ~ImageEffect_InsertText();

private:
    
    int                m_alignTextMode;
    int                m_defaultSizeFont;
        
    QComboBox         *m_textRotation;
    
    QCheckBox         *m_borderText;    
    QCheckBox         *m_transparentText;
    
    QHButtonGroup     *m_alignButtonGroup;
        
    QFont              m_textFont;
    
    KColorButton      *m_fontColorButton;
    
    FontChooserWidget *m_fontChooserWidget;
    
    KTextEdit         *m_textEdit;
    
    InsertTextWidget  *m_previewWidget;

private:
        
    void writeSettings(void);
    
private slots:

    void readSettings(void);
    
    void slotOk();
    void slotDefault();
    void slotFontPropertiesChanged(const QFont &font);
    void slotUpdatePreview();
    void slotAlignModeChanged(int mode);
};

}  // NameSpace DigikamInsertTextImagesPlugin

#endif /* IMAGEEFFECT_INSERTEXT_H */
