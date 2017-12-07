/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-21
 * Description : widget to display an imagelist
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2010 by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2009-2010 by Luka Renko <lure at kubuntu dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIMAGES_LIST_H
#define DIMAGES_LIST_H

// Qt includes

#include <QPushButton>
#include <QStringList>
#include <QTreeWidget>
#include <QWidget>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QIcon>
#include <QUrl>

// Local includes

#include "loadingdescription.h"
#include "digikam_export.h"
#include "dinfointerface.h"

namespace Digikam
{

class DImagesList;
class DImagesListView;

class DIGIKAM_EXPORT DImagesListViewItem : public QTreeWidgetItem
{

public:

    enum State
    {
        Waiting,
        Success,
        Failed
    };

public:

    explicit DImagesListViewItem(DImagesListView* const view, const QUrl& url);
    ~DImagesListViewItem();

    bool hasValidThumbnail() const;

    void setUrl(const QUrl& url);
    QUrl url() const;

    void setComments(const QString& comments);
    QString comments() const;

    void setTags(const QStringList& tags);
    QStringList tags() const;

    void setRating(int rating);
    int rating() const;

    void setThumb(const QPixmap& pix, bool hasThumb=true);
    void setProgressAnimation(const QPixmap& pix);

    void setProcessedIcon(const QIcon& icon);
    void setState(State state);
    State state() const;

    void updateInformation();

    // implement this, if you have special item widgets, e.g. an edit line
    // they will be set automatically when adding items, changing order, etc.
    virtual void updateItemWidgets() {};

protected:

    DImagesListView* view() const;

private:

    void setPixmap(const QPixmap& pix);

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DImagesListView : public QTreeWidget
{
    Q_OBJECT

public:

    enum ColumnType
    {
        Thumbnail = 0,
        Filename,
        User1,
        User2,
        User3,
        User4,
        User5,
        User6
    };

public:

    explicit DImagesListView(DImagesList* const parent = 0);
    explicit DImagesListView(int iconSize, DImagesList* const parent = 0);
    ~DImagesListView();

    void setColumnLabel(ColumnType column, const QString& label);
    void setColumnEnabled(ColumnType column, bool enable);
    void setColumn(ColumnType column, const QString& label, bool enable);

    DImagesListViewItem* findItem(const QUrl& url);
    QModelIndex indexFromItem(DImagesListViewItem* item, int column = 0) const;
    DImagesListViewItem* getCurrentItem() const;

    DInfoInterface* iface() const;

Q_SIGNALS:

    void signalAddedDropedItems(const QList<QUrl>&);
    void signalItemClicked(QTreeWidgetItem*);
    void signalContextMenuRequested();

private Q_SLOTS:

    void slotItemClicked(QTreeWidgetItem* item, int column);

public:

    void enableDragAndDrop(const bool enable = true);

private:

    void dragEnterEvent(QDragEnterEvent* e);
    void dragMoveEvent(QDragMoveEvent* e);
    void dropEvent(QDropEvent* e);
    void contextMenuEvent(QContextMenuEvent * e);

    void setup(int iconSize);

    void drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const;

private:

    int m_iconSize;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT CtrlButton : public QPushButton
{
    Q_OBJECT

public:

    explicit CtrlButton(const QIcon& icon, QWidget* parent = 0);
    virtual ~CtrlButton();
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DImagesList : public QWidget
{
    Q_OBJECT

public:

    enum ControlButtonPlacement
    {
        NoControlButtons = 0,
        ControlButtonsLeft,
        ControlButtonsRight,
        ControlButtonsAbove,
        ControlButtonsBelow
    };

    enum ControlButton
    {
       Add       = 0x1,
       Remove    = 0x2,
       MoveUp    = 0x4,
       MoveDown  = 0x8,
       Clear     = 0x10,
       Load      = 0x20,
       Save      = 0x40
    };
    Q_DECLARE_FLAGS(ControlButtons, ControlButton)

public:

    explicit DImagesList(QWidget* const parent = 0, int iconSize = -1);
    virtual ~DImagesList();

    void                setAllowRAW(bool allow);
    void                setAllowDuplicate(bool allow);

    void                loadImagesFromCurrentSelection();

    /** A function to load all the images from the album if no image has been selected by user.
     */
    void                loadImagesFromCurrentAlbum();

    /** a function to check whether an image has been selected or not.
     */
    bool                checkSelection();

    int                 iconSize()  const;

    DImagesListView*    listView()  const;

    void                processing(const QUrl& url);
    void                processed(const QUrl& url, bool success);
    void                cancelProcess();
    void                clearProcessedStatus();

    void                setControlButtons(ControlButtons buttonMask);
    void                setControlButtonsPlacement(ControlButtonPlacement placement);
    void                enableControlButtons(bool enable = true);
    void                enableDragAndDrop(const bool enable = true);

    void                updateThumbnail(const QUrl& url);

    virtual QList<QUrl> imageUrls(bool onlyUnprocessed = false) const;
    virtual void        removeItemByUrl(const QUrl& url);
    QUrl                getCurrentUrl() const;

    void setIface(DInfoInterface* const iface);
    DInfoInterface* iface() const;

Q_SIGNALS:

    void signalAddItems(const QList<QUrl>&);
    void signalMoveUpItem();
    void signalMoveDownItem();
    void signalRemovedItems(const QList<int>&);
    void signalImageListChanged();
    void signalFoundRAWImages(bool);
    void signalItemClicked(QTreeWidgetItem*);
    void signalContextMenuRequested();
    void signalXMLSaveItem(QXmlStreamWriter&, int);
    void signalXMLLoadImageElement(QXmlStreamReader&);
    void signalXMLCustomElements(QXmlStreamWriter&);
    void signalXMLCustomElements(QXmlStreamReader&);

public Q_SLOTS:

    virtual void slotAddImages(const QList<QUrl>& list);
    virtual void slotRemoveItems();

protected Q_SLOTS:

    void slotProgressTimerDone();

    virtual void slotAddItems();
    virtual void slotMoveUpItems();
    virtual void slotMoveDownItems();
    virtual void slotClearItems();
    virtual void slotLoadItems();
    virtual void slotSaveItems();
    virtual void slotThumbnail(const LoadingDescription&, const QPixmap&);
    virtual void slotImageListChanged();

private:

    void setIconSize(int size);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DImagesList::ControlButtons)

#endif // DIMAGES_LIST_H
