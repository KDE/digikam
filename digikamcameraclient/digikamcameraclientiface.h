#ifndef DIGIKAMCAMERACLIENTIFACE_H
#define DIGIKAMCAMERACLIENTIFACE_H

#include <dcopobject.h>

class DigikamCameraClientIface : virtual public DCOPObject
{
    K_DCOP

public:

k_dcop:

    virtual ASYNC cameraOpen(QString libraryPath, QString downloadAlbum,
                             QString title, QString model,
                             QString port,  QString path) = 0;

    virtual ASYNC cameraConnect() = 0;

    virtual ASYNC cameraChangeLibraryPath(QString libraryPath) = 0;

    virtual ASYNC cameraChangeDownloadAlbum(QString album) = 0;

    virtual ASYNC cameraDownloadSelected() = 0;

    virtual void  close() = 0;

};

#endif // DIGIKAMCAMERACLIENTIFACE_H
