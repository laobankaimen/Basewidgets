function(PF_PRIVATE_INIT_BUILD_SYSTEM RELEASE_DIR)
    if(WIN32)
        set(PF_LIB_DIR_NAME "${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}_${CMAKE_CXX_COMPILER_ID}_$<CONFIG>" CACHE INTERNAL "Platform Library Dir Name")
        set(PF_BUILD_TYPE "$<CONFIG>" CACHE INTERNAL "Build Type")
    else()
        set(PF_LIB_DIR_NAME "${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}_${CMAKE_CXX_COMPILER_ID}_${CMAKE_BUILD_TYPE}" CACHE INTERNAL "Platform Library Dir Name")
        set(PF_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE INTERNAL "Build Type")
    endif()
	if(RELEASE_DIR)
		set(PF_RELEASE_DIR "${RELEASE_DIR}" CACHE INTERNAL "Release Repo Dir")
	else()
		set(PF_RELEASE_DIR "${CMAKE_SOURCE_DIR}/../com.plugin.releases" CACHE INTERNAL "Release Repo Dir")
	endif()

	if(NOT DEFINED PF_INSTALL_DIR)
		set(PF_INSTALL_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" CACHE PATH "Install Directory" FORCE)
		set(CMAKE_INSTALL_PREFIX "${PF_INSTALL_DIR}" CACHE PATH "CMake Install Directory" FORCE)
		message(STATUS "Install Directory: ${PF_INSTALL_DIR}")
	endif()
endfunction()

#-------------------------------------------------------------------------------------------------------

# 发布目标
function (PF_PUBLISH_TARGET TARGET_NAME)
    PF_PRIVATE_INIT_BUILD_SYSTEM("")

	set(PF_INSTALL_FUNCTION_NAME PF_PUBLISH_${TARGET_NAME})
	if(COMMAND ${PF_INSTALL_FUNCTION_NAME})
		message(STATUS "Publish ${TARGET_NAME} Target")
		cmake_language(CALL ${PF_INSTALL_FUNCTION_NAME})
	else()
		message(WARNING, "Cannot Find ${PF_INSTALL_FUNCTION_NAME} Command, Use Default Install Command")
		string(TOLOWER "${TARGET_NAME}" LOWER_TARGET_NAME)
		set(PF_TARGET_INC_PATH "${PF_RELEASE_DIR}/${LOWER_TARGET_NAME}")
		set(PF_TARGET_LIB_PATH "${PF_RELEASE_DIR}/${LOWER_TARGET_NAME}/${PF_LIB_DIR_NAME}")
	
		install(TARGETS ${TARGET_NAME}
				RUNTIME DESTINATION "${PF_TARGET_LIB_PATH}"
				LIBRARY DESTINATION "${PF_TARGET_LIB_PATH}"
				ARCHIVE DESTINATION "${PF_TARGET_LIB_PATH}")
		install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include" DESTINATION "${PF_TARGET_INC_PATH}" OPTIONAL)
		install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/interface" DESTINATION "${PF_TARGET_INC_PATH}" OPTIONAL)
		install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION ${PF_TARGET_INC_PATH} OPTIONAL)
		
		if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
			install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PF_BUILD_TYPE}/${TARGET_NAME}.pdb" DESTINATION "${PF_TARGET_LIB_PATH}" OPTIONAL)
		endif()
	endif()
endfunction()

# 发布插件
function (PF_PUBLISH_PLUGIN PLUGIN_ID TARGET_NAME)
	PF_PRIVATE_INIT_BUILD_SYSTEM("")

	set(PF_INSTALL_FUNCTION_NAME PF_PUBLISH_${PLUGIN_ID})
	if(COMMAND ${PF_INSTALL_FUNCTION_NAME})
		message(STATUS "Publish ${PLUGIN_ID} Plugin")
		cmake_language(CALL ${PF_INSTALL_FUNCTION_NAME})
	else()
		message(WARNING, "Cannot Find ${PF_INSTALL_FUNCTION_NAME} Command, Use Default Install Command")
		set(PF_PLUGIN_DIR "${PF_RELEASE_DIR}/plugins/${PLUGIN_ID}")
		install(TARGETS ${TARGET_NAME}
				RUNTIME DESTINATION "${PF_PLUGIN_DIR}/${PF_LIB_DIR_NAME}"
				LIBRARY DESTINATION "${PF_PLUGIN_DIR}/${PF_LIB_DIR_NAME}"
				ARCHIVE DESTINATION "${PF_PLUGIN_DIR}/${PF_LIB_DIR_NAME}")

		install(DIRECTORY "${CMAKE_SOURCE_DIR}/interface" DESTINATION ${PF_PLUGIN_DIR} OPTIONAL)
		install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets" DESTINATION ${PF_PLUGIN_DIR} OPTIONAL)
		
		if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
			install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PF_BUILD_TYPE}/${TARGET_NAME}.pdb" DESTINATION "${PF_PLUGIN_DIR}/${PF_LIB_DIR_NAME}" OPTIONAL)
		endif()
	endif()
endfunction()

#-------------------------------------------------------------------------------------------------------

