#include "iojobsthread.h"
#include "iojob.h"

namespace Digikam
{

IOJobsThread::IOJobsThread(QObject *const parent)
    : RActionThreadBase(parent)
{
}

void IOJobsThread::copyPAlbum(const PAlbum *srcAlbum, const PAlbum *destAlbum, const CopyJob::OperationType opType)
{
    RJobCollection collection;
    CopyAlbumJob *j = new CopyAlbumJob(srcAlbum, destAlbum, opType);

    // TODO: Create a connection here for progress or whatever feedback

    collection.insert(j, 0);
    appendJobs(collection);
}

void IOJobsThread::copyFiles(const QList<QUrl> &srcFiles, const PAlbum *destAlbum, const CopyJob::OperationType opType)
{
    RJobCollection collection;

    foreach (const QUrl &url, srcFiles)
    {
        CopyFileJob *j = new CopyFileJob(url, destAlbum, opType);

        // TODO: Create a connection here for progress or whatever feedback

        collection.insert(j, 0);
    }

    appendJobs(collection);
}

void IOJobsThread::deletePAlbum(const PAlbum *albumToDelete, bool isPermanentDeletion)
{
    RJobCollection collection;
    DeleteAlbumJob *j = new DeleteAlbumJob(albumToDelete, isPermanentDeletion);

    // TODO: Create a connection here for progress or whatever feedback

    collection.insert(j, 0);
    appendJobs(collection);
}

void IOJobsThread::deleteFiles(const QList<QUrl> &srcsToDelete, bool isPermanentDeletion)
{
    RJobCollection collection;

    foreach (const QUrl &url, srcsToDelete)
    {
        DeleteFileJob *j = new DeleteFileJob(url, isPermanentDeletion);

        // TODO: Create a connection here for progress or whatever feedback

        collection.insert(j, 0);
    }

    appendJobs(collection);
}

} // namespace Digikam
