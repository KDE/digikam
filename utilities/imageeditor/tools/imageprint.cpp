/* ============================================================
 * Authors: F.J. Cruz <fj.cruz@supercable.es>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2004-07-13
 * Description : image editor printing interface.
 *
 * Copyright 2004-2007 by Gilles Caulier
 * Copyright 2006 by F.J. Cruz
 *
 * KeepRatio and Alignment options imported from Gwenview program.
 * Copyright (c) 2003 Angelo Naselli
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
#include <kglobalsettings.h>
#include <knuminput.h>
#include <kprinter.h>
#include <ktempfile.h>
#include <kpropertiesdialog.h>

// Local includes

#include "ddebug.h"
#include "dimg.h"
#include "editorwindow.h"
#include "icctransform.h"
#include "imageprint.h"
#include "imageprint.moc"

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
        DWarning() << "Supplied Image for printing is null" << endl;
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
    w = metrics.width();
    h = metrics.height();
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
        size.scale( w, h, QSize::ScaleMin );
    }
    else
    {
        int unit = (m_printer.option("app-imageeditor-scale-unit").isEmpty() ?
            ImageEditorPrintDialogPage::DK_INCHES : m_printer.option("app-imageeditor-scale-unit").toInt());
        double inches = 1;
    
        if (unit == ImageEditorPrintDialogPage::DK_MILLIMETERS) 
        {
            inches = 1/25.4;
        } 
        else if (unit == ImageEditorPrintDialogPage::DK_CENTIMETERS) 
        {
            inches = 1/2.54;
        }
    
        double wImg = (m_printer.option("app-imageeditor-scale-width").isEmpty() ?
                1 : m_printer.option("app-imageeditor-scale-width").toDouble()) * inches;
        double hImg = (m_printer.option("app-imageeditor-scale-height").isEmpty() ?
                1 : m_printer.option("app-imageeditor-scale-height").toDouble()) * inches;
        size.setWidth( int(wImg * m_printer.resolution()) );
        size.setHeight( int(hImg * m_printer.resolution()) );
    
        if ( m_printer.option( "app-imageeditor-auto-rotate" ) == t )
            m_printer.setOrientation( wImg <= hImg ? KPrinter::Portrait : KPrinter::Landscape );
    
        if (size.width() > w || size.height() > h) 
        {
            int resp = KMessageBox::warningYesNoCancel(KApplication::kApplication()->mainWidget(),
                i18n("The image will not fit on the page, what do you want to do?"),
                QString::null,KStdGuiItem::cont(),
                i18n("Shrink") );
    
            if (resp==KMessageBox::Cancel) 
            {
                m_printer.abort();
                // no need to return false, user decided to abort
                return true;
            } 
            else if (resp == KMessageBox::No) 
            { // Shrink
                size.scale(w, h, QSize::ScaleMin);
            }
        }
    }

    // Align image.
    int alignment = (m_printer.option("app-imageeditor-alignment").isEmpty() ?
        Qt::AlignCenter : m_printer.option("app-imageeditor-alignment").toInt());

    int x = 0;
    int y = 0;

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

    d->inProfilePath     = config->readPathEntry("WorkSpaceProfile");
    d->outputProfilePath = config->readPathEntry("ProofProfileFile");
}

// Image print dialog class -------------------------------------------------------------

class ImageEditorPrintDialogPagePrivate
{

public:

    ImageEditorPrintDialogPagePrivate()
    {
        cmEnabled     = false;
        position      = 0;
        keepRatio     = 0;
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

    bool                               cmEnabled;

    QRadioButton                      *scaleToFit;
    QRadioButton                      *scale;

    QCheckBox                         *keepRatio;
    QCheckBox                         *addFileName;
    QCheckBox                         *blackwhite;
    QCheckBox                         *autoRotate;
    QCheckBox                         *colorManaged;

    QPushButton                       *cmPreferences;

    QWidget                           *parent;

    KDoubleNumInput                   *width;
    KDoubleNumInput                   *height;

    KComboBox                         *position;
    KComboBox                         *units;

    DImg                               image;

    ImageEditorPrintDialogPage::Unit  previousUnit;
};

ImageEditorPrintDialogPage::ImageEditorPrintDialogPage(DImg& image, QWidget *parent, const char *name )
                          : KPrintDialogPage( parent, name )
{
    d = new ImageEditorPrintDialogPagePrivate;
    d->image  = image;
    d->parent = parent;
    setTitle( i18n("Image Settings") );

    readSettings();

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( KDialog::marginHint() );
    layout->setSpacing( KDialog::spacingHint() );

    // ------------------------------------------------------------------------

    QHBoxLayout *layout2 = new QHBoxLayout( layout ); 
    layout2->setSpacing(3);

    QLabel* textLabel = new QLabel( this, "Image position:" );
    textLabel->setText( i18n( "Image position:" ) );
    layout2->addWidget( textLabel );
    d->position = new KComboBox( false, this, "Print position" );
    d->position->clear();
    d->position->insertItem( i18n( "Top-Left" ) );
    d->position->insertItem( i18n( "Top-Central" ) );
    d->position->insertItem( i18n( "Top-Right" ) );
    d->position->insertItem( i18n( "Central-Left" ) );
    d->position->insertItem( i18n( "Central" ) );
    d->position->insertItem( i18n( "Central-Right" ) );
    d->position->insertItem( i18n( "Bottom-Left" ) );
    d->position->insertItem( i18n( "Bottom-Central" ) );
    d->position->insertItem( i18n( "Bottom-Right" ) );
    layout2->addWidget( d->position );
    QSpacerItem *spacer1 = new QSpacerItem( 101, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( spacer1 );

    d->addFileName = new QCheckBox( i18n("Print fi&lename below image"), this);
    d->addFileName->setChecked( false );
    layout->addWidget( d->addFileName );

    d->blackwhite = new QCheckBox ( i18n("Print image in &black and white"), this);
    d->blackwhite->setChecked( false );
    layout->addWidget (d->blackwhite );

    d->autoRotate = new QCheckBox( i18n("&Auto-rotate page"), this );
    d->autoRotate->setChecked( false );
    layout->addWidget( d->autoRotate );

    // ------------------------------------------------------------------------

    QHBox *cmbox = new QHBox(this);

    d->colorManaged = new QCheckBox(i18n("Use Color Management for Printing"), cmbox);
    d->colorManaged->setChecked( false );

    d->cmPreferences = new QPushButton(i18n("Settings..."), cmbox);

    QWidget *space = new QWidget(cmbox);
    cmbox->setStretchFactor(space, 10);
    cmbox->setSpacing(KDialog::spacingHint());

    layout->addWidget(cmbox);
   
    // ------------------------------------------------------------------------

    QVButtonGroup *group = new QVButtonGroup( i18n("Scaling"), this );
    group->setRadioButtonExclusive( true );
    layout->addWidget( group );

    d->scaleToFit = new QRadioButton( i18n("Scale image to &fit"), group );
    d->scaleToFit->setChecked( true );

    d->scale = new QRadioButton( i18n("Print e&xact size: "), group );

    QHBox *hb = new QHBox( group );
    hb->setSpacing( KDialog::spacingHint() );
    QWidget *w = new QWidget(hb);
    w->setFixedWidth(d->scale->style().subRect( QStyle::SR_RadioButtonIndicator, d->scale ).width());

    d->width = new KDoubleNumInput( hb, "exact width" );
    d->width->setMinValue( 1 );

    new QLabel( "x", hb );

    d->height = new KDoubleNumInput( hb, "exact height" );
    d->height->setMinValue( 1 );

    d->units = new KComboBox( false, hb, "unit combobox" );
    d->units->insertItem( i18n("Millimeters") );
    d->units->insertItem( i18n("Centimeters") );
    d->units->insertItem( i18n("Inches") );

    d->keepRatio = new QCheckBox( i18n("Keep ratio"), hb);

    w = new QWidget(hb);
    hb->setStretchFactor( w, 1 );
    d->previousUnit = DK_MILLIMETERS;

    // ------------------------------------------------------------------------

    connect( d->colorManaged, SIGNAL(toggled(bool)),
             this, SLOT(slotAlertSettings( bool )) );

    connect( d->cmPreferences, SIGNAL(clicked()),
             this, SLOT(slotSetupDlg()) );

    connect( d->scale, SIGNAL( toggled( bool )),
             this, SLOT( toggleScaling( bool )));

    connect(d->width, SIGNAL( valueChanged( double )), 
            this, SLOT( slotWidthChanged( double )));

    connect(d->height, SIGNAL( valueChanged( double )), 
            this, SLOT( slotHeightChanged( double )));

    connect(d->keepRatio, SIGNAL( toggled( bool )), 
            this, SLOT( toggleRatio( bool )));

    connect(d->units, SIGNAL(activated(const QString &)), 
            this, SLOT(slotUnitChanged(const QString& )));
}

ImageEditorPrintDialogPage::~ImageEditorPrintDialogPage()
{
    delete d;
}

void ImageEditorPrintDialogPage::getOptions( QMap<QString,QString>& opts, bool /*incldef*/ )
{
    QString t = "true";
    QString f = "false";

    opts["app-imageeditor-alignment"]       = QString::number(getPosition(d->position->currentText()));
    opts["app-imageeditor-printFilename"]   = d->addFileName->isChecked() ? t : f;
    opts["app-imageeditor-blackwhite"]      = d->blackwhite->isChecked() ? t : f;
    opts["app-imageeditor-scaleToFit"]      = d->scaleToFit->isChecked() ? t : f;
    opts["app-imageeditor-scale"]           = d->scale->isChecked() ? t : f;
    opts["app-imageeditor-scale-unit"]      = QString::number(stringToUnit(d->units->currentText()));
    opts["app-imageeditor-scale-width"]     = QString::number( d->width->value() );
    opts["app-imageeditor-scale-height"]    = QString::number( d->height->value() );
    opts["app-imageeditor-scale-KeepRatio"] = d->keepRatio->isChecked() ? t : f;
    opts["app-imageeditor-auto-rotate"]     = d->autoRotate->isChecked() ? t : f;
    opts["app-imageeditor-color-managed"]   = d->colorManaged->isChecked() ? t : f;
}

