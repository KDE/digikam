//////////////////////////////////////////////////////////////////////////////
//
//    DIGIKAMTHUMBNAIL.CPP
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

#define XMD_H

// Qt Includes.

#include <qcstring.h>
#include <qstring.h>
#include <qimage.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qdir.h>
#include <qwmatrix.h>

// KDE includes.

#include <kdebug.h>
#include <kurl.h>
#include <kinstance.h>
#include <kio/global.h>
#include <kimageio.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmdcodec.h>
#include <ktempfile.h>

// C Ansi includes.

extern "C"
{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <jpeglib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <png.h>
}

// Local includes

#include "exiforientation_p.h"
#include "digikamthumbnail.h"

#define X_DISPLAY_MISSING 1
#include <Imlib2.h>

#include "digikam_export.h"

using namespace KIO;

extern "C"
{
int dcraw_identify(const char* infile, const char* outfile);
}

static void exifRotate(const QString& filePath, QImage& thumb)
{
    // Rotate thumbnail based on EXIF rotate tag
    QWMatrix matrix;

    KExifData::ImageOrientation orientation
        = getExifOrientation(filePath);

    bool doXform = (orientation != KExifData::NORMAL &&
                    orientation != KExifData::UNSPECIFIED);

    switch (orientation) {
       case KExifData::NORMAL:
       case KExifData::UNSPECIFIED:
          break;

       case KExifData::HFLIP:
          matrix.scale(-1,1);
          break;

       case KExifData::ROT_180:
          matrix.rotate(180);
          break;

       case KExifData::VFLIP:
          matrix.scale(1,-1);
          break;

       case KExifData::ROT_90_HFLIP:
          matrix.scale(-1,1);
          matrix.rotate(90);
          break;

       case KExifData::ROT_90:
          matrix.rotate(90);
          break;

       case KExifData::ROT_90_VFLIP:
          matrix.scale(1,-1);
          matrix.rotate(90);
          break;

       case KExifData::ROT_270:
          matrix.rotate(270);
          break;
    }

    //transform accordingly
    if ( doXform )
       thumb = thumb.xForm( matrix );
}

#define PNG_BYTES_TO_CHECK 4

static QImage loadPNG(const QString& path)
{
    png_uint_32         w32, h32;
    int                 w, h;
    bool                has_alpha;
    bool                has_grey;
    FILE               *f;
    png_structp         png_ptr = NULL;
    png_infop           info_ptr = NULL;
    int                 bit_depth, color_type, interlace_type;

    has_alpha = 0;
    has_grey = 0;

    QImage qimage;

    f = fopen(path.latin1(), "rb");
    if (!f)
        return qimage;

    unsigned char       buf[PNG_BYTES_TO_CHECK];

    fread(buf, 1, PNG_BYTES_TO_CHECK, f);
    if (!png_check_sig(buf, PNG_BYTES_TO_CHECK))
    {
        fclose(f);
        return qimage;
    }
    rewind(f);

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fclose(f);
        return qimage;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(f);
        return qimage;
    }

    if (setjmp(png_ptr->jmpbuf))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(f);
        return qimage;
    }

    png_init_io(png_ptr, f);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) (&w32),
                 (png_uint_32 *) (&h32), &bit_depth, &color_type,
                 &interlace_type, NULL, NULL);

    w = w32;
    h = h32;

    qimage.create(w, h, 32);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_expand(png_ptr);

    if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
        has_alpha = 1;

    if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        has_alpha = 1;
        has_grey = 1;
    }

    if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
        has_grey = 1;


    unsigned char     **lines;
    int                 i;

    if (has_alpha)
        png_set_expand(png_ptr);

    if (QImage::systemByteOrder() == QImage::LittleEndian)
    {
        png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
        png_set_bgr(png_ptr);
    }
    else
    {
        png_set_swap_alpha(png_ptr);
        png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
    }

    /* 16bit color -> 8bit color */
    if ( bit_depth == 16 )
        png_set_strip_16(png_ptr);

    /* pack all pixels to byte boundaires */

    png_set_packing(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_expand(png_ptr);

    lines = (unsigned char **)malloc(h * sizeof(unsigned char *));
    if (!lines)
    {
        png_read_end(png_ptr, info_ptr);
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(f);
        return qimage;
    }

    if (has_grey)
    {
        png_set_gray_to_rgb(png_ptr);
        if (png_get_bit_depth(png_ptr, info_ptr) < 8)
            png_set_gray_1_2_4_to_8(png_ptr);
    }

    int sizeOfUint = sizeof(unsigned int);
    for (i = 0; i < h; i++)
        lines[i] = ((unsigned char *)(qimage.bits())) +
                   (i * w * sizeOfUint);

    png_read_image(png_ptr, lines);
    free(lines);

    png_textp text_ptr;
    int num_text=0;
    png_get_text(png_ptr,info_ptr,&text_ptr,&num_text);
    while (num_text--) {
        qimage.setText(text_ptr->key,0,text_ptr->text);
        text_ptr++;
    }


    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
    fclose(f);

    return qimage;
}

