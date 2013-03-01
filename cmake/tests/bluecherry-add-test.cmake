set (CMAKE_INCLUDE_CURRENT_DIR TRUE)

macro (bluecherry_add_test name_)
    foreach (file ${ARGN})
        get_filename_component (mocFile "${file}" NAME_WE)
        set (mocFile "${CMAKE_CURRENT_BINARY_DIR}/${mocFile}.moc")
        qt4_generate_moc (${file} "${mocFile}")
        set_property (SOURCE ${file} APPEND PROPERTY OBJECT_DEPENDS "${mocFile}")
    endforeach ()

    file (RELATIVE_PATH sourcePath "${CMAKE_SOURCE_DIR}/tests" "${CMAKE_CURRENT_SOURCE_DIR}")

    get_filename_component (exeDir "${CMAKE_CURRENT_BINARY_DIR}/${name_}" PATH)
    file (MAKE_DIRECTORY "${exeDir}")

    add_executable (${name_} ${ARGN} ${bluecherry_client_SRCS})
    add_test ("${sourcePath}/${name_}" ${name_})

    target_link_libraries (${name_} ${bluecherry_client_LIBRARIES})
endmacro ()
