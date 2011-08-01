#ifndef CLONECONTAINER_H
#define CLONECONTAINER_H

#include"clonebrush.h"


namespace Digikam
{

//------------------------CloneContainer--------------------------------------
 class DIGIKAM_EXPORT CloneContainer
 {
 public:
      CloneContainer()
      {
          opacity = 100;
          selectMode = false;
          drawMode = false;


       };
        ~CloneContainer(){};
Public:

        CloneBrush brush;// selected brush
        int brushID;   //id in the brushMap of selected brush
        int brushDia;//diameter of the brush shape
        int mainDia;
        int opacity;
        bool selectMode; //set to true if the left button is clicked ,in this mode, click on the image to select a center point of the source area
        bool drawMode;   //set to true if the right button is clicked ,in this mode, when move the mouse, a stroke will be draw on the image

   };
}
#endif // CLONECONTAINER_H
