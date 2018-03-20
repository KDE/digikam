/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 1999-2001 by Mirko Boehm <mirko at kde dot org>
 * Copyright (C) 1999-2001 by Espen Sand <espen at kde dot org>
 * Copyright (C) 1999-2001 by Holger Freyther <freyther at kde dot org>
 * Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde dot org>
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

#ifndef DBCONFIGDLG_DIALOG_H
#define DBCONFIGDLG_DIALOG_H

// Qt includes

#include <QDialog>
#include <QDialogButtonBox>

// Local includes

#include "dconfigdlgwidgets.h"
#include "digikam_export.h"

namespace Digikam
{

class DConfigDlgPrivate;

/**
 * @short A dialog base class which can handle multiple pages.
 *
 * This class provides a dialog base class which handles multiple
 * pages and allows the user to switch between these pages in
 * different ways.
 *
 * Currently, @p Auto, @p Plain, @p List, @p Tree and @p Tabbed face
 * types are available (@see DConfigDlgView).
 *
 */
class DIGIKAM_EXPORT DConfigDlg : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DConfigDlg)

public:

    /**
     *  @li @p Auto   - A dialog with a face based on the structure of the
     *                  available pages.
     *                  If only a single page is added, the dialog behaves like
     *                  in @p Plain mode, with multiple pages without sub pages
     *                  it behaves like in @p List mode and like in @p Tree mode
     *                  otherwise.
     *  @li @p Plain  - A normal dialog.
     *  @li @p List   - A dialog with an icon list on the left side and a
     *                  representation of the contents on the right side.
     *  @li @p Tree   - A dialog with a tree on the left side and a
     *                  representation of the contents on the right side.
     *  @li @p Tabbed - A dialog with a tab bar above the representation
     *                  of the contents.
     */
    enum FaceType
    {
        Auto   = DConfigDlgView::Auto,
        Plain  = DConfigDlgView::Plain,
        List   = DConfigDlgView::List,
        Tree   = DConfigDlgView::Tree,
        Tabbed = DConfigDlgView::Tabbed
    };

public:

    /**
     * Creates a new page dialog.
     */
    explicit DConfigDlg(QWidget* parent = 0, Qt::WindowFlags flags = 0);

    /**
     * Destroys the page dialog.
     */
    ~DConfigDlg();

    /**
     * Sets the face type of the dialog.
     */
    void setFaceType(FaceType faceType);

    /**
     * Adds a new top level page to the dialog.
     *
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated @see DConfigDlgWdgItem.
     */
    DConfigDlgWdgItem* addPage(QWidget* widget, const QString& name);

    /**
     * Adds a new top level page to the dialog.
     *
     * @param item The @see DConfigDlgWdgItem which describes the page.
     */
    void addPage(DConfigDlgWdgItem* item);

    /**
     * Inserts a new page in the dialog.
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
     * Inserts a new page in the dialog.
     *
     * @param before The new page will be insert before this @see DConfigDlgWdgItem
     *               on the same level in hierarchy.
     *
     * @param item The @see DConfigDlgWdgItem which describes the page.
     */
    void insertPage(DConfigDlgWdgItem* before, DConfigDlgWdgItem* item);

    /**
     * Inserts a new sub page in the dialog.
     *
     * @param parent The new page will be insert as child of this @see DConfigDlgWdgItem.
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated @see DConfigDlgWdgItem.
     */
    DConfigDlgWdgItem* addSubPage(DConfigDlgWdgItem* parent, QWidget* widget, const QString& name);

    /**
     * Inserts a new sub page in the dialog.
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

    /**
     * Sets the collection of standard buttons displayed by this dialog.
     */
    void setStandardButtons(QDialogButtonBox::StandardButtons buttons);

    /**
     * Returns the QPushButton corresponding to the standard button which, or 0 if the standard
     * button doesn't exist in this dialog.
     */
    QPushButton* button(QDialogButtonBox::StandardButton which) const;

    /**
      * Set an action button.
      */
    void addActionButton(QAbstractButton* button);

Q_SIGNALS:

    /**
     * This signal is emitted whenever the current page has changed.
     *
     * @param item The new current page or 0 if no current page is available.
     */
    void currentPageChanged(DConfigDlgWdgItem* current, DConfigDlgWdgItem* before);

    /**
     * This signal is emitted whenever a page has been removed.
     *
     * @param page The page which has been removed
     **/
    void pageRemoved(DConfigDlgWdgItem* page);

protected:

    /**
     * This constructor can be used by subclasses to provide a custom page widget.
     *
     * \param widget The DConfigDlgWdg object will be reparented to this object, so you can create
     * it without parent and you are not allowed to delete it.
     */
    DConfigDlg(DConfigDlgWdg* widget, QWidget* parent, Qt::WindowFlags flags = 0);
    DConfigDlg(DConfigDlgPrivate& dd, DConfigDlgWdg* widget, QWidget* parent, Qt::WindowFlags flags = 0);

    /**
     * Returns the page widget of the dialog or 0 if no page widget is set.
     */
    DConfigDlgWdg* pageWidget();

    /**
     * Returns the page widget of the dialog or 0 if no page widget is set.
     */
    const DConfigDlgWdg* pageWidget() const;

    /**
     * Set the page widget of the dialog.
     *
     * @note the previous pageWidget will be deleted.
     *
     * @param widget The DConfigDlgWdg object will be reparented to this object, so you can create
     * it without parent and you are not allowed to delete it.
     */
    void setPageWidget(DConfigDlgWdg* widget);

    /**
     * Returns the button box of the dialog or 0 if no button box is set.
     */
    QDialogButtonBox* buttonBox();

    /**
     * Returns the button box of the dialog or 0 if no button box is set.
     */
    const QDialogButtonBox* buttonBox() const;

    /**
     * Set the button box of the dialog
     *
     * @note the previous buttonBox will be deleted.
     *
     * @param box The QDialogButtonBox object will be reparented to this object, so you can create
     * it without parent and you are not allowed to delete it.
     */
    void setButtonBox(QDialogButtonBox* box);

protected:

    DConfigDlgPrivate* const d_ptr;
};

}  // namespace Digikam

#endif // DBCONFIGDLG_DIALOG_H
