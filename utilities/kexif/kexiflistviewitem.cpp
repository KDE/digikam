#include "kexifentry.h"
#include "kexiflistviewitem.h"

KExifListViewItem::KExifListViewItem(KListView *parent,
                                     KExifEntry* entry)
    : KListViewItem(parent)
{
    mExifEntry = 0;
    if (entry) {
        mExifEntry = entry;
        setText(0,mExifEntry->getName());
        setText(1,mExifEntry->getValue());
    }

}

KExifListViewItem::KExifListViewItem(KListView *parent,
                                     QListViewItem* afterItem,
                                     KExifEntry* entry)
    : KListViewItem(parent, afterItem)
{
    mExifEntry = 0;
    if (entry) {
        mExifEntry = entry;
        setText(0,mExifEntry->getName());
        setText(1,mExifEntry->getValue());
    }
}


KExifListViewItem::~KExifListViewItem()
{
    if (mExifEntry) {
        mExifEntry = 0;
    }
}

