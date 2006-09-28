/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-07-13
 * Description : image editor printing interface.
 *
 * Copyright 2004-2006 by Gilles Caulier
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
#include <qpushbutton.h>

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

#include "dimg.h"
#include "editorwindow.h"
#include "icctransform.h"
#include "imageprint.h"

namespace Digikam
{

class ImagePrintPrivate
{

public:

    ImagePrintPrivate(){}

    QString filename;
    QString inProfilePath;
    QString outputProfilePath;

    DImg    image;
};

ImagePrint::ImagePrint(DImg& image, KPrinter& printer, const QString& filename)
          : m_printer(printer)
{
    d = new ImagePrintPrivate();
    d->image    = image;
    d->filename = filename;
}

ImagePrint::~ImagePrint()
{
    delete d;
}

bool ImagePrint::printImageWithQt()
{
    if ( d->image.isNull() )
    {
        kdWarning() << "Supplied Image for printing is null" << endl;
        return false;
    }

    QString t = "true";
    QString f = "false";

    if (m_printer.option( "app-imageeditor-color-managed") != f)
    {
        IccTransform *transform = new IccTransform();
        readSettings();

        if (d->image.getICCProfil().isNull())
        {
            transform->setProfiles( d->inProfilePath, d->outputProfilePath );
        }
        else
        {
            transform->setProfiles(d->outputProfilePath);
        }
        
        transform->apply( d->image );
    }
    
    QImage image2Print = d->image.copyQImage();

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
            m_printer.setOrientation( size.width() <= size.height() ? KPrinter::Portrait 
                                       : KPrinter::Landscape );

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
        QString fname = minimizeString( d->filename, fm, w );

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

void ImagePrint::readSettings()
{
     KConfig* config = kapp->config();

    config->setGroup("Color Management");

    d->inProfilePath = config->readPathEntry("InProfileFile");
    d->outputProfilePath = config->readPathEntry("ProofProfileFile");
}

// Image print dialog class -------------------------------------------------------------

class ImageEditorPrintDialogPagePrivate
{

public:

    ImageEditorPrintDialogPagePrivate()
    {
        cmEnabled     = false;
        scaleToFit    = 0;
        scale         = 0;
        addFileName   = 0;
        blackwhite    = 0;
        autoRotate    = 0;
        colorManaged  = 0;
        cmPreferences = 0;
        parent        = 0;
        width         = 0;
        height        = 0;
        units         = 0;
    }

    bool             cmEnabled;

    QRadioButton    *scaleToFit;
    QRadioButton    *scale;

    QCheckBox       *addFileName;
    QCheckBox       *blackwhite;
    QCheckBox       *autoRotate;
    QCheckBox       *colorManaged;

    QPushButton     *cmPreferences;

    QWidget         *parent;

    KDoubleNumInput *width;
    KDoubleNumInput *height;

    KComboBox       *units;
};

ImageEditorPrintDialogPage::ImageEditorPrintDialogPage( QWidget *parent, const char *name )
                          : KPrintDialogPage( parent, name )
{
    d = new ImageEditorPrintDialogPagePrivate;
    d->parent = parent;
    setTitle( i18n("Image Settings") );

    readSettings();

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( KDialog::marginHint() );
    layout->setSpacing( KDialog::spacingHint() );

    d->addFileName = new QCheckBox( i18n("Print fi&lename below image"), this);
    d->addFileName->setChecked( false );
    layout->addWidget( d->addFileName );

    d->blackwhite = new QCheckBox ( i18n("Print image in &black and white"), this);
    d->blackwhite->setChecked( false );
    layout->addWidget (d->blackwhite );

    d->autoRotate = new QCheckBox( i18n("&Auto-rotate page"), this );
    d->autoRotate->setChecked( false );
    layout->addWidget( d->autoRotate );

    QVButtonGroup *cmgroup = new QVButtonGroup(i18n("Color Management Settings"), this);
    layout->addWidget(cmgroup);

    QHBox *cmbox = new QHBox(cmgroup);
    layout->addWidget(cmbox);
    cmbox->setSpacing(KDialog::spacingHint());
    QWidget *wth = new QWidget(cmbox);

    d->colorManaged = new QCheckBox(i18n("Color Management"), cmbox);
    d->colorManaged->setChecked( false );

    d->cmPreferences = new QPushButton(i18n("Settings"), cmbox);

    wth->setFixedWidth(d->colorManaged->style().subRect( QStyle::SR_CheckBoxIndicator, 
                                                         d->colorManaged ).width());
    wth = new QWidget(cmbox);
    cmbox->setStretchFactor(wth, 1);

    connect( d->colorManaged, SIGNAL(toggled(bool)),
             this, SLOT(slotAlertSettings( bool )) );

    connect( d->cmPreferences, SIGNAL(clicked()),
             this, SLOT(slotSetupDlg()) );
    

    QVButtonGroup *group = new QVButtonGroup( i18n("Scaling"), this );
    group->setRadioButtonExclusive( true );
    layout->addWidget( group );

    d->scaleToFit = new QRadioButton( i18n("Scale image to &fit"), group );
    d->scaleToFit->setChecked( true );

    d->scale = new QRadioButton( i18n("Print e&xact size: "), group );

    connect( d->scale, SIGNAL( toggled( bool )),
             this, SLOT( toggleScaling( bool )));

    QHBox *hb = new QHBox( group );
    layout->addWidget( hb );
    hb->setSpacing( KDialog::spacingHint() );
    QWidget *w = new QWidget(hb);
    w->setFixedWidth(d->scale->style().subRect( QStyle::SR_RadioButtonIndicator, d->scale ).width());

    d->width = new KDoubleNumInput( hb, "exact width" );
    d->width->setMinValue( 1 );

    new QLabel( "x", hb );

    d->height = new KDoubleNumInput( hb, "exact height" );
    d->height->setMinValue( 1 );

    d->units = new KComboBox( false, hb, "unit combobox" );
    d->units->insertItem( i18n("Centimeters") );
    d->units->insertItem( i18n("Inches") );

    w = new QWidget(hb);
    hb->setStretchFactor( w, 1 );
}

ImageEditorPrintDialogPage::~ImageEditorPrintDialogPage()
{
    delete d;
}

void ImageEditorPrintDialogPage::getOptions( QMap<QString,QString>& opts,
                                             bool /*incldef*/ )
{
    QString t = "true";
    QString f = "false";

    opts["app-imageeditor-printFilename"] = d->addFileName->isChecked() ? t : f;
    opts["app-imageeditor-blackwhite"] = d->blackwhite->isChecked() ? t : f;
    opts["app-imageeditor-scaleToFit"] = d->scaleToFit->isChecked() ? t : f;
    opts["app-imageeditor-scale"] = d->scale->isChecked() ? t : f;
    opts["app-imageeditor-scale-unit"] = d->units->currentText();
    opts["app-imageeditor-scale-width"] = QString::number( d->width->value() );
    opts["app-imageeditor-scale-height"] = QString::number( d->height->value() );
    opts["app-imageeditor-auto-rotate"] = d->autoRotate->isChecked() ? t : f;
    opts["app-imageeditor-color-managed"] = d->colorManaged->isChecked() ? t : f;
}

void ImageEditorPrintDialogPage::setOptions( const QMap<QString,QString>& opts )
{
    QString t = "true";
    QString f = "false";

    d->addFileName->setChecked( opts["app-imageeditor-printFilename"] != f );
    // This sound strange, but if I copy the code on the line above, the checkbox
    // was always checked. And this isn't the wanted behavior. So, with this works.
    // KPrint magic ;-)
    d->blackwhite->setChecked ( false );
    d->scaleToFit->setChecked( opts["app-imageeditor-scaleToFit"] != f );
    d->scale->setChecked( opts["app-imageeditor-scale"] == t );
    d->autoRotate->setChecked( opts["app-imageeditor-auto-rotate"] == t );

    d->colorManaged->setChecked( false );

    d->units->setCurrentItem( opts["app-imageeditor-scale-unit"] );

    bool   ok;
    double val;

    val = opts["app-imageeditor-scale-width"].toDouble( &ok );

    if ( ok )
        d->width->setValue( val );

    val = opts["app-imageeditor-scale-height"].toDouble( &ok );

    if ( ok )
        d->height->setValue( val );

    if ( d->scale->isChecked() == d->scaleToFit->isChecked() )
        d->scaleToFit->setChecked( !d->scale->isChecked() );

    toggleScaling( d->scale->isChecked() );
}

void ImageEditorPrintDialogPage::toggleScaling( bool enable )
{
    d->width->setEnabled( enable );
    d->height->setEnabled( enable );
    d->units->setEnabled( enable );
}

void ImageEditorPrintDialogPage::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("Color Management");
    d->cmEnabled = config->readBoolEntry("EnableCM", false);
}

void ImageEditorPrintDialogPage::slotSetupDlg()
{
    EditorWindow* editor = dynamic_cast<EditorWindow*>(d->parent);
    editor->setup(true);
}

void ImageEditorPrintDialogPage::slotAlertSettings( bool t)
{
    if (t && !d->cmEnabled)
    {
        QString message = i18n("<p>Color Management is disabled.</p> \
                                <p>You can enable it now by clicking on the \"Settings\" button.</p>");
        KMessageBox::information(this, message);
        d->colorManaged->setChecked(!t);
    }
}

}  // namespace Digikam

#include "imageprint.moc"
