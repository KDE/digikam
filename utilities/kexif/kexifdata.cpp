#include <stdlib.h>
#include "kexififd.h"
#include "kexifdata.h"
#include "kdebug.h"

#include <qfile.h>
#include <qdir.h>


KExifData::KExifData()
{
    ifdVector.clear();
    mExifData = 0;
    mExifByteOrder = "";
}

KExifData::~KExifData()
{
    ifdVector.clear();

    if (mExifData) {
        exif_data_unref(mExifData);
        mExifData = 0;
    }
}

int KExifData::readFromFile(const QString& filename)
{
    if (mExifData) {
        exif_data_unref(mExifData);
        mExifData = 0;
    }

    mExifData = exif_data_new_from_file(filename.latin1());

    if (!mExifData) {
        return NOEXIF;
    }

    exif_data_ref(mExifData);

    ExifByteOrder order;
    order = exif_data_get_byte_order(mExifData);
    mExifByteOrder = QString(exif_byte_order_get_name(order));

    // -- Load IFDs ------------------------------------------

    if (EXIF_IFD_COUNT) {

        ifdVector.clear();

        for (unsigned int i=0; i < EXIF_IFD_COUNT; i++) {

            QString ifdName(exif_ifd_get_name((ExifIfd)i));
            KExifIfd ifd(ifdName, mExifData->ifd[i]);
            ifdVector.push_back(ifd);
        }
    }

    // -- Load thumbnail -------------------------------------

    if (mExifData->data) {
        mThumbnail.loadFromData(mExifData->data,
                                mExifData->size);
    }

    return SUCCESS;
}

int KExifData::readFromData(char* data, int size)
{
    if (mExifData) {
        exif_data_unref(mExifData);
        mExifData = 0;
    }

    mExifData = exif_data_new_from_data((const unsigned char*)data, size);

    if (!mExifData) {
        qWarning("Data has No Exif Content");
        return NOEXIF;
    }

    exif_data_ref(mExifData);

    ExifByteOrder order;
    order = exif_data_get_byte_order(mExifData);
    mExifByteOrder = QString(exif_byte_order_get_name(order));

    // -- Load IFDs ------------------------------------------

    if (EXIF_IFD_COUNT) {

        ifdVector.clear();

        for (unsigned int i=0; i < EXIF_IFD_COUNT; i++) {

            QString ifdName(exif_ifd_get_name((ExifIfd)i));
            KExifIfd ifd(ifdName, mExifData->ifd[i]);
            ifdVector.push_back(ifd);
        }
    }

    // -- Load thumbnail -------------------------------------

    if (mExifData->data) {
        mThumbnail.loadFromData(mExifData->data,
                                mExifData->size);
    }

    return SUCCESS;
}

int KExifData::getThumbnail(QImage& thumb)
{
    if (mThumbnail.isNull())
        return ERROR;

    thumb = mThumbnail;
    return SUCCESS;
}


void KExifData::saveFile(const QString& filename)
{
    FILE *f;
    f = fopen (filename.latin1(), "wb");
    fwrite (mExifData->data, 1, mExifData->size, f);
    fclose (f);
}

int KExifData::setUserComment(QString& comment)
{
    ExifEntry *e;
    
    e = exif_content_get_entry (mExifData->ifd[EXIF_IFD_EXIF], EXIF_TAG_USER_COMMENT); 

    kdDebug() << "Got EXIF tag: " << e->tag << endl;

    if (!e) {
        e = exif_entry_new ();
        exif_entry_initialize (e, EXIF_TAG_USER_COMMENT);
        kdDebug() << "No UserComment field. Creating it. " << endl;
    }

    if (e->data)
        free (e->data);

    e->size = comment.length() + 8;
    e->data = (unsigned char *)malloc ((sizeof (char) * e->size) + 1);

    if (!e->data) {
         kdDebug() << "Not enough memory for EXIF data." << endl;
         return (0);
    }

    char characterCode[8] = "ASCII";
    characterCode[5] = 0;
    characterCode[6] = 0;
    characterCode[7] = 0;

    strncpy((char *)e->data, characterCode, 8);
    strncpy(((char *)e->data) + 8, comment.latin1(), comment.length());
    e->components = comment.length() + 8;

    return 1;
}


void KExifData::saveExifData(QString& filename)
{
    unsigned char *d = NULL;
    unsigned int ds;


    /* Make sure the EXIF data is not too big. */
    exif_data_save_data (mExifData, &d, &ds);
    if (ds) {
        if (ds > 0xffff) {
            kdDebug() << "Too much EXIF data (" << ds << " bytes). Only 0xffff bytes are allowed." << endl;
            return;
        }
    };

    /* Write to new Jpeg file and replace the EXIF data with the new data on the fly */
    QFile file( filename );
    QFile fileOut( filename + ".exiftmp" );
    file.open( IO_ReadOnly );
    fileOut.open( IO_WriteOnly );
    QDataStream instream( &file );
    QDataStream outstream( &fileOut );

    Q_UINT16 twobytes = 0x0000;
    while(twobytes != 0xffe1) {
      instream >> twobytes;
      outstream << twobytes;
    }

    Q_UINT16 sectionLen;
    instream >> sectionLen;

    // skip old EXIF data
    Q_UINT8 byte;
    for(Q_UINT16 i=0 ; i < (sectionLen - 4) ; i++)
      instream >> byte;

    kdDebug() << "EXIF data is of length " << ds << endl;
    outstream << (Q_INT16)(ds + 4);
    outstream.writeRawBytes((char *)d, ds);

    kdDebug() << "Found section of length " << sectionLen << endl;

    while(!instream.atEnd()) {
      instream >> byte;
      outstream << byte;
    }

    file.close();
    fileOut.close();
    file.remove();
    QDir dir;
    dir.rename( filename + ".exiftmp", filename ,TRUE );

    kdDebug() << "Wrote file '" << filename.latin1() << "'." << endl;

    return;
}

