# - Try to find libpgf
# Once done this will define
#
#  PGF_FOUND - system has libgf
#  PGF_INCLUDE_DIRS - the libpgf include directory
#  PGF_LIBRARIES - Link these to use libpgf

# PKG-CONFIG is required.
INCLUDE(FindPkgConfig REQUIRED)

IF(PKG_CONFIG_FOUND)
	INCLUDE(FindPkgConfig)

	PKG_CHECK_MODULES(PGF libpgf)
ENDIF(PKG_CONFIG_FOUND)
