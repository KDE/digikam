/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2004-11-17
 * Description : A tab to display Exif image informations
 *
 * Copyright 2004-2006 by Gilles Caulier
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

#define PNG_BYTES_TO_CHECK 4

// C Ansi includes.

extern "C"
{
#include <unistd.h>
}

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cassert>

// Qt includes.
 
#include <qlayout.h>
#include <qfile.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qfileinfo.h>
#include <qcombobox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kfileitem.h>
#include <kdebug.h>

// LibKExif includes.

#include <libkexif/kexifwidget.h>

// LibExiv2 includes.

#include "exiv2/image.hpp"
#include "exiv2/exif.hpp"

// Local includes.

#include "navigatebarwidget.h"
#include "imagepropertiesexiftab.h"

namespace Digikam
{

class ImagePropertiesEXIFTabPriv
{
public:

    ImagePropertiesEXIFTabPriv()
    {
        levelCombo  = 0;
        exifWidget  = 0;
        navigateBar = 0;
    }

    QComboBox         *levelCombo;

    QString            currentItem;
    
    KExifWidget       *exifWidget;
    
    NavigateBarWidget *navigateBar;
};

ImagePropertiesEXIFTab::ImagePropertiesEXIFTab(QWidget* parent, bool navBar)
                      : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImagePropertiesEXIFTabPriv;
    QGridLayout *topLayout = new QGridLayout(this, 2, 2, KDialog::marginHint(), KDialog::spacingHint());

    d->navigateBar  = new NavigateBarWidget(this, navBar);
    topLayout->addMultiCellWidget(d->navigateBar, 0, 0, 0, 2);
        
    QLabel* levelLabel = new QLabel(i18n("Level of detail:"), this);
    d->levelCombo      = new QComboBox(this);
    topLayout->addMultiCellWidget(levelLabel, 1, 1, 0, 1);
    topLayout->addMultiCellWidget(d->levelCombo, 1, 1, 2, 2);

    QWhatsThis::add( d->levelCombo, i18n("<p>Select here the Exif information level to display<p>"
                                        "<b>Simple</b>: display general information about the photograph "
                                        " (default).<p>"
                                        "<b>Full</b>: display all EXIF sections.") );  
    
    d->exifWidget = new KExifWidget(this);
    topLayout->addMultiCellWidget(d->exifWidget, 2, 2, 0, 2);

    d->levelCombo->insertItem(i18n("Simple"));
    d->levelCombo->insertItem(i18n("Full"));
    
    connect(d->levelCombo, SIGNAL(activated(int)),
            this, SLOT(slotLevelChanged(int)));
    
    connect(d->navigateBar, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(d->navigateBar, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->navigateBar, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->navigateBar, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));

    // -- read config ---------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    d->levelCombo->setCurrentItem(config->readNumEntry("EXIF Level", 0));
    d->currentItem = config->readEntry("Current EXIF Item", QString());
    slotLevelChanged(0);
}

ImagePropertiesEXIFTab::~ImagePropertiesEXIFTab()
{
    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    config->writeEntry("EXIF Level", d->levelCombo->currentItem());
    config->writeEntry("Current EXIF Item", d->currentItem);
    delete d;
}

void ImagePropertiesEXIFTab::setCurrentURL(const KURL& url, int itemType)
{
    if (url.isEmpty())
    {
       d->exifWidget->loadFile(url.path());
       d->navigateBar->setFileName();
       setEnabled(false);
       return;
    }

    setEnabled(true);

    if (!d->exifWidget->getCurrentItemName().isNull())
        d->currentItem = d->exifWidget->getCurrentItemName();
    
    // In first, trying to load Raw exif data embedded in PNG file.
    
    QByteArray ba = loadRawExifProfileFromPNG(url.path());
    if (!ba.isNull())
    {
        setCurrentData(ba, url.filename(), itemType);
        d->exifWidget->setCurrentItem(d->currentItem);
        d->navigateBar->setFileName(url.filename());
        d->navigateBar->setButtonsState(itemType);
        return;
    }
    
    // In second, trying to load exif data using Exiv2 library.
    ba = loadExifWithExiv2(url.path());
    if (!ba.isNull())
    {
        setCurrentData(ba, url.filename(), itemType);
        d->exifWidget->setCurrentItem(d->currentItem);
        d->navigateBar->setFileName(url.filename());
        d->navigateBar->setButtonsState(itemType);
        return;
    }
        
    // In third, we trying to load exif data using likexif from THM file (thumbnail) if exist,
    // especially provided by recent USM camera.
    // At end, trying to get Exif data from real image.

    QFileInfo fi(url.path());
    QFileInfo thmLo(fi.dirPath() + "/" + fi.baseName() + ".thm");          // Lowercase
    QFileInfo thmUp(fi.dirPath() + "/" + fi.baseName() + ".THM");          // Uppercase

    if (thmLo.exists())
        d->exifWidget->loadFile(thmLo.filePath());
    else if (thmUp.exists())
        d->exifWidget->loadFile(thmUp.filePath());
    else
        d->exifWidget->loadFile(url.path());
   
    d->exifWidget->setCurrentItem(d->currentItem);
    d->navigateBar->setFileName(url.filename());
    d->navigateBar->setButtonsState(itemType);
}

void ImagePropertiesEXIFTab::setCurrentData(const QByteArray& data, const QString& filename, int itemType)
{
    if (data.isEmpty())
    {
       d->exifWidget->loadData(data.data(), data.size());
       d->navigateBar->setFileName();
       setEnabled(false);
       return;
    }

    setEnabled(true);

    if (!d->exifWidget->getCurrentItemName().isNull())
        d->currentItem = d->exifWidget->getCurrentItemName();
    
    d->exifWidget->loadData(data.data(), data.size());
    d->exifWidget->setCurrentItem(d->currentItem);
    
    d->navigateBar->setFileName(filename);
    d->navigateBar->setButtonsState(itemType);
}

void ImagePropertiesEXIFTab::slotLevelChanged(int)
{
    if (d->levelCombo->currentText() == i18n("Simple"))
        d->exifWidget->setMode(KExifWidget::SIMPLE);
    else
        d->exifWidget->setMode(KExifWidget::FULL);
}

QByteArray ImagePropertiesEXIFTab::loadRawExifProfileFromPNG(const KURL& url)
{
    QByteArray ba;
    png_uint_32  w32, h32;
    FILE        *f;
    int          bit_depth, color_type, interlace_type;
    png_structp  png_ptr  = NULL;
    png_infop    info_ptr = NULL;
    
    // -------------------------------------------------------------------
    // Open the file
    
    f = fopen(QFile::encodeName(url.path()), "rb");
    if ( !f )
        return ba;

    unsigned char buf[PNG_BYTES_TO_CHECK];

    fread(buf, 1, PNG_BYTES_TO_CHECK, f);
    if (!png_check_sig(buf, PNG_BYTES_TO_CHECK))
    {
        fclose(f);
        return ba;
    }
    rewind(f);

    // -------------------------------------------------------------------
    // Initialize the internal structures
    
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fclose(f);
        return ba;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(f);
        return ba;
    }

    if (setjmp(png_ptr->jmpbuf))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(f);
        return ba;
    }

    png_init_io(png_ptr, f);
    
    // -------------------------------------------------------------------
    // Read all PNG info up to image data
    
    png_read_info(png_ptr, info_ptr);
    
    png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) (&w32),
                 (png_uint_32 *) (&h32), &bit_depth, &color_type,
                 &interlace_type, NULL, NULL);

    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    // Check if we have a Raw profile embedded using ImageMagick technic.
    
    png_text* text_ptr;
    int num_comments = png_get_text(png_ptr, info_ptr, &text_ptr, NULL);

    for (int i = 0; i < num_comments; i++)
    {
        if (memcmp(text_ptr[i].key, "Raw profile type exif", 21) == 0)
        {
            png_uint_32 length;
            uchar *data = readRawProfile(text_ptr, &length, i);
            ba.resize(length);
            memcpy(ba.data(), data, length);
            delete [] data;
        }
    }
    
    // -------------------------------------------------------------------
    
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
    png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);

    fclose(f);
    return ba;
}

uchar* ImagePropertiesEXIFTab::readRawProfile(png_textp text, png_uint_32 *length, int ii)
{
    uchar          *info = 0;
    
    register long   i;
    
    register uchar *dp;
    
    register        png_charp sp;
    
    png_uint_32     nibbles;
    
    unsigned char unhex[103]={0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,1, 2,3,4,5,6,7,8,9,0,0,
                              0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,10,11,12,
                              13,14,15};
    
    sp = text[ii].text+1;
    
    // Look for newline 

    while (*sp != '\n')
        sp++;
    
    // Look for length 

    while (*sp == '\0' || *sp == ' ' || *sp == '\n')
        sp++;
    
    *length = (png_uint_32) atol(sp);
    
    while (*sp != ' ' && *sp != '\n')
        sp++;
    
    // Allocate space 
    
    if (*length == 0)
    {
        kdDebug() << "Unable To Copy Raw Profile: invalid profile length"  << endl;
        return (false);
    }
    
    info = new uchar[*length];
    
    if (!info)
    {
        kdDebug() << "Unable To Copy Raw Profile: cannot allocate memory"  << endl;
        return (false);
    }
    
    // Copy profile, skipping white space and column 1 "=" signs 

    dp      = info;
    nibbles = *length * 2;
    
    for (i = 0; i < (long) nibbles; i++)
    {
        while (*sp < '0' || (*sp > '9' && *sp < 'a') || *sp > 'f')
        {
            if (*sp == '\0')
            {
                kdDebug() << "Unable To Copy Raw Profile: ran out of data" << endl;
                return (false);
            }
            
            sp++;
        }
    
        if (i%2 == 0)
            *dp = (uchar) (16*unhex[(int) *sp++]);
        else
            (*dp++) += unhex[(int) *sp++];
    }
    
    return info;
}

QByteArray ImagePropertiesEXIFTab::loadExifWithExiv2(const KURL& url)
{
    try
    {    
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)(QFile::encodeName(url.path())));
        image->readMetadata();
        Exiv2::ExifData exifData = image->exifData();
    
        if (exifData.empty())
            return QByteArray();
    
        exifData.sortByKey();
        
        Exiv2::DataBuf const c(exifData.copy());
        const uchar ExifHeader[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
        QByteArray data(c.size_ + sizeof(ExifHeader));
        memcpy(data.data(), ExifHeader, sizeof(ExifHeader));
        memcpy(data.data() + sizeof(ExifHeader), c.pData_, c.size_);
        return (data);
    }
    catch( Exiv2::Error& e )
    {
        kdDebug() << "Caught Exiv2 exception (" << e.code() << ")" << endl;
        return QByteArray();
    }
}

}  // NameSpace Digikam

#include "imagepropertiesexiftab.moc"
