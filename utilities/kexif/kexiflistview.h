#ifndef KEXIFLISTVIEW_H
#define KEXIFLISTVIEW_H

#include <klistview.h>
#include <qptrlist.h>

class KExifEntry;

class KExifListView : public KListView {

    Q_OBJECT

public:

    KExifListView(QWidget* parent);
    ~KExifListView();

    void addItems(const QPtrList<KExifEntry>& entryList);

private slots:

    void slot_selectionChanged(QListViewItem *item);

signals:

    void signal_itemDescription(const QString& desc);

};

#endif
