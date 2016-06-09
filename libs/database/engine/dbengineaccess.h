#ifndef DATABASE_ENGINE_ACCESS_H
#define DATABASE_ENGINE_ACCESS_H

//Local includes

#include "digikam_export.h"
#include "dbengineparameters.h"
#include "dbengineerrorhandler.h"

namespace Digikam 
{

class InitializationObserver;
class DbEngineAccessStaticPriv;

class DIGIKAM_DATABASE_EXPORT DbEngineAccess
{
public:
    
    // Create a default DbEngineAccess object.
    
    explicit DbEngineAccess();
    virtual ~DbEngineAccess();
    
    virtual bool checkReadyForUse();
    
private:
    
    static DbEngineAccessStaticPriv* d;
};

} //namespace Digikam

#endif //DATABASE_ENGINE_ACCESS_H