function(PF_PRIVATE_READY_BUILD_TOOLS)
	set(PF_PRIVATE_BUILDTOOL_DIR "${PF_RELEASE_DIR}/buildtools")
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		if(NOT EXISTS "${PF_PRIVATE_BUILDTOOL_DIR}/script_linux")
			message(STATUS "UnZip script_linux.zip")
			execute_process(COMMAND unzip "-d" "${PF_PRIVATE_BUILDTOOL_DIR}" "${PF_PRIVATE_BUILDTOOL_DIR}/script_linux.zip")
			execute_process(COMMAND chmod "-R" "+x" "${PF_PRIVATE_BUILDTOOL_DIR}/script_linux")
		endif()
	elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		if(NOT EXISTS "${PF_PRIVATE_BUILDTOOL_DIR}/script_windows")
			message(STATUS "UnZip script_windows.zip")
			execute_process(COMMAND ${PF_PRIVATE_BUILDTOOL_DIR}/bin/unzip.exe "${PF_PRIVATE_BUILDTOOL_DIR}/script_windows.zip" "-d" "${PF_PRIVATE_BUILDTOOL_DIR}")
		endif()
	endif()
endfunction()

PF_PRIVATE_INIT_BUILD_SYSTEM("")
PF_PRIVATE_READY_BUILD_TOOLS()
file(GLOB_RECURSE ALL_INSTALL_FILES "${PF_RELEASE_DIR}/install.cmake")
LIST(LENGTH ALL_INSTALL_FILES ALL_INSTALL_FILES_LENGTH)
if(${ALL_INSTALL_FILES_LENGTH})
	foreach(VAR IN LISTS ALL_INSTALL_FILES)
		include(${VAR})
	endforeach()
endif()

function(PF_PRIVATE_IMPORT_TARGET_DEFAULT TARGET_NAME)
	message(STATUS "Import ${TARGET_NAME} Lib")
	string(TOLOWER "${TARGET_NAME}" LOWER_TARGET_NAME)
    set(PF_PLUGIN_INC_PATH "${PF_RELEASE_DIR}/plugins/${LOWER_TARGET_NAME}/interface")
	if(EXISTS "${PF_PLUGIN_INC_PATH}")
    	include_directories("${PF_PLUGIN_INC_PATH}")
	endif()
    set(PF_PLUGIN_INC_PATH "${PF_RELEASE_DIR}/plugins/${LOWER_TARGET_NAME}/include")
	if(EXISTS "${PF_PLUGIN_INC_PATH}")
    	include_directories("${PF_PLUGIN_INC_PATH}")
	endif()
	link_directories("${PF_RELEASE_DIR}/${LOWER_TARGET_NAME}/${PF_LIB_DIR_NAME}")
endfunction()

function(PF_PRIVATE_IMPORT_PLUGIN_DEFAULT PLUGIN_ID)
	message(STATUS "Import ${PLUGIN_ID} Plugin")
    set(PF_PLUGIN_INC_PATH "${PF_RELEASE_DIR}/plugins/${PLUGIN_ID}/interface")
	if(EXISTS "${PF_PLUGIN_INC_PATH}")
    	include_directories("${PF_PLUGIN_INC_PATH}")
	endif()
endfunction()


function(PF_PRIVATE_INSTALL_TARGET TARGET_NAME)
	get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)

	# 是一个可执行程序
	if("${TARGET_TYPE}" STREQUAL "EXECUTABLE")
		message(STATUS "Install ${TARGET_NAME} Executable")
		install(TARGETS ${TARGET_NAME}
			RUNTIME DESTINATION "${PF_INSTALL_DIR}"
			LIBRARY DESTINATION "${PF_INSTALL_DIR}"
			ARCHIVE DESTINATION "${PF_INSTALL_DIR}")
		if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
			install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PF_BUILD_TYPE}/${TARGET_NAME}.pdb" DESTINATION "${PF_INSTALL_DIR}" OPTIONAL)
		endif()
	else()
		# 是一个常规的库
		string(TOLOWER "${TARGET_NAME}" LOWER_TARGET_NAME)
		if(EXISTS "${PF_RELEASE_DIR}/${LOWER_TARGET_NAME}")
			message(STATUS "Install ${TARGET_NAME} Target")
			install(DIRECTORY "${PF_RELEASE_DIR}/${LOWER_TARGET_NAME}/${PF_LIB_DIR_NAME}/" DESTINATION "${PF_INSTALL_DIR}" OPTIONAL PATTERN "*.lib" EXCLUDE)
			install(DIRECTORY "${PF_RELEASE_DIR}/${LOWER_TARGET_NAME}/assets/" DESTINATION "${PF_INSTALL_DIR}" OPTIONAL)
		endif()
	endif()
endfunction()

