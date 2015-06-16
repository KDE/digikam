#ifndef IOJOB_H
#define IOJOB_H

#include "KDCRAW/RActionJob"
#include "QUrl"
#include "album.h"

using namespace KDcrawIface;

namespace Digikam
{

class IOJob : public RActionJob
{
    Q_OBJECT

public:

    IOJob();
};

// ---------------------------------------

class CopyJob : public IOJob
{
    Q_OBJECT

public:

    enum OperationType
    {
        Copy = 0,
        Move
    };

    CopyJob(const PAlbum *dest, OperationType type);

protected:
    const PAlbum  *m_dest;
    OperationType  m_type;
};

// -----------

class CopyFileJob : public CopyJob
{
    Q_OBJECT

public:

    CopyFileJob(const QUrl &src, const PAlbum *dest, OperationType type);

protected:

    void run();

private:

    QUrl m_srcFile;
};

// -----------

class CopyAlbumJob : public CopyJob
{
    Q_OBJECT

public:

    CopyAlbumJob(const PAlbum *src, const PAlbum *dest, OperationType type);

protected:

    void run();

private:

    const PAlbum *m_srcAlbum;
};

// ---------------------------------------

class DeleteJob : public IOJob
{
    Q_OBJECT

public:

    DeleteJob(bool isPermanentDeletion);

protected:

    bool m_isPermanentDeletion;
};

// -----------

class DeleteFileJob : public DeleteJob
{
    Q_OBJECT

public:

    DeleteFileJob(const QUrl &srcToDelete, bool isPermanentDeletion);

protected:

    void run();

private:

    QUrl m_srcToDelete;
};

// -----------

class DeleteAlbumJob : public DeleteJob
{
    Q_OBJECT

public:

    DeleteAlbumJob(const PAlbum *album, bool isPermanentDeletion);

protected:

    void run();

private:

    const PAlbum *m_albumToDelete;
};

// ---------------------------------------

//class RenameJob : public FileSystemJob
//{

//};

// ---------------------------------------

//class StatJob : public FileSystemJob
//{
//    Q_OBJECT

//public:

//    StatJob();

//protected:

//    void run();
//};

// ---------------------------------------

//class MkdirJob : public FileSystemJob
//{
//    Q_OBJECT

//public:

//    MkdirJob();
//};


} // namespace Digikam

#endif // IOJOB_H
