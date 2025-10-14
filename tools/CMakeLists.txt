function(initialize_tool tool_name)
    if(${CMAKE_CURRENT_SOURCE_DIR}/${tool_name}/main.cpp IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/${tool_name}.exe)
        message(STATUS "Configuring ${tool_name} tool...")
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${tool_name}/out)
        if(WIN32)
            set(${tool_name}_triplet x64-windows-static)
            set(${tool_name}_build_type RelWithDebInfo)
        elseif(UNIX)
            set(${tool_name}_triplet x64-linux)
            set(${tool_name}_build_type Release)
        endif()
        execute_process(COMMAND ${CMAKE_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/${tool_name} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=${${tool_name}_build_type} -DVCPKG_TARGET_TRIPLET=${${tool_name}_triplet}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${tool_name}/out
            RESULT_VARIABLE result
            OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${tool_name}/out/${tool_name}.log
            ERROR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${tool_name}/out/${tool_name}_error.log
        )

        if(result)
            message(FATAL_ERROR "Failed to build ${tool_name}, see ${CMAKE_CURRENT_SOURCE_DIR}/${tool_name}/out/${tool_name}_error.log for details")
        endif()

        message(STATUS "Building ${tool_name} tool...")
        execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release --target ${tool_name}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${tool_name}/out
            RESULT_VARIABLE result
            OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${tool_name}/out/${tool_name}.log
            ERROR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${tool_name}/out/${tool_name}.log
        )

        if(result)
            message(FATAL_ERROR "Failed to build ${tool_name}, see ${CMAKE_CURRENT_SOURCE_DIR}/${tool_name}/out/${tool_name}_error.log for details")
        endif()

        message(STATUS "${tool_name} tool built successfully")
    endif()
endfunction(initialize_tool)

initialize_tool(binary_embed)
initialize_tool(reflect_map)
initialize_tool(entity_proxy_gen)
initialize_tool(id_proxy_gen)


function(build_resource project_name input_file output_folder mode)
    cmake_path(GET input_file FILENAME result_name)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/binary_embed)
    execute_process(
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/binary_embed.exe "direct" ${project_name} ${input_file} ${output_folder} ${mode}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE result
        ERROR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/binary_embed/${result_name}_error.log
    )
    if(result)
        message(FATAL_ERROR "Failed to build resource ${input_file}")
    endif()
    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs)
endfunction(build_resource)

function(build_resource_json project_name input_file output_folder mode)
    cmake_path(GET input_file FILENAME result_name)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/binary_embed)
    execute_process(
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/binary_embed.exe "json" ${project_name} ${input_file} ${output_folder} ${mode}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE result
        ERROR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/binary_embed/${result_name}_error.log
    )
    if(result)
        message(FATAL_ERROR "Failed to build resource ${input_file}")
    endif()
    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs)
endfunction(build_resource_json)

function(build_resource_merge_json project_name input_folder output_folder mode)
    cmake_path(GET input_folder FILENAME result_name)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/binary_embed)
    execute_process(
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/binary_embed.exe "merge_json" ${project_name} ${input_folder} ${output_folder} ${mode}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE result
        ERROR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/binary_embed/${result_name}_error.log
    )
    if(result)
        message(FATAL_ERROR "Failed to build resource ${input_folder}")
    endif()
    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs)
endfunction(build_resource_merge_json)


function(build_resource_senbt project_name input_file output_folder mode)
    cmake_path(GET input_file FILENAME result_name)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/binary_embed)
    execute_process(
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/binary_embed.exe "senbt" ${project_name} ${input_file} ${output_folder} ${mode}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE result
        ERROR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/binary_embed/${result_name}_error.log
    )
    if(result)
        message(FATAL_ERROR "Failed to build resource ${input_file}")
    endif()
    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs)
endfunction(build_resource_senbt)

function(build_resource_merge_senbt project_name input_folder output_folder mode)
    cmake_path(GET input_folder FILENAME result_name)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/binary_embed)
    execute_process(
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/binary_embed.exe "merge_senbt" ${project_name} ${input_folder} ${output_folder} ${mode}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE result
        ERROR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/binary_embed/${result_name}_error.log
    )
    if(result)
        message(FATAL_ERROR "Failed to build resource ${input_folder}")
    endif()
    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs)
endfunction(build_resource_merge_senbt)

