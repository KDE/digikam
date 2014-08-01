/*

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
#ifndef BALOOWRAP_H
#define BALOOWRAP_H

// Qt
#include <QObject>
// KDE
#include <kurl.h>
// Local


class KJob;
class KUrl;

namespace Digikam
{

/**
 * A real metadata backend using Baloo to store and retrieve metadata.
 */
class  BalooWrap : public QObject
{
    Q_OBJECT
public:
    BalooWrap(QObject* parent = 0);
    ~BalooWrap();

    QStringList getTags(KUrl& url);

    QString getComment(KUrl& url);

    int getRating(KUrl& url);

    void setTags(KUrl& url, QStringList* tags);

    void setComment(KUrl& url, QString* comment);

    void setRating(KUrl& url, int rating);

    void setAllData(KUrl& url, QStringList *tags, QString *comment, int rating);

//    virtual TagSet allTags() const;

//    virtual void refreshAllTags();

//    virtual void storeSemanticInfo(const KUrl&, const SemanticInfo&);

//    virtual void retrieveSemanticInfo(const KUrl&);

//    virtual QString labelForTag(const SemanticInfoTag&) const;

//    virtual SemanticInfoTag tagForLabel(const QString&);

private Q_SLOTS:
    void slotFetchFinished(KJob* job);

private:
    class Private;
    Private* const d;
};

} // namespace

#endif /* BALOOWRAP_H */
