/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-13
 * Description :
 *
 * Copyright 2004-2005 by Gilles Caulier
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
#include <qstyle.h>

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

namespace Digikam
{

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

    m_autoRotate = new QCheckBox( i18n("&Auto-rotate page"), this );
    m_autoRotate->setChecked( false );
    layout->addWidget( m_autoRotate );

    QVButtonGroup *group = new QVButtonGroup( i18n("Scaling"), this );
    group->setRadioButtonExclusive( true );
    layout->addWidget( group );

    m_scaleToFit = new QRadioButton( i18n("Scale image to &fit"), group );
    m_scaleToFit->setChecked( true );

    m_scale = new QRadioButton( i18n("Print e&xact size: "), group );

    connect( m_scale, SIGNAL( toggled( bool )),
             this, SLOT( toggleScaling( bool )));

    QHBox *hb = new QHBox( group );
    layout->addWidget( hb );
    hb->setSpacing( KDialog::spacingHint() );
    QWidget *w = new QWidget(hb);
    w->setFixedWidth(m_scale->style().subRect( QStyle::SR_RadioButtonIndicator, m_scale ).width());

    m_width = new KDoubleNumInput( hb, "exact width" );
    m_width->setMinValue( 1 );

    new QLabel( "x", hb );

    m_height = new KDoubleNumInput( hb, "exact height" );
    m_height->setMinValue( 1 );

    m_units = new KComboBox( false, hb, "unit combobox" );
    m_units->insertItem( i18n("Centimeters") );
    m_units->insertItem( i18n("Inches") );

    w = new QWidget(hb);
    hb->setStretchFactor( w, 1 );
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
    opts["app-imageeditor-auto-rotate"] = m_autoRotate->isChecked() ? t : f;
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
    m_autoRotate->setChecked( opts["app-imageeditor-auto-rotate"] == t );

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

ImagePrint::ImagePrint(DImg& image, KPrinter& printer, const QString& filename)
          : m_image( image ), m_printer( printer ), m_filename( filename )
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

    // TODO : perform all prepare to print transformations using DImg methods.
    // Paco, we will need to apply printer ICC profile here !

    QImage image2Print = m_image.copyQImage();

    // Black & white print ?
    if ( m_printer.option( "app-imageeditor-blackwhite" ) != f)
    {
        image2Print = image2Print.convertDepth( 1, Qt::MonoOnly |
                                                Qt::ThresholdDither |
                                                Qt::AvoidDither );
    }

    QPainter p;
    p.begin( &m_printer );

    QPaintDeviceMetrics metrics( &m_printer );
    p.setFont( KGlobalSettings::generalFont() );
    QFontMetrics fm = p.fontMetrics();

    int w, h; // will be set to the width and height of the printer
              // when the orientation is decided.
    int filenameOffset = 0;

    QSize size = image2Print.size();

    bool printFilename = m_printer.option( "app-imageeditor-printFilename" ) != f;
    if ( printFilename )
    {
        // filename goes into one line!
        filenameOffset = fm.lineSpacing() + 14;
        h -= filenameOffset;
    }

    if ( m_printer.option( "app-imageeditor-scaleToFit" ) != f )
    {
        if ( m_printer.option( "app-imageeditor-auto-rotate" ) == t )
            m_printer.setOrientation( size.width() <= size.height() ? KPrinter::Portrait : KPrinter::Landscape );

        // Scale image to fit pagesize
        w = metrics.width();
        h = metrics.height();
        size.scale( w, h, QSize::ScaleMin );
    }
    else
    {
        // scale image to exact dimensions
        QString unit  = m_printer.option("app-imageeditor-scale-unit");
        double  wunit = m_printer.option("app-imageeditor-scale-width").toDouble();
        double  hunit = m_printer.option("app-imageeditor-scale-height").toDouble();

        if ( m_printer.option( "app-imageeditor-auto-rotate" ) == t )
            m_printer.setOrientation( wunit <= hunit ? KPrinter::Portrait : KPrinter::Landscape );

        w = metrics.width();
        h = metrics.height();

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

        size.scale( wresize, hresize, QSize::ScaleMin );
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
        x = (w - size.width())/2;
    else if ( alignment & Qt::AlignLeft )
        x = 0;
    else if ( alignment & Qt::AlignRight )
        x = w - size.width();

    // y - alignment
    if ( alignment & Qt::AlignVCenter )
        y = (h - size.height())/2;
    else if ( alignment & Qt::AlignTop )
        y = 0;
    else if ( alignment & Qt::AlignBottom )
        y = h - size.height();

    // Perform the actual drawing.
    p.drawImage( QRect( x, y, size.width(), size.height()), image2Print );

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

}  // namespace Digikam

#include "imageprint.moc"