function(build_resources project_name output_folder FILES HEADERS_PAR SOURCES_PAR mode)
    foreach(file ${FILES})
        build_resource(${project_name} ${file} ${output_folder} ${mode})
    endforeach()

    FILE(GLOB HEADERS
        "${output_folder}/*.hpp"
        "${output_folder}/*/*.hpp"
        "${output_folder}/*/*/*.hpp"
        "${output_folder}/*/*/*/*.hpp"
        "${output_folder}/*/*/*/*/*.hpp"
        "${output_folder}/*/*/*/*/*/*.hpp"
        "${output_folder}/*/*/*/*/*/*/*.hpp"
    )
    FILE(GLOB SOURCES
        "${output_folder}/*.cpp"
        "${output_folder}/*/*.cpp"
        "${output_folder}/*/*/*.cpp"
        "${output_folder}/*/*/*/*.cpp"
        "${output_folder}/*/*/*/*/*.cpp"
        "${output_folder}/*/*/*/*/*/*.cpp"
        "${output_folder}/*/*/*/*/*/*/*.cpp"
    )

    set(${HEADERS_PAR} ${HEADERS})
    set(${SOURCES_PAR} ${SOURCES})

    return(PROPAGATE ${HEADERS_PAR} ${SOURCES_PAR})
endfunction(build_resources)


function(build_resources_json project_name output_folder FILES HEADERS_PAR SOURCES_PAR mode)
    foreach(file ${FILES})
        build_resource_json(${project_name} ${file} ${output_folder} ${mode})
    endforeach()

    FILE(GLOB HEADERS
        "${output_folder}/*.hpp"
        "${output_folder}/*/*.hpp"
        "${output_folder}/*/*/*.hpp"
        "${output_folder}/*/*/*/*.hpp"
        "${output_folder}/*/*/*/*/*.hpp"
        "${output_folder}/*/*/*/*/*/*.hpp"
        "${output_folder}/*/*/*/*/*/*/*.hpp"
    )
    FILE(GLOB SOURCES
        "${output_folder}/*.cpp"
        "${output_folder}/*/*.cpp"
        "${output_folder}/*/*/*.cpp"
        "${output_folder}/*/*/*/*.cpp"
        "${output_folder}/*/*/*/*/*.cpp"
        "${output_folder}/*/*/*/*/*/*.cpp"
        "${output_folder}/*/*/*/*/*/*/*.cpp"
    )

    set(${HEADERS_PAR} ${HEADERS})
    set(${SOURCES_PAR} ${SOURCES})

    return(PROPAGATE ${HEADERS_PAR} ${SOURCES_PAR})
endfunction(build_resources_json)


function(build_resources project_name output_folder FILES HEADERS_PAR SOURCES_PAR mode)
    foreach(file ${FILES})
        cmake_path(GET file EXTENSION file_ext)
        if(file_ext STREQUAL ".json")
            build_resource_json(${project_name} ${file} ${output_folder} ${mode})
        elseif(file_ext STREQUAL ".senbt")
            build_resource_senbt(${project_name} ${file} ${output_folder} ${mode})
        else()
            build_resource(${project_name} ${file} ${output_folder} ${mode})
        endif() 
    endforeach()

    FILE(GLOB HEADERS
        "${output_folder}/*.hpp"
        "${output_folder}/*/*.hpp"
        "${output_folder}/*/*/*.hpp"
        "${output_folder}/*/*/*/*.hpp"
        "${output_folder}/*/*/*/*/*.hpp"
        "${output_folder}/*/*/*/*/*/*.hpp"
        "${output_folder}/*/*/*/*/*/*/*.hpp"
    )
    FILE(GLOB SOURCES
        "${output_folder}/*.cpp"
        "${output_folder}/*/*.cpp"
        "${output_folder}/*/*/*.cpp"
        "${output_folder}/*/*/*/*.cpp"
        "${output_folder}/*/*/*/*/*.cpp"
        "${output_folder}/*/*/*/*/*/*.cpp"
        "${output_folder}/*/*/*/*/*/*/*.cpp"
    )

    set(${HEADERS_PAR} ${HEADERS})
    set(${SOURCES_PAR} ${SOURCES})

    return(PROPAGATE ${HEADERS_PAR} ${SOURCES_PAR})
endfunction(build_resources)

function(create_resources_header resources_folder resource_headers)
    file(MAKE_DIRECTORY "${resources_folder}")
    set(resources_header "${resources_folder}/include.hpp")
    foreach(file ${resource_headers})
        if(${file} IS_NEWER_THAN ${resources_header})
            set(needs_update TRUE)
        endif()
    endforeach()
    if(NOT needs_update)
        return()
    endif()
    file(REMOVE ${resources_header})
    file(WRITE ${resources_header} "#pragma once\n")
    foreach(file ${resource_headers})
        file(APPEND ${resources_header} "#include \"${file}\"\n")
    endforeach()
