#ifndef KEXIFENTRY_H
#define KEXIFENTRY_H

#include <qstring.h>

extern "C" {
#include <libexif/exif-entry.h>
#include <stdlib.h>
}

class KExifEntry {

public:

    KExifEntry();
    KExifEntry(ExifEntry* exifEntry);
    ~KExifEntry();

    KExifEntry(const KExifEntry& entry);
    KExifEntry& operator=(const KExifEntry& entry);

    void setEntry(ExifEntry* exifEntry);

    QString getName();
    QString getTitle();
    QString getValue();
    QString getDescription();

    ExifEntry* exifEntry() {
        return mExifEntry;
    }

private:

    ExifEntry* mExifEntry;

    QString mName;
    QString mTitle;
    QString mValue;
    QString mDescription;

    void readEntry();

};

#endif
