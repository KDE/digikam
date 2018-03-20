/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Matthias Kretz <kretz at kde dot org>
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

#ifndef DBCONFIGDLG_MODELS_H
#define DBCONFIGDLG_MODELS_H

// Qt includes

#include <QAbstractItemModel>
#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DConfigDlgModelPrivate;

/**
 *  @short A base class for a model used by DConfigDlgView.
 *
 *  This class is an abstract base class which must be used to
 *  implement custom models for DConfigDlgView. Additional to the standard
 *  Qt::ItemDataRoles it provides the two roles
 *
 *    @li HeaderRole
 *    @li WidgetRole
 *
 *  which are used to return a header string for a page and a QWidget
 *  pointer to the page itself.
 */
class DConfigDlgModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DConfigDlgModel)

public:

    /**
     * Additional roles that DConfigDlgView uses.
     */
    enum Role
    {
        /**
         * A string to be rendered as page header.
         */
        HeaderRole = Qt::UserRole + 1,

        /**
         * A pointer to the page widget.
         * This is the widget that is shown when the item is selected.
         */
        WidgetRole
    };

    /**
     * Constructs a page model with the given parent.
     */
    explicit DConfigDlgModel(QObject* const parent = 0);

    /**
     * Destroys the page model.
     */
    virtual ~DConfigDlgModel();

protected:

    DConfigDlgModel(DConfigDlgModelPrivate& dd, QObject* const parent);

protected:

    DConfigDlgModelPrivate* const d_ptr;
};

// --------------------------------------------------------------------------

/**
 * DConfigDlgWdgItem is used by @ref DConfigDlgWdg and represents
 * a page.
 */
class DIGIKAM_EXPORT DConfigDlgWdgItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString header READ header WRITE setHeader)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    /**
     * This property holds whether the item is enabled.
     * It dis-/enables both the widget and the item in the list-/treeview.
     */
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)

public:

    /**
     * Creates a new page widget item.
     *
     * @param widget The widget that is shown as page in the DConfigDlgWdg.
     */
    DConfigDlgWdgItem(QWidget* widget);

    /**
     * Creates a new page widget item.
     *
     * @param widget The widget that is shown as page in the DConfigDlgWdg.
     * @param name The localized string that is show in the navigation view
     *             of the DConfigDlgWdg.
     */
    DConfigDlgWdgItem(QWidget* widget, const QString& name);

    /**
     * Destroys the page widget item.
     */
    ~DConfigDlgWdgItem();

    /**
     * Returns the widget of the page widget item.
     */
    QWidget* widget() const;

    /**
     * Sets the name of the item as shown in the navigation view of the page
     * widget.
     */
    void setName(const QString& name);

    /**
     * Returns the name of the page widget item.
     */
    QString name() const;

    /**
     * Sets the header of the page widget item.
     *
     * If setHeader(QString()) is used, what is the default if the header
     * does not got set explicit, then the defined name() will also be used
     * for the header. If setHeader("") is used, the header will be hidden
     * even if the @a DConfigDlgView::FaceType is something else then Tabbed.
     *
     * @param header Header of the page widget item.
     */
    void setHeader(const QString& header);

    /**
     * Returns the header of the page widget item.
     */
    QString header() const;

    /**
     * Sets the icon of the page widget item.
     * @param icon Icon of the page widget item.
     */
    void setIcon(const QIcon& icon);

    /**
     * Returns the icon of the page widget item.
     */
    QIcon icon() const;

    /**
     * Sets whether the page widget item is checkable in the view.
     * @param checkable True if the page widget is checkable,
     *                  otherwise false.
     */
    void setCheckable(bool checkable);

    /**
     * Returns whether the page widget item is checkable.
     */
    bool isCheckable() const;

    /**
     * Returns whether the page widget item is checked.
     */
    bool isChecked() const;

    /**
     * Returns whether the page widget item is enabled.
     */
    bool isEnabled() const;

public Q_SLOTS:

    /**
     * Sets whether the page widget item is enabled.
     */
    void setEnabled(bool);

    /**
     * Sets whether the page widget item is checked.
     */
    void setChecked(bool checked);

Q_SIGNALS:

    /**
     * This signal is emitted whenever the icon or header
     * is changed.
     */
    void changed();

    /**
     * This signal is emitted whenever the user checks or
     * unchecks the item of @see setChecked() is called.
     */
    void toggled(bool checked);

private:

    class Private;
    Private* const d;
};

// --------------------------------------------------------------------------

class DConfigDlgWdgModelPrivate;

/**
 * This page model is used by @see DConfigDlgWdg to provide
 * a hierarchical layout of pages.
 */
class DConfigDlgWdgModel : public DConfigDlgModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DConfigDlgWdgModel)

public:

    /**
     * Creates a new page widget model.
     *
     * @param parent The parent object.
     */
    explicit DConfigDlgWdgModel(QObject* parent = 0);

    /**
     * Destroys the page widget model.
     */
    ~DConfigDlgWdgModel();

    /**
     * Adds a new top level page to the model.
     *
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated @see DConfigDlgWdgItem.
     */
    DConfigDlgWdgItem* addPage(QWidget* widget, const QString& name);

    /**
     * Adds a new top level page to the model.
     *
     * @param item The @see DConfigDlgWdgItem which describes the page.
     */
    void addPage(DConfigDlgWdgItem* item);

    /**
     * Inserts a new page in the model.
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
     * Inserts a new page in the model.
     *
     * @param before The new page will be insert before this @see DConfigDlgWdgItem
     *               on the same level in hierarchy.
     *
     * @param item The @see DConfigDlgWdgItem which describes the page.
     */
    void insertPage(DConfigDlgWdgItem* before, DConfigDlgWdgItem* item);

    /**
     * Inserts a new sub page in the model.
     *
     * @param parent The new page will be insert as child of this @see DConfigDlgWdgItem.
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated @see DConfigDlgWdgItem.
     */
    DConfigDlgWdgItem* addSubPage(DConfigDlgWdgItem* parent, QWidget* widget, const QString& name);

    /**
     * Inserts a new sub page in the model.
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
     * These methods are reimplemented from QAbstractItemModel.
     */
    int columnCount(const QModelIndex& parent = QModelIndex()) const                        Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const               Q_DECL_OVERRIDE;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)  Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex& index) const                                     Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex& index) const                                      Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent = QModelIndex()) const                           Q_DECL_OVERRIDE;

    /**
     * Returns the @see DConfigDlgWdgItem for a given index or 0 if the index is invalid.
     */
    DConfigDlgWdgItem* item(const QModelIndex& index) const;

    /**
     * Returns the index for a given @see DConfigDlgWdgItem. The index is invalid if the
     * item can't be found in the model.
     */
    QModelIndex index(const DConfigDlgWdgItem* item) const;

Q_SIGNALS:

    /**
     * This signal is emitted whenever a checkable page changes its state. @param checked is true
     * when the @param page is checked, or false if the @param page is unchecked.
     */
    void toggled(DConfigDlgWdgItem* page, bool checked);

private:

    Q_PRIVATE_SLOT(d_func(), void _k_itemChanged())
    Q_PRIVATE_SLOT(d_func(), void _k_itemToggled(bool))
};

}  // namespace Digikam

#endif // DBCONFIGDLG_MODELS_H
