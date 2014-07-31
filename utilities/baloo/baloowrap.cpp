
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
// Self
#include "baloowrap.h"


// Qt
#include <QStringList>

// KDE
#include <KDebug>
#include <KUrl>

// Baloo
#include <baloo/file.h>
#include <baloo/filefetchjob.h>
#include <baloo/filemodifyjob.h>
#include <baloo/taglistjob.h>

namespace Digikam
{

struct BalooWrap::Private
{
    int dummy;
//    TagSet mAllTags;
};

BalooWrap::BalooWrap(QObject* parent)
: QObject(parent)
, d(new BalooWrap::Private)
{
}

BalooWrap::~BalooWrap()
{
    delete d;
}

//TagSet BalooWrap::allTags() const
//{
//    if (d->mAllTags.empty()) {
//        const_cast<BalooWrap*>(this)->refreshAllTags();
//    }
//    return d->mAllTags;
//}

//void BalooWrap::refreshAllTags()
//{
//    Baloo::TagListJob* job = new Baloo::TagListJob();
//    job->exec();

//    d->mAllTags.clear();
//    Q_FOREACH(const QString& tag, job->tags()) {
//        d->mAllTags << tag;
//    }
//}

//void BalooWrap::storeSemanticInfo(const KUrl& url, const SemanticInfo& semanticInfo)
//{
//    Baloo::File file(url.toLocalFile());
//    file.setRating(semanticInfo.mRating);
//    file.setUserComment(semanticInfo.mDescription);
//    file.setTags(semanticInfo.mTags.toList());

//    Baloo::FileModifyJob* job = new Baloo::FileModifyJob(file);
//    job->start();
//}

//void BalooWrap::retrieveSemanticInfo(const KUrl& url)
//{
//    Baloo::FileFetchJob* job = new Baloo::FileFetchJob(url.toLocalFile());
//    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotFetchFinished(KJob*)));

//    job->start();
//}

void BalooWrap::slotFetchFinished(KJob* job)
{
    Baloo::FileFetchJob* fjob = static_cast<Baloo::FileFetchJob*>(job);
    Baloo::File file = fjob->file();

//    SemanticInfo si;
//    si.mRating = file.rating();
//    si.mDescription = file.userComment();
//    si.mTags = file.tags().toSet();

//    emit semanticInfoRetrieved(KUrl::fromLocalFile(file.url()), si);
}

//QString BalooWrap::labelForTag(const SemanticInfoTag& uriString) const
//{
//    return uriString;
//}

//SemanticInfoTag BalooWrap::tagForLabel(const QString& label)
//{
//    return label;
//}

} // namespace
