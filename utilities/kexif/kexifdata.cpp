#include <stdlib.h>
#include "kexififd.h"
#include "kexifdata.h"
#include "kdebug.h"

#include <qfile.h>
#include <kio/job.h>


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


void KExifData::saveExifComment(QString& filename, QString& comment)
{
    unsigned int count = 0;
    Q_UINT8 byte;

    /* Write to new Jpeg file and replace the EXIF data with the new data on the fly */
    QFile file( filename );
    file.open( IO_ReadWrite );

    QDataStream stream( &file );

    // TODO get byte order from jpeg file!
    stream.setByteOrder(QDataStream::LittleEndian);
 
    stream >> byte;

    // skip until EXIF marker is found
    while(!stream.atEnd())
    {
      while(byte != 0xff) {
        stream >> byte;
      }
      stream >> byte;

      // consume 0xff's used for padding
      while(byte == 0xff)
        stream >> byte;

      // stop when we reach APP0 marker or start of image data
      if(byte == 0xe1 || byte == 0xc0)  // TODO: check more markers!
        break;
    }

    if(byte != 0xe1) {
      kdDebug() << "No EXIF information found." << endl;
      file.close();
      return;
    }

    Q_UINT16 sectionLen;
    stream >> sectionLen;


    // use counter to find byte offsets
    count = 0;

    QMemArray<unsigned char> buf(sectionLen);

    Q_UINT32 userCommentOffset = 0;
    Q_UINT32 oldCommentLength = 0;

    for(int i=0 ; i < (sectionLen - 2) ; i++)
    {
      stream >> byte;
      buf[i] = byte;
      count++;

      // search for UserComment tag
      if(i > 4 && buf[i]==0x00 && buf[i-1]==0x07 && buf[i-2]==0x92 && buf[i-3]==0x86)
      {
        stream >> oldCommentLength;
        stream >> userCommentOffset;
        count+=8;
        kdDebug() << "Found UserComment offset: " << userCommentOffset << endl;
        kdDebug() << "Found oldCommentLength: " << oldCommentLength << endl;
        break;
      }
    }
    if(!userCommentOffset)
    {
      kdDebug() << "No EXIF UserComment found." << endl;
      file.close();
      return;
    }

    while(count < userCommentOffset + 6)
    {
      stream >> byte;
      count++;
    }

    // write character code ASCII into offset position
    stream << 0x49435341;
    stream << 0x00000049;

    // don't write more than the old field length
    unsigned int writeLen = (oldCommentLength < comment.length() + 8) ? oldCommentLength - 8 : comment.length(); 

    // write comment into stream
    for(unsigned int i = 0 ; i < writeLen ; i++)
    {
      stream << (Q_UINT8)comment.at(i).latin1();
    }

    // overwrite rest of old comment. Standard suggests \20, but that's not done in practice 
    for(unsigned int i=0 ; i < oldCommentLength - writeLen - 8; i++)
    {
      stream << (Q_UINT8)0x00;
    }

    file.close();
    file.flush();

    kdDebug() << "Wrote file '" << filename.latin1() << "'." << endl;

    return;
}