function(PF_PRIVATE_INSTALL_PLUGIN PLUGIN_ID)
	# 这是一个插件
	if(EXISTS "${PF_RELEASE_DIR}/plugins/${PLUGIN_ID}")
		message(STATUS "Install ${PLUGIN_ID} Plugin")
		install(DIRECTORY "${PF_RELEASE_DIR}/plugins/${PLUGIN_ID}/${PF_LIB_DIR_NAME}/" DESTINATION "${PF_INSTALL_DIR}/plugins/${PLUGIN_ID}" OPTIONAL PATTERN "*.lib" EXCLUDE)
		install(DIRECTORY "${PF_RELEASE_DIR}/plugins/${PLUGIN_ID}/assets/" DESTINATION "${PF_INSTALL_DIR}/plugins/${PLUGIN_ID}" OPTIONAL)
	endif()
endfunction()


# 导入一个目标
function(PF_IMPORT_TARGET TARGET_NAME)
	set(PF_IMPORT_FUNCTION_NAME PF_IMPORT_${TARGET_NAME})
	if(COMMAND ${PF_IMPORT_FUNCTION_NAME})
		message(STATUS "Import ${TARGET_NAME} Target")
		cmake_language(CALL ${PF_IMPORT_FUNCTION_NAME})
	else()
		message(WARNING, "Cannot Find ${PF_IMPORT_FUNCTION_NAME} Command, Use Default Import Command")
		PF_PRIVATE_IMPORT_TARGET_DEFAULT(${TARGET_NAME})
	endif()
	list(LENGTH PF_PRIVATE_TARGET_LIST LENGTH)
	if(${LENGTH})
		set(PF_PRIVATE_TARGET_LIST "${TARGET_NAME};${PF_PRIVATE_TARGET_LIST}" PARENT_SCOPE)
	else()
		set(PF_PRIVATE_TARGET_LIST ${TARGET_NAME} PARENT_SCOPE)
	endif()

endfunction()

# 导入一个插件
function(PF_IMPORT_PLUGIN PLUGIN_ID TARGET_NAME)
	set(PF_IMPORT_FUNCTION_NAME PF_IMPORT_${PLUGIN_ID})
	if(COMMAND ${PF_IMPORT_FUNCTION_NAME})
		message(STATUS "Import ${PLUGIN_ID} Plugin")
		cmake_language(CALL ${PF_IMPORT_FUNCTION_NAME})
	else()
		message(WARNING, "Cannot Find ${PF_IMPORT_FUNCTION_NAME} Command, Use Default Import Command")
		PF_PRIVATE_IMPORT_PLUGIN_DEFAULT(${PLUGIN_ID})
	endif()
	list(LENGTH PF_PRIVATE_TARGET_LIST LENGTH)
	if(${LENGTH})
		set(PF_PRIVATE_TARGET_LIST "${TARGET_NAME};${PF_PRIVATE_TARGET_LIST}" PARENT_SCOPE)
	else()
		set(PF_PRIVATE_TARGET_LIST ${TARGET_NAME} PARENT_SCOPE)
	endif()
endfunction()

#-------------------------------------------------------------------------------------------------------

# 安装一个目标
function(PF_INSTALL_TARGET TARGET_NAME)
	message(STATUS "Install ${TARGET_NAME} Target")
	set(PF_INSTALL_FUNCTION_NAME PF_INSTALL_${TARGET_NAME})
	if(COMMAND ${PF_INSTALL_FUNCTION_NAME})
		cmake_language(CALL ${PF_INSTALL_FUNCTION_NAME})
	else()
		message(WARNING, "Cannot Find ${PF_INSTALL_FUNCTION_NAME} Command, Use Default Instance Command")
		PF_PRIVATE_INSTALL_TARGET(${TARGET_NAME})
	endif()
endfunction()

# 安装一个插件
function(PF_INSTALL_PLUGIN PLUGIN_ID)
	message(STATUS "Install ${PLUGIN_ID} Plugin")
	set(PF_INSTALL_FUNCTION_NAME PF_INSTALL_${PLUGIN_ID})
	if(COMMAND ${PF_INSTALL_FUNCTION_NAME})
		cmake_language(CALL ${PF_INSTALL_FUNCTION_NAME})
	else()
		message(WARNING, "Cannot Find ${PF_INSTALL_FUNCTION_NAME} Command, Use Default Instance Command")
		PF_PRIVATE_INSTALL_PLUGIN(${PLUGIN_ID})
	endif()
endfunction()

#-------------------------------------------------------------------------------------------------------

# 删除目录[源目录]
function(PF_PRIVATE_DELETE_DIR CUSTOM_TARGET_NAME SOURCE_DIR)
	PF_PRIVATE_INIT_BUILD_SYSTEM("")
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		add_custom_command(TARGET ${CUSTOM_TARGET_NAME}
			COMMAND "${PF_RELEASE_DIR}/buildtools/script_linux/deletedir.sh" "${SOURCE_DIR}"
		)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		add_custom_command(TARGET ${CUSTOM_TARGET_NAME}
			COMMAND "${PF_RELEASE_DIR}/buildtools/script_windows/bin/bash.exe" "--login" "--noediting" "${PF_RELEASE_DIR}/buildtools/script_windows/scripts/deletedir_win.sh" "${SOURCE_DIR}"
		)
    endif()
endfunction()

