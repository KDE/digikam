#ifndef KEXIFIFD_H
#define KEXIFIFD_H

#include <qptrlist.h>
#include <qstring.h>

extern "C" {
#include <libexif/exif-content.h>
}

class KExifEntry;

class KExifIfd {

public:

    KExifIfd();
    KExifIfd(const QString& name, ExifContent* content);
    ~KExifIfd();

    KExifIfd(const KExifIfd& ifd);
    KExifIfd& operator=(const KExifIfd& ifd);


    void setName(const QString& name);
    QString getName();
    void setContent(ExifContent* content);
    int numberEntries();

    QPtrList<KExifEntry> entryList;

private:

    ExifContent* mExifContent;
    QString mName;

};

#endif
