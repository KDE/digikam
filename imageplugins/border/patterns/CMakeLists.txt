
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  pine-pattern.png wood-pattern.png paper-pattern.png parque-pattern.png  	              ice-pattern.png leaf-pattern.png marble-pattern.png rain-pattern.png  	              craters-pattern.png dried-pattern.png pink-pattern.png stone-pattern.png  	              chalk-pattern.png granit-pattern.png rock-pattern.png wall-pattern.png DESTINATION ${DATA_INSTALL_DIR}/digikam/data )



#original Makefile.am contents follow:

#borderpicsdir = $(kde_datadir)/digikam/data
#borderpics_DATA = pine-pattern.png wood-pattern.png paper-pattern.png parque-pattern.png \
#	              ice-pattern.png leaf-pattern.png marble-pattern.png rain-pattern.png \
#	              craters-pattern.png dried-pattern.png pink-pattern.png stone-pattern.png \
#	              chalk-pattern.png granit-pattern.png rock-pattern.png wall-pattern.png
