#include <qptrlist.h>
#include <qheader.h>

#include <klocale.h>

#include "kexifentry.h"
#include "kexiflistviewitem.h"
#include "kexiflistview.h"

KExifListView::KExifListView(QWidget* parent)
    : KListView(parent)
{
    header()->hide();
    addColumn(i18n("Name"));
    addColumn(i18n("Value"));
    setSorting(-1);
    setResizeMode(QListView::AllColumns);
    setAllColumnsShowFocus(true);

    connect(this, SIGNAL(selectionChanged(QListViewItem*)),
            this, SLOT(slot_selectionChanged(QListViewItem*)));
}


KExifListView::~KExifListView()
{

}

void KExifListView::addItems(const QPtrList<KExifEntry>& entryList)
{
    QPtrListIterator<KExifEntry>
        entryIterator(entryList);

    KExifEntry *exifEntry   = 0;
    KExifListViewItem *prevItem = 0;

    while( (exifEntry = entryIterator.current()) != 0 ) {
        ++entryIterator;

        KExifListViewItem *item = 0;
        if (!prevItem)
            item = new KExifListViewItem(this, exifEntry);
        else
            item = new KExifListViewItem(this, prevItem,
                                         exifEntry);
        prevItem = item;

    }
}

void KExifListView::slot_selectionChanged(QListViewItem *item)
{
    if (!item) {
        emit signal_itemDescription(QString(""));
        return;
    }

    KExifListViewItem* viewItem =
        static_cast<KExifListViewItem *>(item);
    emit signal_itemDescription(viewItem->exifEntry()->getDescription());

}

#include "kexiflistview.moc"
