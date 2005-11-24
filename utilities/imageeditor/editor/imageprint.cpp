/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-13
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * Original printing code from Kuickshow program.
 * Copyright (C) 2002 Carsten Pfeiffer <pfeiffer at kde.org>
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

// Qt lib includes

#include <qobject.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qstring.h>
#include <qsize.h>
#include <qcursor.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qcheckbox.h>
#include <qfont.h>
#include <qgrid.h>
#include <qimage.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qcolor.h>
#include <qcombobox.h>

// KDE lib includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kimageio.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <knuminput.h>
#include <kprinter.h>
#include <ktempfile.h>
#include <kpropertiesdialog.h>

// Local includes

#include "imageprint.h"

// Image printdialog class -------------------------------------------------------------


ImageEditorPrintDialogPage::ImageEditorPrintDialogPage( QWidget *parent, const char *name )
                          : KPrintDialogPage( parent, name )
{
    setTitle( i18n("Image Settings") );

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( KDialog::marginHint() );
    layout->setSpacing( KDialog::spacingHint() );

    m_addFileName = new QCheckBox( i18n("Print fi&lename below image"), this);
    m_addFileName->setChecked( false );
    layout->addWidget( m_addFileName );

    m_blackwhite = new QCheckBox ( i18n("Print image in &black and white"), this);
    m_blackwhite->setChecked( false );
    layout->addWidget (m_blackwhite );

    QVButtonGroup *group = new QVButtonGroup( i18n("Scaling"), this );
    group->setRadioButtonExclusive( true );
    layout->addWidget( group );

    m_scaleToFit = new QRadioButton( i18n("Scale image to &fit"), group );
    m_scaleToFit->setChecked( true );

    QWidget *widget = new QWidget( group );
    QGridLayout *grid = new QGridLayout( widget, 3, 3 );
    grid->addColSpacing( 0, 30 );
    grid->setColStretch( 0, 0 );
    grid->setColStretch( 1, 1 );
    grid->setColStretch( 2, 10 );

    m_scale = new QRadioButton( i18n("Print e&xact size: "), widget );
    grid->addMultiCellWidget( m_scale, 0, 0, 0, 1 );
    group->insert( m_scale );
    
    connect( m_scale, SIGNAL( toggled( bool )),
             SLOT( toggleScaling( bool )));

    m_units = new KComboBox( false, widget, "unit combobox" );
    grid->addWidget( m_units, 0, 2, AlignLeft );
    m_units->insertItem( i18n("Centimeters") );
    m_units->insertItem( i18n("Inches") );

    m_width = new KDoubleNumInput( widget, "exact width" );
    grid->addWidget( m_width, 1, 1 );
    m_width->setLabel( i18n("&Width:" ) );
    m_width->setMinValue( 1 );

    m_height = new KDoubleNumInput( widget, "exact height" );
    grid->addWidget( m_height, 2, 1 );
    m_height->setLabel( i18n("&Height:" ) );
    m_height->setMinValue( 1 );
}

ImageEditorPrintDialogPage::~ImageEditorPrintDialogPage()
{
}

void ImageEditorPrintDialogPage::getOptions( QMap<QString,QString>& opts,
                                           bool /*incldef*/ )
{
    QString t = "true";
    QString f = "false";

    opts["app-imageeditor-printFilename"] = m_addFileName->isChecked() ? t : f;
    opts["app-imageeditor-blackwhite"] = m_blackwhite->isChecked() ? t : f;
    opts["app-imageeditor-scaleToFit"] = m_scaleToFit->isChecked() ? t : f;
    opts["app-imageeditor-scale"] = m_scale->isChecked() ? t : f;
    opts["app-imageeditor-scale-unit"] = m_units->currentText();
    opts["app-imageeditor-scale-width"] = QString::number( m_width->value() );
    opts["app-imageeditor-scale-height"] = QString::number( m_height->value() );
}


void ImageEditorPrintDialogPage::setOptions( const QMap<QString,QString>& opts )
{
    QString t = "true";
    QString f = "false";

    m_addFileName->setChecked( opts["app-imageeditor-printFilename"] != f );
    // This sound strange, but if I copy the code on the line above, the checkbox
    // was always checked. And this isn't the wanted behavior. So, with this works.
    // KPrint magic ;-)
    m_blackwhite->setChecked ( false );
    m_scaleToFit->setChecked( opts["app-imageeditor-scaleToFit"] != f );
    m_scale->setChecked( opts["app-imageeditor-scale"] == t );

    m_units->setCurrentItem( opts["app-imageeditor-scale-unit"] );

    bool   ok;
    double val;

    val = opts["app-imageeditor-scale-width"].toDouble( &ok );
    
    if ( ok )
        m_width->setValue( val );
    
    val = opts["app-imageeditor-scale-height"].toDouble( &ok );
    
    if ( ok )
        m_height->setValue( val );

    if ( m_scale->isChecked() == m_scaleToFit->isChecked() )
        m_scaleToFit->setChecked( !m_scale->isChecked() );

    toggleScaling( m_scale->isChecked() );
}