void ImageEditorPrintDialogPage::setOptions( const QMap<QString,QString>& opts )
{
    QString t = "true";
    QString f = "false";
    QString stVal;
    bool    ok;
    double  dVal;
    int     iVal;

    iVal = opts["app-imageeditor-alignment"].toInt( &ok );
    if (ok) 
    {
        stVal = setPosition(iVal);
        d->position->setCurrentItem(stVal);
    }

    d->addFileName->setChecked( opts["app-imageeditor-printFilename"] != f );
    // This sound strange, but if I copy the code on the line above, the checkbox
    // was always checked. And this isn't the wanted behavior. So, with this works.
    // KPrint magic ;-)
    d->blackwhite->setChecked ( false );
    d->scaleToFit->setChecked( opts["app-imageeditor-scaleToFit"] != f );
    d->scale->setChecked( opts["app-imageeditor-scale"] == t );
    d->autoRotate->setChecked( opts["app-imageeditor-auto-rotate"] == t );

    d->colorManaged->setChecked( false );

    Unit unit = static_cast<Unit>( opts["app-imageeditor-scale-unit"].toInt( &ok ) );
    if (ok) 
    {
        stVal = unitToString(unit);
        d->units->setCurrentItem(stVal);
        d->previousUnit = unit;
    }
    else
    {
        //for back compatibility
        d->units->setCurrentItem(i18n("Millimeters"));
    }

    dVal = opts["app-imageeditor-scale-width"].toDouble( &ok );

    if ( ok )
      d->width->setValue( dVal );

    dVal = opts["app-imageeditor-scale-height"].toDouble( &ok );

    if ( ok )
      d->height->setValue( dVal );

    if ( d->scale->isChecked() == d->scaleToFit->isChecked() )
        d->scaleToFit->setChecked( !d->scale->isChecked() );

    d->keepRatio->setChecked( opts["app-imageeditor-scale-KeepRatio"] == t );

}
int ImageEditorPrintDialogPage::getPosition(const QString& align) 
{
    int alignment;
    
    if (align == i18n("Central-Left")) 
    {
        alignment = Qt::AlignLeft | Qt::AlignVCenter;
    } 
    else if (align == i18n("Central-Right")) 
    {
        alignment = Qt::AlignRight | Qt::AlignVCenter;
    } 
    else if (align == i18n("Top-Left")) 
    {
        alignment = Qt::AlignTop | Qt::AlignLeft;
    } 
    else if (align == i18n("Top-Right")) 
    {
        alignment = Qt::AlignTop | Qt::AlignRight;
    } 
    else if (align == i18n("Bottom-Left")) 
    {
        alignment = Qt::AlignBottom | Qt::AlignLeft;
    }  
    else if (align == i18n("Bottom-Right")) 
    {
        alignment = Qt::AlignBottom | Qt::AlignRight;
    } 
    else if (align == i18n("Top-Central")) 
    {
        alignment = Qt::AlignTop | Qt::AlignHCenter;
    } 
    else if (align == i18n("Bottom-Central")) 
    {
        alignment = Qt::AlignBottom | Qt::AlignHCenter;
    }  
    else  
    {
        // Central
        alignment = Qt::AlignCenter; // Qt::AlignHCenter || Qt::AlignVCenter
    }
    
    return alignment;
}

