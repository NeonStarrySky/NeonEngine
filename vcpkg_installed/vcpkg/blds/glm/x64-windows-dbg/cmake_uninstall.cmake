if(NOT EXISTS "D:/neon/program/project/cpp/NeonEngine/vcpkg_installed/vcpkg/blds/glm/x64-windows-dbg/install_manifest.txt")
  message(FATAL_ERROR "Cannot find install manifest: D:/neon/program/project/cpp/NeonEngine/vcpkg_installed/vcpkg/blds/glm/x64-windows-dbg/install_manifest.txt")
endif()

file(READ "D:/neon/program/project/cpp/NeonEngine/vcpkg_installed/vcpkg/blds/glm/x64-windows-dbg/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
foreach(file ${files})
  message(STATUS "Uninstalling $ENV{DESTDIR}${file}")
  if(IS_SYMLINK "$ENV{DESTDIR}${file}" OR EXISTS "$ENV{DESTDIR}${file}")
    exec_program(
      "C:/Users/25108/AppData/Local/vcpkg/downloads/tools/cmake-3.31.10-windows/cmake-3.31.10-windows-x86_64/bin/cmake.exe" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
      OUTPUT_VARIABLE rm_out
      RETURN_VALUE rm_retval
      )
    if(NOT "${rm_retval}" STREQUAL 0)
      message(FATAL_ERROR "Problem when removing $ENV{DESTDIR}${file}")
    endif()
  else(IS_SYMLINK "$ENV{DESTDIR}${file}" OR EXISTS "$ENV{DESTDIR}${file}")
    message(STATUS "File $ENV{DESTDIR}${file} does not exist.")
  endif()
endforeach()