function(PF_DELETE_DIR CUSTOM_TARGET_NAME SOURCE_DIR)
	# 自定义目标列表
	list(FIND PF_FILE_TARGET_LIST ${CUSTOM_TARGET_NAME} INDEX)
	if(INDEX GREATER -1)
	else()
		list(LENGTH PF_FILE_TARGET_LIST LIST_LENGTH)
		if(${LIST_LENGTH})
			set(PF_FILE_TARGET_LIST "${CUSTOM_TARGET_NAME};${PF_FILE_TARGET_LIST}" PARENT_SCOPE)
		else()
			set(PF_FILE_TARGET_LIST "${CUSTOM_TARGET_NAME}" PARENT_SCOPE)
		endif()
	endif()

	list(LENGTH PF_${CUSTOM_TARGET_NAME}_DELETE_DIRLIST_S LIST_LENGTH)
	if(${LIST_LENGTH})
		set(PF_${CUSTOM_TARGET_NAME}_DELETE_DIRLIST_S "${SOURCE_DIR};${PF_${CUSTOM_TARGET_NAME}_DELETE_DIRLIST_S}" PARENT_SCOPE)
	else()
		set(PF_${CUSTOM_TARGET_NAME}_DELETE_DIRLIST_S "${SOURCE_DIR}" PARENT_SCOPE)
	endif()
endfunction()

# 删除文件[源文件]
function(PF_PRIVATE_DELETE_FILE CUSTOM_TARGET_NAME SOURCE_FILE)
	PF_PRIVATE_INIT_BUILD_SYSTEM("")
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		add_custom_command(TARGET ${CUSTOM_TARGET_NAME}
			COMMAND "${PF_RELEASE_DIR}/buildtools/script_linux/deletefile.sh" "${SOURCE_FILE}"
		)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		add_custom_command(TARGET ${CUSTOM_TARGET_NAME}
			COMMAND "${PF_RELEASE_DIR}/buildtools/script_windows/bin/bash.exe" "--login" "--noediting" "${PF_RELEASE_DIR}/buildtools/script_windows/scripts/deletefile_win.sh" "${SOURCE_FILE}"
		)
    endif()
endfunction()

function(PF_DELETE_FILE CUSTOM_TARGET_NAME SOURCE_FILE)
	# 自定义目标列表
	list(FIND PF_FILE_TARGET_LIST ${CUSTOM_TARGET_NAME} INDEX)
	if(INDEX GREATER -1)
	else()
		list(LENGTH PF_FILE_TARGET_LIST LIST_LENGTH)
		if(${LIST_LENGTH})
			set(PF_FILE_TARGET_LIST "${CUSTOM_TARGET_NAME};${PF_FILE_TARGET_LIST}" PARENT_SCOPE)
		else()
			set(PF_FILE_TARGET_LIST "${CUSTOM_TARGET_NAME}" PARENT_SCOPE)
		endif()
	endif()

	list(LENGTH PF_${CUSTOM_TARGET_NAME}_DELETE_FILELIST_S LIST_LENGTH)
	if(${LIST_LENGTH})
		set(PF_${CUSTOM_TARGET_NAME}_DELETE_FILELIST_S "${SOURCE_FILE};${PF_${CUSTOM_TARGET_NAME}_DELETE_FILELIST_S}" PARENT_SCOPE)
	else()
		set(PF_${CUSTOM_TARGET_NAME}_DELETE_FILELIST_S "${SOURCE_FILE}" PARENT_SCOPE)
	endif()
endfunction()

# 拷贝文件[源文件 目标文件]，注意拷贝将强制覆盖目标文件
function(PF_PRIVATE_COPY_FILE CUSTOM_TARGET_NAME COPY_SOURCE_FILE COPY_DEST_FILE)
	PF_PRIVATE_INIT_BUILD_SYSTEM("")
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		add_custom_command(TARGET ${CUSTOM_TARGET_NAME}
			COMMAND "${PF_RELEASE_DIR}/buildtools/script_linux/copyfile.sh" "${COPY_SOURCE_FILE}" "${COPY_DEST_FILE}"
		)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		add_custom_command(TARGET ${CUSTOM_TARGET_NAME}
			COMMAND "${PF_RELEASE_DIR}/buildtools/script_windows/bin/bash.exe" "--login" "--noediting" "${PF_RELEASE_DIR}/buildtools/script_windows/scripts/copyfile_win.sh" "${COPY_SOURCE_FILE}" "${COPY_DEST_FILE}"
		)
    endif()
endfunction()

