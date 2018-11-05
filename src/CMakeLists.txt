project (udev-test)

# find_package(udev REQUIRED)

aux_source_directory(. SOURCE_FILES)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# target_link_libraries(${PROJECT_NAME}
    # ${UDEV_LIBRARY}
# )

