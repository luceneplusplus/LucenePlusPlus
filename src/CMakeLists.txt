
add_subdirectory(core)
add_subdirectory(config)

if(ENABLE_CONTRIB)
  add_subdirectory(contrib)
endif()

if(ENABLE_DEMO)
  add_subdirectory(demo)
endif()

if(ENABLE_TEST)
  enable_testing()
  add_subdirectory(test)
endif()