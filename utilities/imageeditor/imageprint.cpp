/* ============================================================
 * File  : imageprint.cpp
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
#include <qlayout.h>
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
#include <kapp.h>
#include <kconfig.h>
#include <kimageio.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <knuminput.h>
#include <kprinter.h>
#include <ktempfile.h>
#include <kpropertiesdialog.h>
#include <kapplication.h> 

// Local includes

#include "imageprint.h"


//////////////////////////////////// CONSTRUCTORS ///////////////////////////////////////////

ImagePrint::ImagePrint(const QString& filename, KPrinter& printer, const QString& originalFileName)
          : m_filename( filename), m_printer( printer ), m_originalFileName( originalFileName)
{
}

ImageEditorPrintDialogPage::ImageEditorPrintDialogPage( QWidget *parent, const char *name )
                          : KPrintDialogPage( parent, name )
{
    setTitle( i18n("Image Settings") );

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( KDialog::marginHint() );
    layout->setSpacing( KDialog::spacingHint() );

    m_addFileName = new QCheckBox( i18n("Print fi&lename below image"), this);
    m_addFileName->setChecked( true );
    layout->addWidget( m_addFileName );

    m_blackwhite = new QCheckBox ( i18n("Print image in &black and white"), this);
    m_blackwhite->setChecked( false );
    layout->addWidget (m_blackwhite );

    QVButtonGroup *group = new QVButtonGroup( i18n("Scaling"), this );
    group->setRadioButtonExclusive( true );
    layout->addWidget( group );
    m_shrinkToFit = new QCheckBox( i18n("Shrink image to &fit, if necessary"), group );
    m_shrinkToFit->setChecked( true );

    QWidget *widget = new QWidget( group );
    QGridLayout *grid = new QGridLayout( widget, 3, 3 );
    grid->addColSpacing( 0, 30 );
    grid->setColStretch( 0, 0 );
    grid->setColStretch( 1, 1 );
    grid->setColStretch( 2, 10 );

    m_scale = new QRadioButton( i18n("Print e&xact size: "), widget );
    m_scale->setEnabled( false ); // ###
    grid->addMultiCellWidget( m_scale, 0, 0, 0, 1 );
    group->insert( m_scale );
    
    connect( m_scale, SIGNAL( toggled( bool )),
             SLOT( toggleScaling( bool )));

    m_units = new KComboBox( false, widget, "unit combobox" );
    grid->addWidget( m_units, 0, 2, AlignLeft );
    m_units->insertItem( i18n("Millimeters") );
    m_units->insertItem( i18n("Centimeters") );
    m_units->insertItem( i18n("Inches") );

    m_width = new KIntNumInput( widget, "exact width" );
    grid->addWidget( m_width, 1, 1 );
    m_width->setLabel( i18n("&Width:" ) );
    m_width->setMinValue( 1 );

    m_height = new KIntNumInput( widget, "exact height" );
    grid->addWidget( m_height, 2, 1 );
    m_height->setLabel( i18n("&Height:" ) );
    m_height->setMinValue( 1 );
}


//////////////////////////////////// DESTRUCTOR /////////////////////////////////////////////

ImagePrint::~ImagePrint()
{
}


ImageEditorPrintDialogPage::~ImageEditorPrintDialogPage()
{
}


//////////////////////////////////////// FONCTIONS //////////////////////////////////////////

bool ImagePrint::printImageWithQt()
{
    QImage image( m_filename );
    
    if ( image.isNull() ) 
        {
        kdWarning() << "Can't load image: " << m_filename << " for printing.\n";
        return false;
        }

    QPainter p;
    p.begin( &m_printer );

    QPaintDeviceMetrics metrics( &m_printer );
    p.setFont( KGlobalSettings::generalFont() );
    QFontMetrics fm = p.fontMetrics();

    int w = metrics.width();
    int h = metrics.height();

    kdDebug() << "Printer Resolution: " << m_printer.resolution() << endl;
    kdDebug() << "Width: " << w << ", Height: " << h
              << ", Image Width: " << image.width()
              << ", Image Height: " << image.height() << endl;
    
    QString t = "true";
    QString f = "false";

    // Black & white print ?
    
    if ( m_printer.option( "app-imageeditor-blackwhite" ) != f) 
        image = image.convertDepth( 1, Qt::MonoOnly | Qt::ThresholdDither | Qt::AvoidDither );

    int filenameOffset = 0;
    bool printFilename = m_printer.option( "app-imageeditor-printFilename" ) != f;
    
    if ( printFilename ) 
        {
        filenameOffset = fm.lineSpacing() + 14;
        h -= filenameOffset; // filename goes into one line!
        }

    // Shrink image to pagesize, if necessary.
    
    bool shrinkToFit = (m_printer.option( "app-imageeditor-shrinkToFit" ) != f);
    
    if ( shrinkToFit && (image.width() > w || image.height() > h) )
        image = image.smoothScale( w, h, QImage::ScaleMin );

    // Align image.
    
    bool ok = false;
    int alignment = m_printer.option("app-imageeditor-alignment").toInt( &ok );
    
    if ( !ok )
        alignment = Qt::AlignCenter; // default

    int x = 0;
    int y = 0;

    // ### need a GUI for this in ImagePrintDialogPage!
    // x - alignment

    kdDebug() << "Width: " << w << ", Height: " << h
              << ", Rescaled Image Width: " << image.width()
              << ", Rescaled Image Height: " << image.height() << endl;
    
    if ( alignment & Qt::AlignHCenter )
        x = (w - image.width())/2;
    else if ( alignment & Qt::AlignLeft )
        x = 0;
    else if ( alignment & Qt::AlignRight )
        x = w - image.width();

    // y - alignment
    if ( alignment & Qt::AlignVCenter )
        y = (h - image.height())/2;
    else if ( alignment & Qt::AlignTop )
        y = 0;
    else if ( alignment & Qt::AlignBottom )
        y = h - image.height();

    // Perform the actual drawing.

    kdDebug() << "Printing at x, y : "
              << x << ", " << y << endl;
    
    p.drawImage( x, y, image );

    if ( printFilename )
        {
        QString fname = minimizeString( m_originalFileName, fm, w );
        
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


/////////////////////////////////////////////////////////////////////////////////////////////

QString ImagePrint::minimizeString( QString text, const QFontMetrics& metrics,
                                    int maxWidth )
{
    if ( text.length() <= 5 )
        return QString::null; // no sense to cut that tiny little string

    bool changed = false;
    
    while ( metrics.width( text ) > maxWidth )
        {
        int mid = text.length() / 2;
        text.remove( mid, 2 ); // remove 2 characters in the middle
        changed = true;
        }

    if ( changed ) // add "..." in the middle
        {
        int mid = text.length() / 2;
        
        if ( mid <= 5 ) // sanity check
            return QString::null;

        text.replace( mid - 1, 3, "..." );
        }

    return text;
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageEditorPrintDialogPage::getOptions( QMap<QString,QString>& opts,
                                           bool /*incldef*/ )
{
    QString t = "true";
    QString f = "false";

//    ### opts["app-imageeditor-alignment"] = ;
    opts["app-imageeditor-printFilename"] = m_addFileName->isChecked() ? t : f;
    opts["app-imageeditor-blackwhite"] = m_blackwhite->isChecked() ? t : f;
    opts["app-imageeditor-shrinkToFit"] = m_shrinkToFit->isChecked() ? t : f;
    opts["app-imageeditor-scale"] = m_scale->isChecked() ? t : f;
    opts["app-imageeditor-scale-unit"] = m_units->currentText();
    opts["app-imageeditor-scale-width-pixels"] = QString::number( scaleWidth() );
    opts["app-imageeditor-scale-height-pixels"] = QString::number( scaleHeight() );
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageEditorPrintDialogPage::setOptions( const QMap<QString,QString>& opts )
{
    QString t = "true";
    QString f = "false";

    m_addFileName->setChecked( opts["app-imageeditor-printFilename"] != f );
    // This sound strange, but if I copy the code on the line above, the checkbox
    // was always checked. And this isn't the wanted behavior. So, with this works.
    // KPrint magic ;-)
    m_blackwhite->setChecked ( false );
    m_shrinkToFit->setChecked( opts["app-imageeditor-shrinkToFit"] != f );
    m_scale->setChecked( opts["app-imageeditor-scale"] == t );

    m_units->setCurrentItem( opts["app-imageeditor-scale-unit"] );

    bool ok;
    int val = opts["app-imageeditor-scale-width-pixels"].toInt( &ok );
    
    if ( ok )
        setScaleWidth( val );
    
    val = opts["app-imageeditor-scale-height-pixels"].toInt( &ok );
    
    if ( ok )
        setScaleHeight( val );

    if ( m_scale->isChecked() == m_shrinkToFit->isChecked() )
        m_shrinkToFit->setChecked( !m_scale->isChecked() );

    // ### re-enable when implemented
    toggleScaling( false && m_scale->isChecked() );
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageEditorPrintDialogPage::toggleScaling( bool enable )
{
    m_width->setEnabled( enable );
    m_height->setEnabled( enable );
    m_units->setEnabled( enable );
}


/////////////////////////////////////////////////////////////////////////////////////////////

int ImageEditorPrintDialogPage::scaleWidth() const
{
    return fromUnitToPixels( m_width->value() );
}


/////////////////////////////////////////////////////////////////////////////////////////////

int ImageEditorPrintDialogPage::scaleHeight() const
{
    return fromUnitToPixels( m_height->value() );
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageEditorPrintDialogPage::setScaleWidth( int pixels )
{
    m_width->setValue( (int) pixelsToUnit( pixels ) );
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageEditorPrintDialogPage::setScaleHeight( int pixels )
{
    m_width->setValue( (int) pixelsToUnit( pixels ) );
}


/////////////////////////////////////////////////////////////////////////////////////////////

int ImageEditorPrintDialogPage::fromUnitToPixels( float /*value*/ ) const
{
    return 1; // ###
}


/////////////////////////////////////////////////////////////////////////////////////////////

float ImageEditorPrintDialogPage::pixelsToUnit( int /*pixels*/ ) const
{
    return 1.0; // ###
}


#include "imageprint.moc"
