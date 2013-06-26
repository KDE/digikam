#include "tagsmanager.h"
#include <kdebug.h>

TagsManager::TagsManager()
    : KDialog(0)
{
    this->resize(800,600);
    //this->show();
    kDebug() << "My new Tags Manager class";
}

TagsManager::~TagsManager()
{
}
