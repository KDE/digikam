
INCLUDE_DIRECTORIES(../../imageplugins/coreplugin/sharpnesseditor
                    ../../imageplugins/coreplugin/sharpnesseditor/clapack
                    ../../imageplugins/coreplugin/hsl
                    ../../imageplugins/coreplugin/ratiocrop
                   )

SET(libclapack_SRCS 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/abort_.c
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/dgesv.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/dlaswp.c
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/endfile.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/idamax.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/open.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/sig_die.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/wref.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/close.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/dgetf2.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/dscal.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/err.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/ieeeck.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/s_cmp.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/s_stop.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/wrtfmt.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/dgemm.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/dgetrf.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/dswap.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/fmt.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/ilaenv.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/s_copy.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/wsfe.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/dger.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/dgetrs.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/dtrsm.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/fmtlib.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/lsame.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/sfe.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/util.c 
    ../../imageplugins/coreplugin/sharpnesseditor/clapack/xerbla.c
   )

# Disable Clapack C code warnings.
FOREACH(_curentfile ${libclapack_SRCS})
    SET_SOURCE_FILES_PROPERTIES(${_curentfile} PROPERTIES COMPILE_FLAGS -w )
ENDFOREACH(_curentfile ${libclapack_SRCS})

SET(digikamimageplugin_core_PART_SRCS 
    ${libclapack_SRCS} 
    ../../imageplugins/coreplugin/imageplugin_core.cpp 
    ../../imageplugins/coreplugin/imageeffect_bcg.cpp 
    ../../imageplugins/coreplugin/imageeffect_rgb.cpp 
    ../../imageplugins/coreplugin/imageeffect_blur.cpp 
    ../../imageplugins/coreplugin/imageeffect_autocorrection.cpp 
    ../../imageplugins/coreplugin/imageeffect_redeye.cpp 
    ../../imageplugins/coreplugin/imageeffect_iccproof.cpp 
    ../../imageplugins/coreplugin/imageeffect_bwsepia.cpp 
    ../../imageplugins/coreplugin/hsl/hspreviewwidget.cpp
    ../../imageplugins/coreplugin/hsl/imageeffect_hsl.cpp 
    ../../imageplugins/coreplugin/sharpnesseditor/matrix.cpp
    ../../imageplugins/coreplugin/sharpnesseditor/refocus.cpp
    ../../imageplugins/coreplugin/sharpnesseditor/unsharp.cpp
    ../../imageplugins/coreplugin/sharpnesseditor/imageeffect_sharpen.cpp 
    ../../imageplugins/coreplugin/ratiocrop/imageselectionwidget.cpp
    ../../imageplugins/coreplugin/ratiocrop/imageeffect_ratiocrop.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_core ${digikamimageplugin_core_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_core digikam)

INSTALL(TARGETS digikamimageplugin_core DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_core_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_core.desktop DESTINATION ${SERVICES_INSTALL_DIR})