QString ImageEditorPrintDialogPage::setPosition(int align) 
{
    QString alignment;
    
    if (align == (Qt::AlignLeft | Qt::AlignVCenter)) 
    {
        alignment = i18n("Central-Left");
    } 
    else if (align == (Qt::AlignRight | Qt::AlignVCenter)) 
    {
        alignment = i18n("Central-Right");
    } 
    else if (align == (Qt::AlignTop | Qt::AlignLeft)) 
    {
        alignment = i18n("Top-Left");
    } 
    else if (align == (Qt::AlignTop | Qt::AlignRight)) 
    {
        alignment = i18n("Top-Right");
    } 
    else if (align == (Qt::AlignBottom | Qt::AlignLeft)) 
    {
        alignment = i18n("Bottom-Left");
    } 
    else if (align == (Qt::AlignBottom | Qt::AlignRight)) 
    {
        alignment = i18n("Bottom-Right");
    } 
    else if (align == (Qt::AlignTop | Qt::AlignHCenter)) 
    {
        alignment = i18n("Top-Central");
    } 
    else if (align == (Qt::AlignBottom | Qt::AlignHCenter)) 
    {
        alignment = i18n("Bottom-Central");
    } 
    else  
    {
        // Central: Qt::AlignCenter or (Qt::AlignHCenter || Qt::AlignVCenter)
        alignment = i18n("Central");
    }
    
    return alignment;
}

