INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata
                   )

SET(libcommonwidgets_STAT_SRCS colorgradientwidget.cpp 
                               dcursortracker.cpp 
                               filesaveoptionsbox.cpp 
                               statuszoombar.cpp 
                               statusprogressbar.cpp 
                               statusnavigatebar.cpp 
                               sidebar.cpp 
                               dpopupmenu.cpp 
                               squeezedcombobox.cpp 
                               histogramwidget.cpp 
                               curveswidget.cpp 
                               paniconwidget.cpp 
                               previewwidget.cpp 
   )

KDE4_AUTOMOC(${libcommonwidgets_STAT_SRCS})

KDE4_ADD_LIBRARY(libcommonwidgets STATIC ${libcommonwidgets_STAT_SRCS})

install( FILES histogramwidget.h 
               colorgradientwidget.h 
               curveswidget.h 
               sidebar.h
               squeezedcombobox.h 
               dpopupmenu.h 
               statuszoombar.h 
               statusnavigatebar.h
               statusprogressbar.h 
               dcursortracker.h 
               paniconwidget.h 
               previewwidget.h  
         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )

