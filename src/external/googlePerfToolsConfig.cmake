# Finds and configures google-perftools if it exists on the system. 
# Andre Anjos - 07.september.2010

# We start by defining GOOGLE_PERFTOOLS_FOUND to false
set(GOOGLE_PERFTOOLS_FOUND "NO" CACHE INTERNAL "package")

# Here we do some variable cleanup and adjustments
find_path(GOOGLE_PERFTOOLS_INCLUDE_1 NAMES google/profiler.h)
set(GOOGLE_PERFTOOLS_INCLUDE_DIRS ${GOOGLE_PERFTOOLS_INCLUDE_1})
find_path(GOOGLE_PERFTOOLS_INCLUDE_2 NAMES google/tcmalloc.h)
set(GOOGLE_PERFTOOLS_INCLUDE_DIRS "${GOOGLE_PERFTOOLS_INCLUDE_DIRS};${GOOGLE_PERFTOOLS_INCLUDE_2}")
list(REMOVE_DUPLICATES GOOGLE_PERFTOOLS_INCLUDE_DIRS)

find_library(GOOGLE_PERFTOOLS_LIBRARY NAMES profiler)
set(GOOGLE_PERFTOOLS_LIBRARIES ${GOOGLE_PERFTOOLS_LIBRARY} CACHE INTERNAL "libraries")

if(GOOGLE_PERFTOOLS_INCLUDE_1 AND GOOGLE_PERFTOOLS_INCLUDE_2 AND GOOGLE_PERFTOOLS_LIBRARY)
  message( STATUS "Google Perftools FOUND: Compiling add-on modules...")
  set(GOOGLE_PERFTOOLS_FOUND "YES" CACHE INTERNAL "package")
  include_directories(SYSTEM ${GOOGLE_PERFTOOLS_INCLUDE_DIRS})
  add_definitions(-DHAVE_GOOGLE_PERFTOOLS=1)
else()
  # This will say why we have got to that conclusion
  set(GOOGLE_PERFTOOLS_FOUND "NO")
  message( STATUS "Google Perftools NOT FOUND: Disabling...")
  if (NOT GOOGLE_PERFTOOLS_INCLUDE_1)
    message( STATUS "Google Perftools <google/profiler.h> not found!")
  endif (NOT GOOGLE_PERFTOOLS_INCLUDE_1)
  if (NOT GOOGLE_PERFTOOLS_INCLUDE_2)
    message( STATUS "Google Perftools <google/tcmalloc.h> not found!")
  endif (NOT GOOGLE_PERFTOOLS_INCLUDE_2)
  if (NOT GOOGLE_PERFTOOLS_LIBRARY)
    message( STATUS "Google Perftools libprofiler.so not found!")
  endif (NOT GOOGLE_PERFTOOLS_LIBRARY)
endif()