function(PF_COPY_FILE CUSTOM_TARGET_NAME COPY_SOURCE_FILE COPY_DEST_FILE)
	# 自定义目标列表
	list(FIND PF_FILE_TARGET_LIST ${CUSTOM_TARGET_NAME} INDEX)
	if(INDEX GREATER -1)
	else()
		list(LENGTH PF_FILE_TARGET_LIST LIST_LENGTH)
		if(${LIST_LENGTH})
			set(PF_FILE_TARGET_LIST "${CUSTOM_TARGET_NAME};${PF_FILE_TARGET_LIST}" PARENT_SCOPE)
		else()
			set(PF_FILE_TARGET_LIST "${CUSTOM_TARGET_NAME}" PARENT_SCOPE)
		endif()
	endif()

	list(LENGTH PF_${CUSTOM_TARGET_NAME}_COPY_FILELIST_S LIST_LENGTH)
	if(${LIST_LENGTH})
		set(PF_${CUSTOM_TARGET_NAME}_COPY_FILELIST_S "${COPY_SOURCE_FILE};${PF_${CUSTOM_TARGET_NAME}_COPY_FILELIST_S}" PARENT_SCOPE)
	else()
		set(PF_${CUSTOM_TARGET_NAME}_COPY_FILELIST_S "${COPY_SOURCE_FILE}" PARENT_SCOPE)
	endif()
	
	list(LENGTH PF_${CUSTOM_TARGET_NAME}_COPY_FILELIST_D LIST_LENGTH)
	if(${LIST_LENGTH})
		set(PF_${CUSTOM_TARGET_NAME}_COPY_FILELIST_D "${COPY_DEST_FILE};${PF_${CUSTOM_TARGET_NAME}_COPY_FILELIST_D}" PARENT_SCOPE)
	else()
		set(PF_${CUSTOM_TARGET_NAME}_COPY_FILELIST_D "${COPY_DEST_FILE}" PARENT_SCOPE)
	endif()
endfunction()


# 拷贝文件夹到目标文件夹下面[源文件夹 目标文件夹]，注意拷贝将强制覆盖目标文件夹
function(PF_PRIVATE_COPY_DIR CUSTOM_TARGET_NAME COPY_SOURCE_DIR COPY_DEST_DIR)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		add_custom_command(TARGET ${CUSTOM_TARGET_NAME}
			COMMAND "${PF_RELEASE_DIR}/buildtools/script_linux/copydir.sh" "${COPY_SOURCE_DIR}" "${COPY_DEST_DIR}"
		)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		add_custom_command(TARGET ${CUSTOM_TARGET_NAME}
			COMMAND "${PF_RELEASE_DIR}/buildtools/script_windows/bin/bash.exe" "--login" "--noediting" "${PF_RELEASE_DIR}/buildtools/script_windows/scripts/copydir_win.sh" "${COPY_SOURCE_DIR}" "${COPY_DEST_DIR}"
		)
    endif()
endfunction()

# 将拷贝任务追加到缓存，在end_publish_flow调用时统一执行
function(PF_COPY_DIR CUSTOM_TARGET_NAME COPY_SOURCE_DIR COPY_DEST_DIR)
	# 自定义目标列表
	list(FIND PF_FILE_TARGET_LIST ${CUSTOM_TARGET_NAME} INDEX)
	if(INDEX GREATER -1)
	else()
		list(LENGTH PF_FILE_TARGET_LIST LIST_LENGTH)
		if(${LIST_LENGTH})
			set(PF_FILE_TARGET_LIST "${CUSTOM_TARGET_NAME};${PF_FILE_TARGET_LIST}" PARENT_SCOPE)
		else()
			set(PF_FILE_TARGET_LIST "${CUSTOM_TARGET_NAME}" PARENT_SCOPE)
		endif()
	endif()

	list(LENGTH PF_${CUSTOM_TARGET_NAME}_COPY_DIRLIST_S LIST_LENGTH)
	if(${LIST_LENGTH})
		set(PF_${CUSTOM_TARGET_NAME}_COPY_DIRLIST_S "${COPY_SOURCE_DIR};${PF_${CUSTOM_TARGET_NAME}_COPY_DIRLIST_S}" PARENT_SCOPE)
	else()
		set(PF_${CUSTOM_TARGET_NAME}_COPY_DIRLIST_S "${COPY_SOURCE_DIR}" PARENT_SCOPE)
	endif()
	
	list(LENGTH PF_${CUSTOM_TARGET_NAME}_COPY_DIRLIST_D LIST_LENGTH)
	if(${LIST_LENGTH})
		set(PF_${CUSTOM_TARGET_NAME}_COPY_DIRLIST_D "${COPY_DEST_DIR};${PF_${CUSTOM_TARGET_NAME}_COPY_DIRLIST_D}" PARENT_SCOPE)
	else()
		set(PF_${CUSTOM_TARGET_NAME}_COPY_DIRLIST_D "${COPY_DEST_DIR}" PARENT_SCOPE)
	endif()
endfunction()

# 拷贝文件夹下所有文件到目标文件夹[源文件夹 目标文件夹]，注意拷贝将强制覆盖目标文件
function(PF_PRIVATE_COPY_SUBDIR CUSTOM_TARGET_NAME COPY_SOURCE_DIR COPY_DEST_DIR)
	PF_PRIVATE_INIT_BUILD_SYSTEM("")
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		add_custom_command(TARGET ${CUSTOM_TARGET_NAME}
			COMMAND "${PF_RELEASE_DIR}/buildtools/script_linux/copydir_down.sh" "${COPY_SOURCE_DIR}" "${COPY_DEST_DIR}"
		)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		add_custom_command(TARGET ${CUSTOM_TARGET_NAME}
			COMMAND "${PF_RELEASE_DIR}/buildtools/script_windows/bin/bash.exe" "--login" "--noediting" "${PF_RELEASE_DIR}/buildtools/script_windows/scripts/copydir_down_win.sh" "${COPY_SOURCE_DIR}" "${COPY_DEST_DIR}"
		)
    endif()
