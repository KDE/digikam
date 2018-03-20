/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006      by Tobias Koenig <tokoe at kde dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DBCONFIGDLG_WIDGETS_H
#define DBCONFIGDLG_WIDGETS_H

// Qt includes

#include <QWidget>

// Local includes

#include "dconfigdlgmodels.h"
#include "dconfigdlgview.h"
#include "digikam_export.h"

namespace Digikam
{

class DConfigDlgWdgPrivate;
/**
 * @short Page widget with many layouts (faces).
 * @see DConfigDlgView with hierarchical page model.
 */
class DIGIKAM_EXPORT DConfigDlgWdg : public DConfigDlgView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DConfigDlgWdg)

public:

    /**
     * Creates a new page widget.
     *
     * @param parent The parent widget.
     */
    explicit DConfigDlgWdg(QWidget* const parent = 0);

    /**
     * Destroys the page widget.
     */
    ~DConfigDlgWdg();

    /**
     * Adds a new top level page to the widget.
     *
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated @see DConfigDlgWdgItem.
     */
    DConfigDlgWdgItem* addPage(QWidget* widget, const QString& name);

    /**
     * Adds a new top level page to the widget.
     *
     * @param item The @see DConfigDlgWdgItem which describes the page.
     */
    void addPage(DConfigDlgWdgItem* item);

    /**
     * Inserts a new page in the widget.
     *
     * @param before The new page will be insert before this @see DConfigDlgWdgItem
     *               on the same level in hierarchy.
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated @see DConfigDlgWdgItem.
     */
    DConfigDlgWdgItem* insertPage(DConfigDlgWdgItem* before, QWidget* widget, const QString& name);

    /**
     * Inserts a new page in the widget.
     *
     * @param before The new page will be insert before this @see DConfigDlgWdgItem
     *               on the same level in hierarchy.
     *
     * @param item The @see DConfigDlgWdgItem which describes the page.
     */
    void insertPage(DConfigDlgWdgItem* before, DConfigDlgWdgItem* item);

    /**
     * Inserts a new sub page in the widget.
     *
     * @param parent The new page will be insert as child of this @see DConfigDlgWdgItem.
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated @see DConfigDlgWdgItem.
     */
    DConfigDlgWdgItem* addSubPage(DConfigDlgWdgItem* parent, QWidget* widget, const QString& name);

    /**
     * Inserts a new sub page in the widget.
     *
     * @param parent The new page will be insert as child of this @see DConfigDlgWdgItem.
     *
     * @param item The @see DConfigDlgWdgItem which describes the page.
     */
    void addSubPage(DConfigDlgWdgItem* parent, DConfigDlgWdgItem* item);

    /**
     * Removes the page associated with the given @see DConfigDlgWdgItem.
     */
    void removePage(DConfigDlgWdgItem* item);

    /**
     * Sets the page which is associated with the given @see DConfigDlgWdgItem to
     * be the current page and emits the currentPageChanged() signal.
     */
    void setCurrentPage(DConfigDlgWdgItem* item);

    /**
     * Returns the @see DConfigDlgWdgItem for the current page or 0 if there is no
     * current page.
     */
    DConfigDlgWdgItem* currentPage() const;

Q_SIGNALS:

    /**
     * This signal is emitted whenever the current page has changed.
     *
     * @param item The new current page or 0 if no current page is available.
     */
    void currentPageChanged(DConfigDlgWdgItem* current, DConfigDlgWdgItem* before);

    /**
     * This signal is emitted whenever a checkable page changes its state. @param checked is true
     * when the @param page is checked, or false if the @param page is unchecked.
     */
    void pageToggled(DConfigDlgWdgItem* page, bool checked);

    /**
     * This signal is emitted when a page is removed.
     * @param page The page which is removed
     * */
    void pageRemoved(DConfigDlgWdgItem* page);

protected:

    DConfigDlgWdg(DConfigDlgWdgPrivate& dd, QWidget* const parent);

private:

    Q_PRIVATE_SLOT(d_func(), void _k_slotCurrentPageChanged(const QModelIndex&, const QModelIndex&))
};

// ----------------------------------------------------------------------------------------------------------------

/**
 * This class provides a widget often used for DConfigDlg titles.
 *
 * DConfigDlgTitle uses the general application font at 1.4 times its size to
 * style the text.
 *
 * DConfigDlgTitle is very simple to use. You can either use its default text
 * (and pixmap) properties or display your own widgets in the title widget.
 *
 */