kio_digikamthumbnailProtocol::kio_digikamthumbnailProtocol(const QCString &pool_socket,
                                                           const QCString &app_socket)
    : SlaveBase("kio_digikamthumbnail", pool_socket, app_socket)
{
    createThumbnailDirs();
}


kio_digikamthumbnailProtocol::~kio_digikamthumbnailProtocol()
{
}


void kio_digikamthumbnailProtocol::get(const KURL& url )
{
    int  size =  metaData("size").toInt();
    bool exif = (metaData("exif") == "yes");

    cachedSize_ = (size <= 128) ? 128 : 256;

    if (cachedSize_ <= 0)
    {
        error(KIO::ERR_INTERNAL, i18n("No or invalid size specified"));
        kdWarning() << "No or invalid size specified" << endl;
        return;
    }

    // generate the thumbnail path
    QString uri = "file://" + QDir::cleanDirPath(url.path(-1));
    KMD5 md5( QFile::encodeName(uri) );
    QString thumbPath = (cachedSize_ == 128) ? smallThumbPath_ : bigThumbPath_;
    thumbPath += QFile::encodeName( md5.hexDigest() ) + ".png";

    QImage img;
    bool regenerate = true;

    // stat the original file
    struct stat st;
    if (::stat(QFile::encodeName(url.path(-1)), &st) != 0)
    {
        error(KIO::ERR_INTERNAL, i18n("File does not exist"));
        return;
    }

    
    img = loadPNG(thumbPath);
    if (!img.isNull())
    {
        if (img.text("Thumb::MTime") == QString::number(st.st_mtime))
            regenerate = false;
    }

    if (regenerate)
    {
        // Try JPEG loading...
        if ( !loadJPEG(img, url.path()) )
        {
            // Try to load with imlib2 API...
            if ( !loadImlib2(img, url.path()) )
            {
                // Try to load with QT/KDELib API...
                if (!img.load(url.path()))
                {
                    // Try to load with dcraw
                    loadDCRAW( img, url.path() );
                }
            }
        }

        if (img.isNull())
        {
            error(KIO::ERR_INTERNAL, i18n("Cannot create thumbnail for %1")
                  .arg(url.prettyURL()));
            kdWarning() << "Cannot create thumbnail for " << url.path() << endl;
            return;
        }

        if (QMAX(img.width(),img.height()) != cachedSize_)
            img = img.smoothScale(cachedSize_, cachedSize_, QImage::ScaleMin);

        if (img.depth() != 32)
            img = img.convertDepth(32);

        img.setText(QString("Thumb::URI").latin1(),
                     0, uri);
        img.setText(QString("Thumb::MTime").latin1(),
                     0, QString::number(st.st_mtime));
        img.setText(QString("Software").latin1(),
                     0, QString("Digikam Thumbnail Generator"));

        KTempFile temp(thumbPath + "-digikam-", ".png");
        if (temp.status() == 0)
        {
            img.save(temp.name(), "PNG", 0);
            ::rename(QFile::encodeName(temp.name()),
                     QFile::encodeName(thumbPath));
        }
    }

    img = img.smoothScale(size, size, QImage::ScaleMin);
    if (exif)
        exifRotate(url.path(), img);

    QByteArray imgData;
    QDataStream stream( imgData, IO_WriteOnly );

    QString shmid = metaData("shmid");

    if (shmid.isEmpty())
    {
        stream << img;
    }
    else
    {
        void *shmaddr = shmat(shmid.toInt(), 0, 0);

        if (shmaddr == (void *)-1)
        {
            error(KIO::ERR_INTERNAL, "Failed to attach to shared memory segment " + shmid);
            kdWarning() << "Failed to attach to shared memory segment " << shmid << endl;
            return;
        }

        if (img.width() * img.height() > cachedSize_ * cachedSize_)
        {
            error(KIO::ERR_INTERNAL, "Image is too big for the shared memory segment");
            kdWarning() << "Image is too big for the shared memory segment" << endl;
            shmdt((char*)shmaddr);
            return;
        }

        stream << img.width() << img.height() << img.depth();
        memcpy(shmaddr, img.bits(), img.numBytes());
        shmdt((char*)shmaddr);
    }

    data(imgData);
    finished();
}


