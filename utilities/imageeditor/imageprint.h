/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-13
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
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

#ifndef IMAGEPRINT_H
#define IMAGEPRINT_H

// Qt lib includes

#include <qwidget.h>
#include <qstring.h>

// KDE lib includes

#include <kurl.h>
#include <kprinter.h>
#include <kdeprint/kprintdialogpage.h>

class QCheckBox;
class QRadioButton;

class KComboBox;
class KDoubleNumInput;

class ImagePrint  
{
public:

    ImagePrint(QImage& image, KPrinter& printer, 
               const QString& fileName);
    ~ImagePrint();

    bool printImageWithQt();

private:
    
    QString minimizeString( QString text, const QFontMetrics& metrics,
                            int maxWidth );                           
private:
    
    QImage    m_image;
    KPrinter& m_printer;
    QString   m_filename;
    
};


///////////////////////////////////////////////////////////////////////////////////

class ImageEditorPrintDialogPage : public KPrintDialogPage
{
    Q_OBJECT

public:

    ImageEditorPrintDialogPage( QWidget *parent = 0L, const char *name = 0 );
    ~ImageEditorPrintDialogPage();

    virtual void getOptions(QMap<QString,QString>& opts, bool incldef = false);
    virtual void setOptions(const QMap<QString,QString>& opts);

private slots:

    void toggleScaling( bool enable );

private:

    QRadioButton    *m_scaleToFit;
    QRadioButton    *m_scale;
    KDoubleNumInput *m_width;
    KDoubleNumInput *m_height;
    KComboBox       *m_units;
    QCheckBox       *m_addFileName;
    QCheckBox       *m_blackwhite;
};

#endif // IMAGEPRINT_H 
