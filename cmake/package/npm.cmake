# 查找 Node.js 可执行文件
find_program(NODE_EXECUTABLE node DOC "Node.js executable")
if(NOT NODE_EXECUTABLE)
    message(WARNING "node not found, JS targets will not work")
endif()

# 查找 TypeScript 执行器 (tsx 或 ts-node)
find_program(TSX_EXECUTABLE tsx DOC "TypeScript executeable")
find_program(TS_NODE_EXECUTABLE ts-node DOC "TypeScript executeable")

# 添加自定义目标来执行 cmake/package 目录下的 js 或 ts 文件
# 用法: NPM_ADD_TARGET(目标名称 脚本名)
# 示例: NPM_ADD_TARGET(run_install install)
#
# 脚本参数通过命令行传递：
#   1. buildType      - 构建类型 (Debug/Release)
#   2. projectRoot    - 项目根目录
#   3. buildDir       - 构建目录
#   4. platformPrefix - 平台相关前缀
#   5. libDir         - 库输出目录
#   6. runtimeDir     - 运行时输出目录
#   7. archiveDir     - 归档输出目录
#   8. ruleJsonPath   - rule.json 路径（可选，自动查找）
function(NPM_ADD_TARGET TARGET_NAME SCRIPT_NAME)
    if(NOT NODE_EXECUTABLE)
        return()
    endif()

    # 脚本文件在 cmake/package/ 目录下
    set(SCRIPT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/package")
    set(JS_FILE "${SCRIPT_DIR}/${SCRIPT_NAME}.js")
    set(TS_FILE "${SCRIPT_DIR}/${SCRIPT_NAME}.ts")

    # 优先使用 js，如果不存在则尝试 ts
    if(EXISTS "${JS_FILE}")
        set(RUN_FILE "${JS_FILE}")
        set(RUN_EXECUTABLE "${NODE_EXECUTABLE}")
        set(RUN_COMMENT "Running ${SCRIPT_NAME}.js...")
    elseif(EXISTS "${TS_FILE}")
        set(RUN_FILE "${TS_FILE}")
        if(TSX_EXECUTABLE)
            set(RUN_EXECUTABLE "${TSX_EXECUTABLE}")
        elseif(TS_NODE_EXECUTABLE)
            set(RUN_EXECUTABLE "${TS_NODE_EXECUTABLE}")
        else()
            message(WARNING "TypeScript file found but tsx/ts-node not found, ${SCRIPT_NAME}.ts will not work")
            return()
        endif()
        set(RUN_COMMENT "Running ${SCRIPT_NAME}.ts...")
    else()
        message(WARNING "js/ts file not found: ${JS_FILE} or ${TS_FILE}")
        return()
    endif()

    # 查找 rule.json
    set(RULE_JSON_PATH "")
    if(EXISTS "${SCRIPT_DIR}/rule.json")
        set(RULE_JSON_PATH "${SCRIPT_DIR}/rule.json")
        message(STATUS "NPM_ADD_TARGET: Found rule.json at ${RULE_JSON_PATH}")
    else()
        message(STATUS "NPM_ADD_TARGET: rule.json not found in ${SCRIPT_DIR}")
    endif()

    # 构建命令
    add_custom_target(${TARGET_NAME}
        COMMAND ${RUN_EXECUTABLE} "${RUN_FILE}"
            "$<CONFIG>"
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${CMAKE_BINARY_DIR}"
            "$<CONFIG>/$<PLATFORM_ID>"
            "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
            "${RULE_JSON_PATH}"
        COMMENT "${RUN_COMMENT}"
        VERBATIM
    )
endfunction()

# 生成目标映射缓存文件（target-map.json）
# 在 configure 阶段结束后（DEFER）收集 rule.json 中定义的目标的输出文件路径
# 包含：主输出文件（dll/so/exe）、链接库文件（lib/a）
function(NPM_GENERATE_TARGET_MAP)
    set(RULE_JSON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/package/rule.json")
    if(NOT EXISTS "${RULE_JSON_PATH}")
        return()
    endif()

    file(READ "${RULE_JSON_PATH}" RULE_CONTENT)
    string(JSON TARGET_COUNT LENGTH "${RULE_CONTENT}" targets)
    math(EXPR TARGET_LAST "${TARGET_COUNT} - 1")

    set(TARGET_MAP "[\n")

    set(FIRST_ENTRY TRUE)

    foreach(I RANGE ${TARGET_LAST})
        string(JSON TARGET_NAME GET "${RULE_CONTENT}" targets ${I} name)

        if(NOT TARGET ${TARGET_NAME})
            continue()
        endif()

        # 检查是否有 outputName
        string(JSON HAS_OUTPUT_NAME ERROR_VARIABLE _err MEMBER "${RULE_CONTENT}" targets ${I} outputName)
        if(HAS_OUTPUT_NAME)
            string(JSON OUTPUT_NAME GET "${RULE_CONTENT}" targets ${I} outputName)
            set(ON_FIELD "\n    \"outputName\": \"${OUTPUT_NAME}\",")
        else()
            set(ON_FIELD "")
        endif()

        # 获取 ARCHIVE_OUTPUT_DIRECTORY（控制 .lib/.a 输出目录）
        # 未设置时默认为 target 的 BINARY_DIR
        get_target_property(_archive_dir ${TARGET_NAME} ARCHIVE_OUTPUT_DIRECTORY)
        if(_archive_dir)
            set(ARCHIVE_DIR_FIELD "\n    \"archiveDir\": \"${_archive_dir}\",")
        else()
            set(ARCHIVE_DIR_FIELD "\n    \"archiveDir\": \"$<TARGET_PROPERTY:${TARGET_NAME},BINARY_DIR>\",")
        endif()

        # 主输出文件（dll/so/dylib/exe）
        if(NOT FIRST_ENTRY)
            string(APPEND TARGET_MAP ",\n")
        endif()
        string(APPEND TARGET_MAP "  {\n    \"name\": \"${TARGET_NAME}\",${ON_FIELD}${ARCHIVE_DIR_FIELD}\n    \"file\": \"$<TARGET_FILE:${TARGET_NAME}>\"\n  }")
        set(FIRST_ENTRY FALSE)

    endforeach()

    string(APPEND TARGET_MAP "\n]\n")

    file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/target-map-$<CONFIG>.json" CONTENT "${TARGET_MAP}")
endfunction()

# 延迟到所有目标定义完成后执行
cmake_language(DEFER CALL NPM_GENERATE_TARGET_MAP)

# 打包
NPM_ADD_TARGET(buildnpm install)