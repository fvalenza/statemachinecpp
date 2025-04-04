# Determine the unique target name based on the current project name
string(REPLACE " " "_" project_name_sanitized "${CMAKE_PROJECT_NAME}")
set(lib_compiler_flags_target "lib_compiler_flags_${project_name_sanitized}")

# Create the INTERFACE library with the unique target name
add_library(${lib_compiler_flags_target} INTERFACE)
target_compile_features(${lib_compiler_flags_target} INTERFACE cxx_std_11)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(${lib_compiler_flags_target} INTERFACE
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

# Export the target name to the parent scope (the project using this file)
set(lib_compiler_flags ${lib_compiler_flags_target} PARENT_SCOPE)
