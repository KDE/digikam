/* ============================================================
 * File  : imageprint.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-13
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
 *
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
class KIntNumInput;

class ImagePrint  
{
public:

    ImagePrint(const QString& filename, KPrinter& printer, 
               const QString& originalFileName);
    ~ImagePrint();

    bool printImageWithQt();

private:
    
    QString   m_filename;
    KPrinter& m_printer;
    QString   m_originalFileName;
    
    void addConfigPages();
    QString minimizeString( QString text, const QFontMetrics& metrics,
                            int maxWidth );                           
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

    // return values in pixels!
    int scaleWidth() const;
    int scaleHeight() const;

    void setScaleWidth( int pixels );
    void setScaleHeight( int pixels );

    int fromUnitToPixels( float val ) const;
    float pixelsToUnit( int pixels ) const;

    QCheckBox *m_shrinkToFit;
    QRadioButton *m_scale;
    KIntNumInput *m_width;
    KIntNumInput *m_height;
    KComboBox *m_units;
    QCheckBox *m_addFileName;
    QCheckBox *m_blackwhite;

};

#endif // IMAGEPRINT_H 