/////////////////////////////////////////////////////////////////////////////////////////
// JPEG Extraction

struct myjpeg_error_mgr : public jpeg_error_mgr
{
    jmp_buf setjmp_buffer;
};

extern "C"
{
    static void myjpeg_error_exit(j_common_ptr cinfo)
    {
        myjpeg_error_mgr* myerr =
            (myjpeg_error_mgr*) cinfo->err;

        char buffer[JMSG_LENGTH_MAX];
        (*cinfo->err->format_message)(cinfo, buffer);
        kdWarning() << buffer << endl;
        longjmp(myerr->setjmp_buffer, 1);
    }
}

bool kio_digikamthumbnailProtocol::loadJPEG(QImage& image, const QString& path)
{
    QString format=QImageIO::imageFormat(path);
    if (format !="JPEG") return false;

    FILE* inputFile=fopen(QFile::encodeName(path), "rb");
    if(!inputFile)
        return false;

    struct jpeg_decompress_struct    cinfo;
    struct myjpeg_error_mgr jerr;

    // JPEG error handling - thanks to Marcus Meissner
    cinfo.err             = jpeg_std_error(&jerr);
    cinfo.err->error_exit = myjpeg_error_exit;

    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(inputFile);
        return false;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, inputFile);
    jpeg_read_header(&cinfo, TRUE);

    int imgSize = QMAX(cinfo.image_width, cinfo.image_height);

    int scale=1;
    while(cachedSize_*scale*2<=imgSize) {
        scale*=2;
    }
    if(scale>8) scale=8;

    cinfo.scale_num=1;
    cinfo.scale_denom=scale;

    // Create QImage
    jpeg_start_decompress(&cinfo);

    QImage img;

    switch(cinfo.output_components) {
    case 3:
    case 4:
        img.create( cinfo.output_width, cinfo.output_height, 32 );
        break;
    case 1: // B&W image
        img.create( cinfo.output_width, cinfo.output_height,
                    8, 256 );
        for (int i=0; i<256; i++)
            img.setColor(i, qRgb(i,i,i));
        break;
    default:
        return false;
    }

    uchar** lines = img.jumpTable();
    while (cinfo.output_scanline < cinfo.output_height)
        jpeg_read_scanlines(&cinfo, lines + cinfo.output_scanline,
                            cinfo.output_height);
    jpeg_finish_decompress(&cinfo);

    // Expand 24->32 bpp
    if ( cinfo.output_components == 3 ) {
        for (uint j=0; j<cinfo.output_height; j++) {
            uchar *in = img.scanLine(j) + cinfo.output_width*3;
            QRgb *out = (QRgb*)( img.scanLine(j) );

            for (uint i=cinfo.output_width; i--; ) {
                in-=3;
                out[i] = qRgb(in[0], in[1], in[2]);
            }
        }
    }

    int newMax = QMAX(cinfo.output_width, cinfo.output_height);
    int newx = cachedSize_*cinfo.output_width / newMax;
    int newy = cachedSize_*cinfo.output_height / newMax;

    jpeg_destroy_decompress(&cinfo);
    fclose(inputFile);

    image = img.smoothScale(newx,newy);

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Load using Imlib2 API

