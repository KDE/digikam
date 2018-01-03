/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : A widget to select Physical or virtual albums with combo-box
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ALBUMSELECTORS_H
#define ALBUMSELECTORS_H

// Qt includes

#include <QWidget>

namespace Digikam
{

class Album;
typedef QList<Album*> AlbumList;

class AlbumSelectors : public QWidget
{
    Q_OBJECT

public:

    enum AlbumType
    {
        PhysAlbum=0,
        TagsAlbum,
        All
    };

public:

    /** Default Contructor. 'label' is front text of label which title widget. 'configName' is name used to store
     *  Albums configuration in settings file. 'parent' is parent widget.
     */
    explicit AlbumSelectors(const QString& label, const QString& configName, QWidget* const parent = 0, AlbumType albumType = All);
    ~AlbumSelectors();

    /** Return list of selected physical albums
     */
    AlbumList selectedAlbums() const;

    /** Return list of selected physical album ids
     */
    QList<int> selectedAlbumIds() const;

    /** Return list of selected tag albums
     */
    AlbumList selectedTags() const;

    /** Return list of selected tag album ids
     */
    QList<int> selectedTagIds() const;

    /** Return list of selected physical and tag albums.
     */
    AlbumList selectedAlbumsAndTags() const;

    /** Reset all Physical and Tag Albums selection.
     */
    void resetSelection();

    /** Select Physical Album from list. If singleSelection is true, only this one is
     *  selected from tree-view and all others are deselected.
     */
    void setAlbumSelected(Album* const album, bool singleSelection=true);

    /** Select Tag Album from list. If singleSelection is true, only this one is
     *  selected from tree-view and all others are deselected.
     */
    void setTagSelected(Album* const album, bool singleSelection=true);
    
    /**
     * Sets the search type selection with the AlbumType.
     **/
    void setTypeSelection(int albumType);

    /**
     * Returns the selected album type.
     **/
    int typeSelection() const;

    /** Return true if whole Albums collection option is checked.
     */
    bool wholeAlbumsChecked() const;

    /** Return true if whole Tags collection option is checked.
     */
    bool wholeTagsChecked() const;

public Q_SLOTS:

    /** Called in constructor. Restore previous settings saved in configuration file.
    */
    void loadState();

    /** Save settings in configuration file. Must be called explicitly by host implementation.
     */
    void saveState();

Q_SIGNALS:

    void signalSelectionChanged();

private Q_SLOTS:

    void slotUpdateClearButtons();
    void slotWholeAlbums(bool);
    void slotWholeTags(bool);

private:

    void initAlbumWidget();
    void initTagWidget();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ALBUMSELECTORS_H
