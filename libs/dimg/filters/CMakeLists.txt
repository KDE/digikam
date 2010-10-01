INSTALL(FILES dimgthreadedfilter.h 
              fx/blurfilter.h 
              cb/cbfilter.h 
              cb/cbsettings.h 
              curves/imagecurves.h
              bcg/bcgfilter.h 
              bcg/bcgsettings.h 
              hsl/hslfilter.h 
              icc/icctransform.h 
              levels/imagelevels.h
              nr/nrfilter.h 
              nr/nrsettings.h
              sharp/sharpenfilter.h 
              sharp/unsharpmaskfilter.h
              sharp/refocusfilter.h 
         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam COMPONENT Devel)

IF(GLIB2_FOUND)
    SUBDIRS(lens)
ENDIF(GLIB2_FOUND)