bool kio_digikamthumbnailProtocol::loadImlib2(QImage& image, const QString& path)
{
    Imlib_Image imlib2_im =
        imlib_load_image_immediately_without_cache(QFile::encodeName(path));

    if (imlib2_im == NULL) {
        return false;
    }

    imlib_context_set_image(imlib2_im);

    org_width_  = imlib_image_get_width();
    org_height_ = imlib_image_get_height();

    if ( QMAX(org_width_, org_height_) != cachedSize_ )
    {
        imlib2_im = imlib_create_cropped_scaled_image(0, 0,
                                                      org_width_, org_height_,
                                                      cachedSize_, cachedSize_);
    }

    new_width_  = imlib_image_get_width();
    new_height_ = imlib_image_get_height();

    image.create( new_width_, new_height_, 32 );
    image.setAlphaBuffer(true) ;

    DATA32 *data = imlib_image_get_data();
    if (!data)
        return false;

    memcpy(image.bits(), data, image.numBytes());

    imlib_free_image();
    return true;
}

bool kio_digikamthumbnailProtocol::loadDCRAW(QImage& image, const QString& path)
{
    // first try with Dave Coffin's "parse" utility

    kdDebug() << k_funcinfo << "Parsing file: " << path << endl;

    KTempFile thumbFile(QString::null, "rawthumb");
    thumbFile.setAutoDelete(true);
    if (thumbFile.status() == 0)
    {
        if (dcraw_identify(QFile::encodeName(path),
                           QFile::encodeName(thumbFile.name())) == 0)
        {
            image.load(thumbFile.name());
            if (!image.isNull())
                return true;
        }
    }
    
    QCString command;

    // run dcraw with options:
    // -c : write to stdout
    // -h : Half-size color image (3x faster than -q)
    // -2 : 8bit ppm output
    // -a : Use automatic white balance
    // -w : Use camera white balance, if possible
    command  = "dcraw -c -h -2 -w -a ";
    command += "'";
    command += QFile::encodeName( path );
    command += "'";
    kdDebug() << "Running dcraw command " << command << endl;

    FILE* f = popen( command.data(), "r" );

    QByteArray imgData;

    if ( !f )
        return false;

    const int MAX_IPC_SIZE = (1024*32);
    char buffer[MAX_IPC_SIZE];

    QFile file;
    file.open( IO_ReadOnly,  f );
    Q_LONG len;
    while ((len = file.readBlock(buffer, MAX_IPC_SIZE)) != 0)
    {
        if ( len == -1 )
        {
            file.close();
            return false;
        }
        else
        {
            int oldSize = imgData.size();
            imgData.resize( imgData.size() + len );
            memcpy(imgData.data()+oldSize, buffer, len);
        }
    }

    file.close();
    pclose( f );

    if ( imgData.isEmpty() )
        return false;

    image.loadFromData( imgData );
    return true;
}

void kio_digikamthumbnailProtocol::createThumbnailDirs()
{
    QString path = QDir::homeDirPath() + "/.thumbnails/";

    smallThumbPath_ = path + "normal/";
    bigThumbPath_   = path + "large/";

    KStandardDirs::makeDir(smallThumbPath_, 0700);
    KStandardDirs::makeDir(bigThumbPath_, 0700);
}

/////////////////////////////////////////////////////////////////////////////////////////
// KIO slave registration

extern "C"
{
    DIGIKAMIMAGEPLUGINS_EXPORT int kdemain(int argc, char **argv)
    {
        KLocale::setMainCatalogue("digikam");
        KInstance instance( "kio_digikamthumbnail" );
        ( void ) KGlobal::locale();

        kdDebug() << "*** Starting kio_digikamthumbnail " << endl;

        if (argc != 4) {
            kdDebug() << "Usage: kio_digikamthumbnail  protocol domain-socket1 domain-socket2"
                      << endl;
            exit(-1);
        }

        KImageIO::registerFormats();

        kio_digikamthumbnailProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        kdDebug() << "*** kio_digikamthumbnail Done" << endl;
        return 0;
    }
}
