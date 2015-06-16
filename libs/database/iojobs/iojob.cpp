#include "iojob.h"
#include <QFile>
#include "digikam_debug.h"

namespace Digikam {

IOJob::IOJob()
{
}

// --------------------------------------------

CopyJob::CopyJob(const PAlbum *dest, CopyJob::OperationType type)
{
    m_dest = dest;
    m_type = type;
}

// --------------------------------------------

CopyFileJob::CopyFileJob(const QUrl &src, const PAlbum *dest, CopyJob::OperationType type)
    : CopyJob(dest, type)
{
    m_srcFile = src;
}

void CopyFileJob::run()
{
    // TODO
    emit signalDone();
}

// --------------------------------------------

CopyAlbumJob::CopyAlbumJob(const PAlbum *src, const PAlbum *dest, CopyJob::OperationType type)
    : CopyJob(dest, type)
{
    m_srcAlbum = src;
}

void CopyAlbumJob::run()
{
    // TODO
    emit signalDone();
}

// --------------------------------------------

DeleteJob::DeleteJob(bool isPermanentDeletion)
{
    m_isPermanentDeletion = isPermanentDeletion;
}

// --------------------------------------------

DeleteFileJob::DeleteFileJob(const QUrl &srcToDelete, bool isPermanentDeletion)
    : DeleteJob(isPermanentDeletion)
{
    m_srcToDelete = srcToDelete;
}

void DeleteFileJob::run()
{
    // TODO
    emit signalDone();
}

// --------------------------------------------

DeleteAlbumJob::DeleteAlbumJob(const PAlbum *album, bool isPermanentDeletion)
    : DeleteJob(isPermanentDeletion)
{
    m_albumToDelete = album;
}

void DeleteAlbumJob::run()
{
    // TODO
    emit signalDone();
}

} // namespace Digikam
