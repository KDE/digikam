#include "iojobsmanager.h"
namespace Digikam {

class IOJobsManagerCreator
{
public:

    IOJobsManager object;
};

Q_GLOBAL_STATIC(IOJobsManagerCreator, creator)

// ----------------------------------------------

IOJobsManager::IOJobsManager()
{
}

IOJobsManager *IOJobsManager::instance()
{
    return &creator->object;
}

IOJobsThread *IOJobsManager::startCopyJob(const PAlbum *srcAlbum, const PAlbum *destAlbum, const CopyJob::OperationType opType)
{
    IOJobsThread *thread = new IOJobsThread(this);
    thread->copyPAlbum(srcAlbum, destAlbum, opType);
    thread->start();

    return thread;
}

IOJobsThread *IOJobsManager::startCopyJob(const QList<QUrl> &srcsList, const PAlbum *destAlbum, const CopyJob::OperationType opType)
{
    IOJobsThread *thread = new IOJobsThread(this);
    thread->copyFiles(srcsList, destAlbum, opType);
    thread->start();

    return thread;
}

IOJobsThread *IOJobsManager::startDeleteJob(const PAlbum *albumToDelete, bool useTrash)
{
    IOJobsThread *thread = new IOJobsThread(this);
    thread->deletePAlbum(albumToDelete, useTrash);
    thread->start();

    return thread;
}

IOJobsThread *IOJobsManager::startDeleteJob(const QList<ImageInfo> &filesToDelete, bool useTrash)
{
    IOJobsThread *thread = new IOJobsThread(this);
    thread->deleteFiles(filesToDelete, useTrash);
    thread->start();

    return thread;
}

} // namespace Digikam
