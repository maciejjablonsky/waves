function(wf_add_shaders TARGET)
    cmake_parse_arguments(
        "PARSE_ARGS"
        ""
        ""
        "SOURCES"
        ${ARGN}
    )

    foreach (SHADER ${PARSE_ARGS_SOURCES})
        get_filename_component(FILE_NAME ${SHADER} NAME)
        get_filename_component(ABSOLUTE_PATH ${SHADER} ABSOLUTE)
        set(SHADER_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME}.spv")
        add_custom_command(
            OUTPUT ${SHADER_OUTPUT}
            COMMAND glslc ${ABSOLUTE_PATH} -o ${SHADER_OUTPUT}
            DEPENDS ${ABSOLUTE_PATH}
            COMMENT "Compiling GLSL shader ${ABSOLUTE_PATH}"
        )
        list(APPEND OUTPUTS ${SHADER_OUTPUT})
    endforeach()

    add_custom_target(${TARGET} DEPENDS ${OUTPUTS})
endfunction()

wf_add_shaders(
    wf_shaders
    SOURCES
        shader.vert
        shader.frag
)
