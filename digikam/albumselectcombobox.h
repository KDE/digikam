/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-09
 * Description : A combo box for selecting albums
 *
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef ALBUMSELECTCOMBOBOX_H
#define ALBUMSELECTCOMBOBOX_H

// Qt includes

// KDE includes

// Local includes

#include "comboboxutilities.h"

class QSortFilterProxyModel;

namespace Digikam
{

class AbstractCheckableAlbumModel;

class AlbumSelectComboBox : public TreeViewLineEditComboBox
{
    Q_OBJECT

public:

    AlbumSelectComboBox(QWidget *parent = 0);
    ~AlbumSelectComboBox();

    /** Once after creation, call one of these three methods.
        Use the first one if you want a standard combo box for PAlbums and
        the second one for tags, while the third allows you to provide
        custom source and filter models.
        The first two also set a default noSelectionText. Customize afterwards if required.
    */
    void setDefaultAlbumModels();
    void setDefaultTagModels();
    void setModels(AbstractCheckableAlbumModel *model, QSortFilterProxyModel *filterModel);

    /** Sets the text that is used to describe the state when no album is selected.
        This may be something like "Any album" or "No tag selected" */
    void setNoSelectionText(const QString& text);

    /** Returns the source model. Retrieve selection information from here. */
    AbstractCheckableAlbumModel *model() const;
    /** Return the filter model in use. */
    QSortFilterProxyModel *filterModel() const;

protected Q_SLOTS:

    /** Updates the text describing the selection ("3 Albums selected").
        Can be overridden to customize the default text. */
    virtual void updateText();

protected:

    AbstractCheckableAlbumModel *m_model;
    QSortFilterProxyModel       *m_filterModel;
    QString                      m_noSelectionText;
};

} // namespace Digikam

#endif /* ALBUMSELECTCOMBOBOX_H */

