#-*-cmake-*-
#
# Once loaded this will define
#  LIBOPTIMSOCHOST_FOUND        - system has liboptimsochost
#  LIBOPTIMSOCHOST_INCLUDE_DIR  - include directory for liboptimsochost
#  LIBOPTIMSOCHOST_LIBRARIES    - libraries you need to link to
#

SET(LIBOPTIMSOCHOST_FOUND   "NO" )

FIND_PATH( LIBOPTIMSOCHOST_INCLUDE_DIR liboptimsochost.h
  "$ENV{LIBOPTIMSOCHOST_LOCATION}/src/optimsochost"
  "$ENV{OPTIMSOC}/src/sw/host/liboptimsochost/src/optimsochost"
  "${PROJECT_SOURCE_DIR}/../liboptimsochost/src/optimsochost"
  /usr/include/optimsochost
  )

FIND_LIBRARY( LIBOPTIMSOCHOST_LIBRARY
  NAMES optimsochost
  PATHS
  "$ENV{LIBOPTIMSOCHOST_LOCATION}/src/.libs"
  "$ENV{OPTIMSOC}/src/sw/host/liboptimsochost/lib"
  "${PROJECT_SOURCE_DIR}/../liboptimsochost/lib"
  DOC   "liboptimsochost library"
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBOPTIMSOCHOST DEFAULT_MSG LIBOPTIMSOCHOST_LIBRARY LIBOPTIMSOCHOST_INCLUDE_DIR)

IF(LIBOPTIMSOCHOST_FOUND)
  SET(LIBOPTIMSOCHOST_LIBRARIES ${LIBOPTIMSOCHOST_LIBRARY})
ENDIF(LIBOPTIMSOCHOST_FOUND)

MARK_AS_ADVANCED(LIBOPTIMSOCHOST_LIBRARY LIBOPTIMSOCHOST_INCLUDE_DIR)