endfunction()

function(PF_COPY_SUBDIR CUSTOM_TARGET_NAME COPY_SOURCE_DIR COPY_DEST_DIR)
	# 自定义目标列表
	list(FIND PF_FILE_TARGET_LIST ${CUSTOM_TARGET_NAME} INDEX)
	if(INDEX GREATER -1)
	else()
		list(LENGTH PF_FILE_TARGET_LIST LIST_LENGTH)
		if(${LIST_LENGTH})
			set(PF_FILE_TARGET_LIST "${CUSTOM_TARGET_NAME};${PF_FILE_TARGET_LIST}" PARENT_SCOPE)
		else()
			set(PF_FILE_TARGET_LIST "${CUSTOM_TARGET_NAME}" PARENT_SCOPE)
		endif()
	endif()

	list(LENGTH PF_${CUSTOM_TARGET_NAME}_COPY_SUBDIRLIST_S LIST_LENGTH)
	if(${LIST_LENGTH})
		set(PF_${CUSTOM_TARGET_NAME}_COPY_SUBDIRLIST_S "${COPY_SOURCE_DIR};${PF_${CUSTOM_TARGET_NAME}_COPY_SUBDIRLIST_S}" PARENT_SCOPE)
	else()
		set(PF_${CUSTOM_TARGET_NAME}_COPY_SUBDIRLIST_S "${COPY_SOURCE_DIR}" PARENT_SCOPE)
	endif()
	
	list(LENGTH PF_${CUSTOM_TARGET_NAME}_COPY_SUBDIRLIST_D LIST_LENGTH)
	if(${LIST_LENGTH})
		set(PF_${CUSTOM_TARGET_NAME}_COPY_SUBDIRLIST_D "${COPY_DEST_DIR};${PF_${CUSTOM_TARGET_NAME}_COPY_SUBDIRLIST_D}" PARENT_SCOPE)
	else()
		set(PF_${CUSTOM_TARGET_NAME}_COPY_SUBDIRLIST_D "${COPY_DEST_DIR}" PARENT_SCOPE)
	endif()
endfunction()

#-------------------------------------------------------------------------------------------------------

# 新增一个自定义发布目标
function(PF_ADD_CUSTOM_TARGET CUSTOM_TARGET_NAME)
	add_custom_target(${CUSTOM_TARGET_NAME})
	#install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${CUSTOM_TARGET_NAME})")
	set_target_properties(${CUSTOM_TARGET_NAME} PROPERTIES FOLDER "BuildTarget")
endfunction()

# 添加QT文件
function(PF_ADD_QT_FILE EXENAME QT_FILE)
	# 待处理目标列表
	list(FIND PF_FILE_QT_TARGET_LIST ${EXENAME} INDEX)
	if(INDEX GREATER -1)
	else()
		list(LENGTH PF_FILE_QT_TARGET_LIST LIST_LENGTH)
		if(${LIST_LENGTH})
			set(PF_FILE_QT_TARGET_LIST "${EXENAME};${PF_FILE_QT_TARGET_LIST}" PARENT_SCOPE)
		else()
			set(PF_FILE_QT_TARGET_LIST "${EXENAME}" PARENT_SCOPE)
		endif()
	endif()

	list(LENGTH PF_QT_${EXENAME}_COPYFILE_LIST LIST_LENGTH)
	if(${LIST_LENGTH})
		set(PF_QT_${EXENAME}_COPYFILE_LIST "${QT_FILE};${PF_QT_${EXENAME}_COPYFILE_LIST}" PARENT_SCOPE)
	else()
		set(PF_QT_${EXENAME}_COPYFILE_LIST "${QT_FILE}" PARENT_SCOPE)
	endif()
endfunction()