endfunction(create_resources_header)


function(create_reflect_map output_file FILES)
    cmake_path(GET output_file FILENAME result_name)
    #convert FILES to a string
    set(FILES_STR "")
    foreach(file ${FILES})
        set(FILES_STR "${file} ${FILES_STR}")
    endforeach()


    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/reflect_map)
    execute_process(
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/reflect_map.exe ${output_file} ${FILES_STR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE result
        ERROR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/reflect_map/${result_name}_error.log
    )
    if(result)
        message(FATAL_ERROR "Failed to build reflection map ${output_file}")
    endif()
    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs)
endfunction(create_reflect_map)

function(id_proxy_gen input_json output_header output_implementation res_type type_header api_namespace)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs)
    execute_process(
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/id_proxy_gen.exe ${input_json} ${output_header} ${output_implementation} ${res_type} ${type_header} ${api_namespace}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE result
        ERROR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/id_proxy_gen_error.log
    )
    if(result)
        message(FATAL_ERROR "Failed to create mapping for ${input_json}")
    endif()
endfunction(id_proxy_gen)


function(create_mappings)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs)
    execute_process(
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/entity_proxy_gen.exe "${CMAKE_CURRENT_SOURCE_DIR}/resources/registry/entities.json" "${CMAKE_CURRENT_SOURCE_DIR}/src/generated/entity/components.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/generated/entity/wrappers.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/generated/entity/factory.hpp" 
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE result
        ERROR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/entities_error.log
    )
    if(result)
        message(FATAL_ERROR "Failed to create mapping for /resources/registry/entities.json")
    endif()
    id_proxy_gen(
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/registry/blocks.senbt" 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/api/collection/block.hpp" 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/api/bin/collection/block.cpp" 
        "base_objects::block" 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/base_objects/block.hpp"
        "collection::block"
    )
    id_proxy_gen(
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/registry/items.json" 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/api/collection/item.hpp" 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/api/bin/collection/item.cpp" 
        "base_objects::item_id_t" 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/base_objects/slot.hpp"
        "collection::item"
    )


    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs)
endfunction(create_mappings)

function(execute_gradle project_dir args)
    if(WIN32)
        set(GRADLEW_CMD  "${project_dir}/gradlew.bat")
    else()
        set(GRADLEW_CMD  "${project_dir}/gradlew")
    endif()
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/gradle)
    message(STATUS "Executing gradle command in ${GRADLEW_CMD}: ${args}")
    execute_process(
        COMMAND ${GRADLEW_CMD} ${args} --no-daemon
        WORKING_DIRECTORY ${project_dir}
        RESULT_VARIABLE result
        ERROR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/gradle/error.log
    )
    if(result)
        message(FATAL_ERROR "Failed to execute gradle command in ${project_dir}, see error.log for details in ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs/gradle")
    endif()
    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/tools/logs)
endfunction(execute_gradle)

function(run_extractor)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/eula.txt")
        execute_gradle(${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor "runServer")
        file(READ ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/eula.txt eula_content)
        string(REPLACE "eula=false" "eula=true" eula_content "${eula_content}")
        file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/eula.txt "${eula_content}")
    endif()
    execute_gradle(${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor "runServer")
endfunction(run_extractor)

function(run_mojang_extractor)
    execute_gradle(${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor "runVanillaDatagen")
endfunction(run_mojang_extractor)

function(perform_extraction)
    message(STATUS "Extracting data from vanilla server")
    run_mojang_extractor()
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/resources/data)
    file(RENAME ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/generated/data/minecraft ${CMAKE_CURRENT_SOURCE_DIR}/resources/data)
    file(RENAME ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/generated/reports/registries.json ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry/protocol.json)

    run_extractor()
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/block_entity_types.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/block_properties.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/block_shapes.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(RENAME ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/blocks.json ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry/blocks.senbt)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/composter_increase_chance.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/entities.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/entity_pose.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/entity_attributes.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/flower_pot_transformation.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/fuels.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/items.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/potion_recipes.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/potion.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/recipes.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/spawn_egg.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/status_effects.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/extractor/run/output/synced_registries.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/registry)

    create_mappings()
endfunction(perform_extraction)

function(prepare_resources)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/registry/blocks.senbt")
        perform_extraction()
    endif()
endfunction(prepare_resources)


