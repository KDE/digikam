#include "kexifentry.h"

KExifEntry::KExifEntry()
{
    mExifEntry = 0;
    mName  = "";
    mTitle = "";
    mValue = "";
    mDescription = "";
}

KExifEntry::KExifEntry(ExifEntry* exifEntry)
{
    mExifEntry = 0;
    mName  = "";
    mTitle = "";
    mValue = "";
    mDescription = "";

    if (exifEntry) {
        setEntry(exifEntry);
    }

}

KExifEntry::~KExifEntry()
{
    if (mExifEntry)
        exif_entry_unref(mExifEntry);
}

KExifEntry::KExifEntry(const KExifEntry& entry)
{
    if (this != &entry) {
        setEntry(entry.mExifEntry);
    }
}

KExifEntry& KExifEntry::operator=(const KExifEntry& entry)
{
    if (this != &entry) {
        setEntry(entry.mExifEntry);
    }
    return (*this);
}

void KExifEntry::setEntry(ExifEntry* exifEntry)
{
    if (exifEntry) {

        if (mExifEntry) {
            exif_entry_unref(mExifEntry);
            mExifEntry = 0;
        }

        mExifEntry = exifEntry;
        exif_entry_ref(mExifEntry);

        readEntry();
    }
}

void KExifEntry::readEntry()
{
    mName  = QString(exif_tag_get_name(mExifEntry->tag));
    mTitle = QString(exif_tag_get_title(mExifEntry->tag));
    mDescription = QString(exif_tag_get_description(mExifEntry->tag));
    char *val = (char *)malloc(1024);
    exif_entry_get_value(mExifEntry, val, 1023);
    mValue = QString(val);
}

QString KExifEntry::getName()
{
    return mName;
}

QString KExifEntry::getTitle()
{
    return mTitle;
}

QString KExifEntry::getValue()
{
    return mValue;
}

QString KExifEntry::getDescription()
{
    return mDescription;
}