# 开始流程
function(PF_PUBLISH_FLOW_BEGIN)
	set(options ENABLE_LOG)
	set(oneValueArgs RELEASE_DIR INSTALL_DIR)
	set(multiValueArgs TARGETS)
	
	cmake_parse_arguments(
		PF_PRIVATE_ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
	)
    if(NOT PF_PRIVATE_ARG_RELEASE_DIR)
        set(PF_PRIVATE_ARG_RELEASE_DIR "${CMAKE_SOURCE_DIR}/../com.plugin.releases")
    endif()

	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	PF_PRIVATE_INIT_BUILD_SYSTEM(${PF_PRIVATE_ARG_RELEASE_DIR})
	PF_PRIVATE_READY_BUILD_TOOLS()

	message(STATUS "Publish Flow Begin")
	set(PF_LIB_DIR_NAME "${PF_LIB_DIR_NAME}" PARENT_SCOPE)
	set(PF_BUILD_TYPE "${PF_BUILD_TYPE}" PARENT_SCOPE)
	
	
	if(PF_PRIVATE_ARG_INSTALL_DIR)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PF_PRIVATE_ARG_INSTALL_DIR}" CACHE PATH "Output Install Directory" FORCE)
		message(STATUS "Reset Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
	else()
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Output Install Directory" FORCE)
		message(STATUS "Reset Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
	endif()

	if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		#if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
		#	# CMAKE默认安装目录，则修改
		#	message(STATUS "Reset Install Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
		#	set(PF_INSTALL_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" PARENT_SCOPE)
		#else()

		# 强制使用构建目录
		set(PF_INSTALL_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" CACHE PATH "Install Directory" FORCE)
		set(CMAKE_INSTALL_PREFIX "${PF_INSTALL_DIR}" CACHE PATH "CMake Install Directory" FORCE)
		message(STATUS "Install Directory: ${PF_INSTALL_DIR}")

		#endif()
	elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		#if("${CMAKE_INSTALL_PREFIX}" STREQUAL "C:/Program Files/${PROJECT_NAME}" OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "C:/Program Files (x86)/${PROJECT_NAME}" )
		#	# CMAKE默认安装目录，则修改
		#	message(STATUS "Reset Install Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PF_BUILD_TYPE}")
		#	set(PF_INSTALL_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PF_BUILD_TYPE}" PARENT_SCOPE)
		#	set(CMAKE_INSTALL_PREFIX "${PF_PRIVATE_ARG_INSTALL_DIR}" CACHE PATH "New Install Directory" FORCE)
		#else()

		# 强制使用构建目录
		set(PF_INSTALL_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PF_BUILD_TYPE}" CACHE PATH "Install Directory" FORCE)
		set(CMAKE_INSTALL_PREFIX "${PF_INSTALL_DIR}" CACHE PATH "CMake Install Directory" FORCE)
		message(STATUS "Install Directory: ${PF_INSTALL_DIR}")

		#endif()
	endif()
endfunction()

# 结束流程
function(PF_PUBLISH_FLOW_END)
	# 安装目标
	LIST(LENGTH PF_PRIVATE_TARGET_LIST PF_PRIVATE_TARGET_LIST_LENGTH)
	if(${PF_PRIVATE_TARGET_LIST_LENGTH})
		foreach(VAR IN LISTS PF_PRIVATE_TARGET_LIST)
		PF_INSTALL_TARGET(${VAR})
		endforeach()
	endif()

	LIST(LENGTH PF_FILE_TARGET_LIST PF_FILE_TARGET_LIST_LENGTH)
	if(${PF_FILE_TARGET_LIST_LENGTH})
		add_custom_target(PFBuildAllTarget ALL DEPENDS ${PF_FILE_TARGET_LIST})
		#install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target PFBuildAllTarget)")
		set_target_properties(PFBuildAllTarget PROPERTIES FOLDER "BuildTarget")
		foreach(VAR IN LISTS PF_FILE_TARGET_LIST)
			# 拷贝文件
			LIST(LENGTH PF_${VAR}_COPY_FILELIST_S PF_COPY_FILELIST_S_LENGTH)
			LIST(LENGTH PF_${VAR}_COPY_FILELIST_D PF_COPY_FILELIST_D_LENGTH)
			#message(STATUS "+++++++++++++++${PF_COPY_FILELIST_S_LENGTH} ${PF_COPY_FILELIST_D_LENGTH}")
			if(${PF_COPY_FILELIST_S_LENGTH})
				if(${PF_COPY_FILELIST_S_LENGTH} EQUAL ${PF_COPY_FILELIST_D_LENGTH})
					foreach(SS DD IN ZIP_LISTS PF_${VAR}_COPY_FILELIST_S PF_${VAR}_COPY_FILELIST_D)
						PF_PRIVATE_COPY_FILE(${VAR} ${SS} ${DD})
					endforeach()
				endif()
			endif()

			# 拷贝目录
			LIST(LENGTH PF_${VAR}_COPY_DIRLIST_S PF_COPY_DIRLIST_S_LENGTH)
			LIST(LENGTH PF_${VAR}_COPY_DIRLIST_D PF_COPY_DIRLIST_D_LENGTH)
			#message(STATUS "+++++++++++++++${PF_COPY_DIRLIST_S_LENGTH} ${PF_COPY_DIRLIST_D_LENGTH}")
			if(${PF_COPY_DIRLIST_S_LENGTH})
				if(${PF_COPY_DIRLIST_S_LENGTH} EQUAL ${PF_COPY_DIRLIST_D_LENGTH})
					foreach(SS DD IN ZIP_LISTS PF_${VAR}_COPY_DIRLIST_S PF_${VAR}_COPY_DIRLIST_D)
						PF_PRIVATE_COPY_DIR(${VAR} ${SS} ${DD})
					endforeach()
				endif()
			endif()

			# 拷贝子目录
			LIST(LENGTH PF_${VAR}_COPY_SUBDIRLIST_S PF_COPY_SUBDIRLIST_S_LENGTH)
			LIST(LENGTH PF_${VAR}_COPY_SUBDIRLIST_D PF_COPY_SUBDIRLIST_D_LENGTH)
			if(${PF_COPY_SUBDIRLIST_S_LENGTH})
				if(${PF_COPY_SUBDIRLIST_S_LENGTH} EQUAL ${PF_COPY_SUBDIRLIST_D_LENGTH})
					foreach(SS DD IN ZIP_LISTS PF_${VAR}_COPY_SUBDIRLIST_S PF_${VAR}_COPY_SUBDIRLIST_D)
						PF_PRIVATE_COPY_SUBDIR(${VAR} ${SS} ${DD})
					endforeach()
				endif()
			endif()
		
			# 删除文件
			LIST(LENGTH PF_${VAR}_DELETE_FILELIST_S PF_DELETE_FILELIST_S_LENGTH)
			if(${PF_DELETE_FILELIST_S_LENGTH})
				foreach(SS IN LISTS PF_${VAR}_DELETE_FILELIST_S)
					PF_PRIVATE_DELETE_FILE(${VAR} ${SS})
				endforeach()
			endif()
		
			# 删除目录
			LIST(LENGTH PF_${VAR}_DELETE_DIRLIST_S PF_DELETE_DIRLIST_S_LENGTH)
			if(${PF_DELETE_DIRLIST_S_LENGTH})
				foreach(SD IN LISTS PF_${VAR}_DELETE_DIRLIST_S)
					PF_PRIVATE_DELETE_DIR(${VAR} ${SD})
				endforeach()
			endif()
		endforeach()
	endif()


	if(DEFINED ENV{QT5_DIR})
		message(STATUS "Find Environment Variable QT5_DIR")
		string(REPLACE "\\" "." PF_QT_SDK_DIR "$ENV{QT5_DIR}/../../..")
		message(STATUS "Get Qt5 Path: ${PF_QT_SDK_DIR}")
		if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
			message(STATUS "Windows Platform Use windeployqt.exe Package")
			set(LOCAL_SCRIPT_DIR "${PF_RELEASE_DIR}/buildtools/script_windows")
			LIST(LENGTH PF_FILE_QT_TARGET_LIST PF_FILE_QT_TARGET_LIST_LENGTH)
			if(${PF_FILE_QT_TARGET_LIST_LENGTH})
				add_custom_target(PFInstallQtFile ALL DEPENDS ${PF_FILE_QT_TARGET_LIST})
				set_target_properties(PFInstallQtFile PROPERTIES FOLDER "BuildTarget")
				foreach(VAR IN LISTS PF_FILE_QT_TARGET_LIST)
					set(QT_FILE_LIST "${PF_QT_${VAR}_COPYFILE_LIST}")
					add_custom_command(TARGET PFInstallQtFile
						COMMAND "${PF_QT_SDK_DIR}/bin/windeployqt.exe" "${PF_INSTALL_DIR}/${VAR}.exe"
					)

					LIST(LENGTH QT_FILE_LIST QT_FILE_LIST_LENGTH)
					if(${QT_FILE_LIST_LENGTH})
						foreach(VAR IN LISTS QT_FILE_LIST)
							add_custom_command(TARGET PFInstallQtFile
								COMMAND "${LOCAL_SCRIPT_DIR}/bin/bash.exe" "--login" "--noediting" "${LOCAL_SCRIPT_DIR}/scripts/copy_qt_files_win.sh" "${PF_QT_SDK_DIR}/bin/${VAR}.dll" "${PF_INSTALL_DIR}/${VAR}.dll" "${PF_BUILD_TYPE}"
								COMMAND "${LOCAL_SCRIPT_DIR}/bin/bash.exe" "--login" "--noediting" "${LOCAL_SCRIPT_DIR}/scripts/copy_qt_files_win.sh" "${PF_QT_SDK_DIR}/bin/${VAR}d.dll" "${PF_INSTALL_DIR}/${VAR}d.dll" "${PF_BUILD_TYPE}"
							)
						endforeach()
					endif()
				endforeach()
			endif()
		endif()
	else()
		message(STATUS "Can Not Found Environment Variable: QT5_DIR")
	endif()

	# 安装QT相关文件
	#LIST(LENGTH PF_QT_COPYFILE_LIST PF_QT_COPYFILE_LIST_LENGTH)
	#if(${PF_QT_COPYFILE_LIST_LENGTH})
	#		PF_PRIAVTE_COPY_QT_FILES("${PF_QT_COPYFILE_LIST}")
	#endif()
	
	message(STATUS "Publish Flow End")
endfunction()

function(PF_GET_BUILDTYPE RESULT)
	set(RESULT ${PF_BUILD_TYPE} PARENT_SCOPE)
endfunction()

function(PF_GET_LIBNAME RESULT)
	PF_PRIVATE_INIT_BUILD_SYSTEM("")
	set(RESULT ${PF_LIB_DIR_NAME} PARENT_SCOPE)
endfunction()

function(PF_GET_TARGET_FILE_NAME TARGET_NAME TARGET_FILE_PATH TARGET_FILE_NAME)
	set(${TARGET_FILE_PATH} "$<TARGET_FILE:${TARGET_NAME}>" PARENT_SCOPE)
	set(${TARGET_FILE_NAME} "$<TARGET_FILE_NAME:${TARGET_NAME}>" PARENT_SCOPE)
endfunction()