void ImageEditorPrintDialogPage::toggleScaling( bool enable )
{
    m_width->setEnabled( enable );
    m_height->setEnabled( enable );
    m_units->setEnabled( enable );
}

// Image print class -----------------------------------------------------------------

ImagePrint::ImagePrint(QImage& image, KPrinter& printer,
                       const QString& filename)
          : m_image( image ), m_printer( printer ),
            m_filename( filename )
{
}

ImagePrint::~ImagePrint()
{
}

bool ImagePrint::printImageWithQt()
{
    if ( m_image.isNull() ) 
    {
        kdWarning() << "Supplied Image for printing is null" << endl;
        return false;
    }

    QString t = "true";
    QString f = "false";
    
    // Black & white print ?
    if ( m_printer.option( "app-imageeditor-blackwhite" ) != f)
    {
        m_image = m_image.convertDepth( 1, Qt::MonoOnly |
                                       Qt::ThresholdDither |
                                       Qt::AvoidDither );
    }

    QPainter p;
    p.begin( &m_printer );

    QPaintDeviceMetrics metrics( &m_printer );
    p.setFont( KGlobalSettings::generalFont() );
    QFontMetrics fm = p.fontMetrics();


    int filenameOffset = 0;
    int w = metrics.width();
    int h = metrics.height();

    bool printFilename = m_printer.option( "app-imageeditor-printFilename" ) != f;
    if ( printFilename )
    {
        // filename goes into one line!
        filenameOffset = fm.lineSpacing() + 14;
        h -= filenameOffset; 
    }
    
    if ( m_printer.option( "app-imageeditor-scaleToFit" ) != f )
    {
        
        // Scale image to fit pagesize
        m_image = m_image.smoothScale( w, h, QImage::ScaleMin );
    }
    else
    {
        // scale image to exact dimensions
        QString unit  = m_printer.option("app-imageeditor-scale-unit");
        double  wunit = m_printer.option("app-imageeditor-scale-width").toDouble();
        double  hunit = m_printer.option("app-imageeditor-scale-height").toDouble();
        int     wresize, hresize;
        if (unit == i18n("Centimeters"))
        {
            // centimeters
            wresize = (int)(metrics.logicalDpiX() * wunit / 2.54);
            hresize = (int)(metrics.logicalDpiY() * hunit / 2.54);
        }
        else
        {
            // inches
            wresize  = (int)(metrics.logicalDpiX() * wunit);
            hresize  = (int)(metrics.logicalDpiY() * hunit);
        }

        m_image = m_image.smoothScale( wresize, hresize, QImage::ScaleMin );
    }

    // Align image.
    bool ok = false;
    int alignment = m_printer.option("app-imageeditor-alignment").toInt( &ok );
    
    if ( !ok )
    {
        // default
        alignment = Qt::AlignCenter; 
    }

    int x = 0;
    int y = 0;

    // TODO: a GUI for this in ImagePrintDialogPage!

    // x - alignment
    if ( alignment & Qt::AlignHCenter )
        x = (w - m_image.width())/2;
    else if ( alignment & Qt::AlignLeft )
        x = 0;
    else if ( alignment & Qt::AlignRight )
        x = w - m_image.width();

    // y - alignment
    if ( alignment & Qt::AlignVCenter )
        y = (h - m_image.height())/2;
    else if ( alignment & Qt::AlignTop )
        y = 0;
    else if ( alignment & Qt::AlignBottom )
        y = h - m_image.height();

    // Perform the actual drawing.
    p.drawImage( x, y, m_image );

    if ( printFilename )
    {
        QString fname = minimizeString( m_filename, fm, w );
        
        if ( !fname.isEmpty() )
        {
            int fw = fm.width( fname );
            int x = (w - fw)/2;
            int y = metrics.height() - filenameOffset/2;
            p.drawText( x, y, fname );
        }
    }

    p.end();

    return true;
}


QString ImagePrint::minimizeString( QString text, const QFontMetrics& metrics,
                                    int maxWidth )
{
    // no sense to cut that tiny little string
    if ( text.length() <= 5 )
        return QString();

    bool changed = false;
    
    while ( metrics.width( text ) > maxWidth )
    {
        int mid = text.length() / 2;
        // remove 2 characters in the middle
        text.remove( mid, 2 ); 
        changed = true;
    }

    // add "..." in the middle
    if ( changed ) 
    {
        int mid = text.length() / 2;
        
        // sanity check
        if ( mid <= 5 ) 
            return QString();

        text.replace( mid - 1, 3, "..." );
    }

    return text;
}

#include "imageprint.moc"
