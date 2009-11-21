/*
 * TagModificationHelper.h
 *
 *  Created on: 21.11.2009
 *      Author: languitar
 */

#ifndef TAGMODIFICATIONHELPER_H
#define TAGMODIFICATIONHELPER_H

// Qt includes
#include <qobject.h>
#include <qstring.h>

// Local includes
#include "album.h"

namespace Digikam
{

class TagModificationHelperPriv;
class TagModificationHelper: public QObject
{
    Q_OBJECT
public:
    TagModificationHelper(QObject *parent, QWidget *dialogParent);
    virtual ~TagModificationHelper();

public Q_SLOTS:

    /**
     * Creates one ore more new tags under the given parent. If only the parent
     * is given, then a dialog is shown to create new tags. Else, if also a
     * title and optionally an icon are given, then these values will be
     * suggested in the dialog.
     *
     * @param parent parent tag album under which to create the new tags. May be
     *               0 to use the root album
     * @param title if this isn't an empty string, then this tag name is
     *              suggested
     * @param iconName an optional name for the icon to suggest for the new tag
     */
    void slotTagNew(TAlbum *parent, const QString &title = QString(),
                    const QString &iconName = QString());

    /**
     * Edits the given tag via a user dialog.
     *
     * @param tag the tag to change
     */
    void slotTagEdit(TAlbum *tag);

    /**
     * Deletes the given tag and after prompting the user for this.
     *
     * @param tag the tag to delete, must not be the root tag album
     */
    void slotTagDelete(TAlbum *tag);

    /**
     * Assigns the given tag to the list of images in the background using the
     * progress signals to indicate the progress.
     *
     * @param tagID id of the tag to assign
     * @param imageIDs list of images that the tag will be assigned to
     */
    // TODO why don't we use real domain objects here instead of ids?
    void slotAssignTags(int tagId, const QList<int>& imageIDs);

Q_SIGNALS:
    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);

private:
    TagModificationHelperPriv *d;

};

}

#endif /* TAGMODIFICATIONHELPER_H */