void ImageEditorPrintDialogPage::toggleScaling( bool enable )
{
    d->width->setEnabled( enable );
    d->height->setEnabled( enable );
    d->units->setEnabled( enable );
    d->keepRatio->setEnabled( enable );
}

void ImageEditorPrintDialogPage::slotHeightChanged (double value) 
{
    d->width->blockSignals(true);
    d->height->blockSignals(true);
    
    if (d->keepRatio->isChecked()) 
    {
        double width = (d->image.width() * value) / d->image.height();
        d->width->setValue( width ? width : 1.);
    }
    d->height->setValue(value);
    
    d->width->blockSignals(false);
    d->height->blockSignals(false);
}

void ImageEditorPrintDialogPage::slotWidthChanged (double value) 
{
    d->width->blockSignals(true);
    d->height->blockSignals(true);
    
    if (d->keepRatio->isChecked()) 
    {
        double height = (d->image.height() * value) / d->image.width();
        d->height->setValue( height ? height : 1);
    }
    d->width->setValue(value);
    
    d->width->blockSignals(false);
    d->height->blockSignals(false);
}

void ImageEditorPrintDialogPage::toggleRatio( bool enable )
{
    if (!enable) return;
    // choosing a startup value of 15x10 cm (common photo dimention)
    // mContent->mHeight->value() or mContent->mWidth->value()
    // are usually empty at startup and hxw (0x0) isn't good IMO keeping ratio
    double hValue, wValue;
    if (d->image.height() > d->image.width()) 
    {
        hValue = d->height->value();
        if (!hValue) hValue = 150*unitToMM(d->previousUnit);
        wValue = (d->image.width() * hValue)/ d->image.height();
    } 
    else 
    {
        wValue = d->width->value();
        if (!wValue) wValue = 150*unitToMM(d->previousUnit);
        hValue = (d->image.height() * wValue)/ d->image.width();
    }
    
    d->width->blockSignals(true);
    d->height->blockSignals(true);
    
    d->width->setValue(wValue);
    d->height->setValue(hValue);
    
    d->width->blockSignals(false);
    d->height->blockSignals(false);
}

void ImageEditorPrintDialogPage::slotUnitChanged(const QString& string) 
{
    Unit newUnit = stringToUnit(string);
    double ratio = unitToMM(d->previousUnit) / unitToMM(newUnit);
    
    d->width->blockSignals(true);
    d->height->blockSignals(true);
    
    d->width->setValue( d->width->value() * ratio);
    d->height->setValue( d->height->value() * ratio);
    
    d->width->blockSignals(false);
    d->height->blockSignals(false);
    
    d->previousUnit = newUnit;
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

double ImageEditorPrintDialogPage::unitToMM(Unit unit) 
{
    if (unit == DK_MILLIMETERS) 
    {
      return 1.;
    } 
    else if (unit == DK_CENTIMETERS) 
    {
      return 10.;
    } 
    else 
    { //DK_INCHES
      return 25.4;
    }
}

ImageEditorPrintDialogPage::Unit ImageEditorPrintDialogPage::stringToUnit(const QString& unit) 
{
    if (unit == i18n("Millimeters")) 
    {
      return DK_MILLIMETERS;
    }
    else if (unit == i18n("Centimeters")) 
    {
      return DK_CENTIMETERS;
    }
    else 
    {//Inches
      return DK_INCHES;
    }
}
  
QString ImageEditorPrintDialogPage::unitToString(Unit unit) 
{
    if (unit == DK_MILLIMETERS) 
    {
      return i18n("Millimeters");
    }
    else if (unit == DK_CENTIMETERS) 
    {
      return i18n("Centimeters");
    } 
    else 
    { //DK_INCHES
      return i18n("Inches");
    }
}

}  // namespace Digikam