class DConfigDlgTitle : public QWidget
{
    Q_OBJECT
    Q_ENUMS(ImageAlignment)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString comment READ comment WRITE setComment)
    Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)
    Q_PROPERTY(int autoHideTimeout READ autoHideTimeout WRITE setAutoHideTimeout)

public:

    /**
     * Possible title pixmap alignments.
     *
     * @li ImageLeft: Display the pixmap left
     * @li ImageRight: Display the pixmap right (default)
     */
    enum ImageAlignment
    {
        ImageLeft, /**< Display the pixmap on the left */
        ImageRight /**< Display the pixmap on the right */
    };

    /**
     * Comment message types
     */
    enum MessageType
    {
        PlainMessage, /**< Normal comment */
        InfoMessage, /**< Information the user should be alerted to */
        WarningMessage, /**< A warning the user should be alerted to */
        ErrorMessage /**< An error message */
    };

public:

    /**
     * Constructs a title widget with the given @param parent.
     */
    explicit DConfigDlgTitle(QWidget* const parent = 0);

    virtual ~DConfigDlgTitle();

    /**
     * @param widget Widget displayed on the title widget.
     */
    void setWidget(QWidget* const widget);

    /**
     * @return the text displayed in the title
     * @see setText()
     */
    QString text() const;

    /**
     * @return the text displayed in the comment below the title, if any
     * @see setComment()
     */
    QString comment() const;

    /**
     * @return the pixmap displayed in the title
     * @see setPixmap()
     */
    const QPixmap* pixmap() const;

    /**
     * Sets this label's buddy to buddy.
     * When the user presses the shortcut key indicated by the label in this
     * title widget, the keyboard focus is transferred to the label's buddy
     * widget.
     * @param buddy the widget to activate when the shortcut key is activated
     */
    void setBuddy(QWidget* const buddy);

    /**
     * Get the current timeout value in milliseconds
     * @return timeout value in msecs
     */
    int autoHideTimeout() const;

public Q_SLOTS:

    /**
     * @param text Text displayed on the label. It can either be plain text or rich text. If it
     * is plain text, the text is displayed as a bold title text.
     * @param alignment Alignment of the text. Default is left and vertical centered.
     * @see text()
     */
    void setText(const QString& text, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter);
    /**
     * @param text Text displayed on the label. It can either be plain text or rich text. If it
     * is plain text, the text is displayed as a bold title text.
     * @param type The sort of message it is; will also set the icon accordingly @see MessageType
     * @see text()
     */
    void setText(const QString& text, MessageType type);

    /**
     * @param comment Text displayed beneath the main title as a comment.
     *                It can either be plain text or rich text.
     * @param type The sort of message it is. @see MessageType
     * @see comment()
     */
    void setComment(const QString& comment, MessageType type = PlainMessage);

    /**
     * @param pixmap Pixmap displayed in the header. The pixmap is by default right, but
     * @param alignment can be used to display it also left.
     * @see pixmap()
     */
    void setPixmap(const QPixmap& pixmap, ImageAlignment alignment = ImageRight);

    /**
     * @param icon name of the icon to display in the header. The pixmap is by default right, but
     * @param alignment can be used to display it also left.
     * @see pixmap()
     */
    void setPixmap(const QString& icon, ImageAlignment alignment = ImageRight);

    /**
     * @param pixmap the icon to display in the header. The pixmap is by default right, but
     * @param alignment can be used to display it also left.
     * @see pixmap()
     */
    void setPixmap(const QIcon& icon, ImageAlignment alignment = ImageRight);

    /**
     * @param pixmap the icon to display in the header. The pixmap is by default right, but
     * @param alignment can be used to display it also left.
     * @see pixmap()
     */
    void setPixmap(MessageType type, ImageAlignment alignment = ImageRight);

    /**
     * Set the autohide timeout of the label
     * Set value to 0 to disable autohide, which is the default.
     * @param msecs timeout value in milliseconds
     */
    void setAutoHideTimeout(int msecs);

protected:

    void changeEvent(QEvent*)           Q_DECL_OVERRIDE;
    void showEvent(QShowEvent*)         Q_DECL_OVERRIDE;
    bool eventFilter(QObject*, QEvent*) Q_DECL_OVERRIDE;

private:

    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void _k_timeoutFinished())
    Q_DISABLE_COPY(DConfigDlgTitle)
};

}  // namespace Digikam

#endif // DBCONFIGDLG_WIDGETS_H
