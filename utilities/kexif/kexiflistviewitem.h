#ifndef KEXIFLISTVIEWITEM_H
#define KEXIFLISTVIEWITEM_H

#include <klistview.h>

class KExifEntry;

class KExifListViewItem : public KListViewItem {

public:

    KExifListViewItem(KListView *parent, KExifEntry* entry);
    KExifListViewItem(KListView *parent, QListViewItem* afterItem,
                      KExifEntry* entry);
    ~KExifListViewItem();

    KExifEntry* exifEntry() const {
        return mExifEntry;
    }

private:

    KExifEntry* mExifEntry;

};

#endif
