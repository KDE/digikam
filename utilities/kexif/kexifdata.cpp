#include <stdlib.h>
#include "kexififd.h"
#include "kexifdata.h"
#include "kexifentry.h"
#include "kdebug.h"
#include <libexif/exif-utils.h>

#include <qfile.h>
#include <kio/job.h>


KExifData::KExifData()
{
    ifdVector.clear();
    mExifData = 0;
    mExifByteOrder = "";
    mUserComment = "";
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

QString KExifData::getUserComment()
{
   if(mUserComment == "")
   {
      QValueVector<KExifIfd>::iterator ifdIterator;
      for (ifdIterator = ifdVector.begin();
            ifdIterator != ifdVector.end();
            ++ifdIterator) {

         if( ifdIterator->getName() == "EXIF" )
         {
            QPtrListIterator<KExifEntry> entryIterator((*ifdIterator).entryList);

            KExifEntry *exifEntry = 0;

            while( (exifEntry = entryIterator.current()) != 0 ) {
               ++entryIterator;
               if(exifEntry->getName() == "UserComment") mUserComment = exifEntry->getValue();
            }
         }
      }
   }

   return mUserComment;

}

KExifData::ImageOrientation KExifData::getImageOrientation()
{
   if (mExifData) {
      //get byte order for later reading
      ExifByteOrder o=exif_data_get_byte_order(mExifData);
      //retrieve Ifd0
      ExifContent *ifd0=mExifData->ifd[EXIF_IFD_0];
      //look for entry in ifd0
      ExifEntry *entry=exif_content_get_entry(ifd0, EXIF_TAG_ORIENTATION);

      if (entry && entry->format == EXIF_FORMAT_SHORT && entry->components == 1) {
         //read value
         ExifShort s = exif_get_short(entry->data, o);

         return (ImageOrientation)s;
      }
   }

   // if something went wrong, return NORMAL
   return NORMAL;
   
}



void KExifData::saveFile(const QString& filename)
{
    FILE *f;
    f = fopen (filename.latin1(), "wb");
    fwrite (mExifData->data, 1, mExifData->size, f);
    fclose (f);
}


void KExifData::writeOrientation(const QString& filename, ImageOrientation orientation)
{
   QString str = "";
   writeFile( filename, str, orientation);
}

void KExifData::writeComment(const QString& filename, const QString& comment)
{
   writeFile( filename, comment, UNSPECIFIED );
}

void KExifData::writeFile(const QString& filename, const QString& comment, ImageOrientation orientation)
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

    if( comment != "" ) 
    {
       // replace EXIF UserCommentTag
       Q_UINT32 userCommentOffset = 0;
       Q_UINT32 oldCommentLength = 0;

       for(int i=0 ; i < (sectionLen - 2) ; i++)
       {
          stream >> byte;
          buf[i] = byte;
          count++;

          // search for UserComment tag
          // this code is not perfect, but the probability that the sequence below is not
          // the UserComment tag is very small.
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
    } 
    else 
    {
       // replace EXIF orientation tag
       for(int i=0 ; i < (sectionLen - 2) ; i++)
       {
          stream >> byte;
          buf[i] = byte;
          count++;

          // search for Orientation tag
          // this code is not perfect, but the probability that the sequence below is not
          // the Orientation tag is very small.
          if(i > 8 && buf[i]==0x00 && buf[i-1]==0x00 && buf[i-2]==0x00 && buf[i-3]==0x01 
                 && buf[i-4]==0x00 && buf[i-5]==0x03 && buf[i-6]==0x01 && buf[i-7]==0x12)
          {
             stream << orientation;
             kdDebug() << "Wrote Image Orientation: " << orientation << endl;
             break;
          }
       }
    }

    file.close();
    file.flush();

    kdDebug() << "Wrote file '" << filename.latin1() << "'." << endl;

    return;
}

