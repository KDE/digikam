#include "kexifentry.h"
#include "kexififd.h"

KExifIfd::KExifIfd()
{
    mName = "";
    entryList.setAutoDelete(true);
    mExifContent = 0;
}

KExifIfd::KExifIfd(const QString& name, ExifContent* content)
{
    mName = name;
    entryList.setAutoDelete(true);
    mExifContent = 0;

    setContent(content);
}

KExifIfd::~KExifIfd()
{
    entryList.clear();

    if (mExifContent)
        exif_content_unref(mExifContent);
}

KExifIfd::KExifIfd(const KExifIfd& ifd)
{
    if (this != &ifd) {
        mName = ifd.mName;
        entryList.setAutoDelete(true);
        setContent(ifd.mExifContent);
    }
}

KExifIfd& KExifIfd::operator=(const KExifIfd& ifd)
{
    if (this != &ifd) {
        mName = ifd.mName;
        entryList.setAutoDelete(true);
        setContent(ifd.mExifContent);
    }
    return (*this);
}


void KExifIfd::setName(const QString& name)
{
    mName = name;
}

void KExifIfd::setContent(ExifContent* content)
{
    if (!content) return;

    if (mExifContent) {
        exif_content_unref(mExifContent);
        mExifContent = 0;
    }

    mExifContent = content;
    exif_content_ref(mExifContent);
    entryList.clear();

    for (unsigned int i=0; i<content->count; i++) {
        entryList.append(new KExifEntry(content->entries[i]));
    }

}

int KExifIfd::numberEntries()
{
    return entryList.count();
}


QString KExifIfd::getName()
{
    return mName;
}
