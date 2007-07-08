
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor
                    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack
                    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/hsl
                    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/ratiocrop
                   )

SET(liblapack_SRCS 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/abort_.c
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/dgesv.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/dlaswp.c
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/endfile.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/idamax.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/open.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/sig_die.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/wref.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/close.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/dgetf2.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/dscal.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/err.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/ieeeck.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/s_cmp.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/s_stop.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/wrtfmt.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/dgemm.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/dgetrf.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/dswap.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/fmt.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/ilaenv.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/s_copy.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/wsfe.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/dger.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/dgetrs.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/dtrsm.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/fmtlib.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/lsame.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/sfe.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/util.c 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/clapack/xerbla.c
   )

SET(digikamimageplugin_core_PART_SRCS 
    ${liblapack_SRCS} 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/imageplugin_core.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/imageeffect_bcg.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/imageeffect_rgb.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/imageeffect_blur.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/imageeffect_autocorrection.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/imageeffect_redeye.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/imageeffect_iccproof.cpp 
#    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/imageeffect_bwsepia.cpp 
#    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/hsl/hspreviewwidget.cpp
#    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/hsl/imageeffect_hsl.cpp 
#    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/imageeffect_sharpen.cpp 
#    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/matrix.cpp
#    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/refocus.cpp
#    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor/unsharp.cpp
#    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/ratiocrop/imageselectionwidget.cpp
#    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/ratiocrop/imageeffect_ratiocrop.cpp 
   )

KDE4_AUTOMOC(${digikamimageplugin_core_PART_SRCS})

KDE4_ADD_PLUGIN(digikamimageplugin_core ${digikamimageplugin_core_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_core digikam)

INSTALL(TARGETS digikamimageplugin_core DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_core_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_core.desktop DESTINATION ${SERVICES_INSTALL_DIR})
