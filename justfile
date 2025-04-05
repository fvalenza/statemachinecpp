alias c := clean
alias gc := gen_cmake
alias b := build
alias cb := rebuild
alias i := install
alias gs := gen_compile_commands
alias r := run
alias d := doc
alias bt := build_tests
alias rt := run_tests
alias t := tests


clean:
    rm -rf build/

gen_cmake *CMAKE_VARS:
    mkdir -p build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=~/root-filetree/devel/install -DCMAKE_BUILD_TYPE=Debug {{CMAKE_VARS}} ..

build TARGETS='all':
    cmake --build build/ --target {{TARGETS}}

rebuild *CMAKE_VARS: clean (gen_cmake "{{CMAKE_VARS}}") build

install:
    cmake --build build/ --target install

gen_compile_commands:
    #!/usr/bin/env bash
    set -euxo pipefail
    if [ ! -f "build/compile_commands.json" ]; then
        echo "compile_commands.json not found in build/. Generating"
        mkdir -p build && cd build && \
        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
    else
        echo "compile_commands.json already existing. Run regen_compile_commands if want to clean and regenerate"
    fi
    cp -f compile_commands.json ..

doc:
    cd build && make doc

run number='':
    ./build/src/manager{{number}}
build_tests:
    cd build && make manager_tests
run_tests:
    ./build/tests/manager_tests

tests: build_tests run_tests

cov:
    cd build && \
    cmake --build . && \
    ctest --progress && \
    lcov  -c -d . -o main_coverage.info.tmp && \
    lcov -r main_coverage.info.tmp '/usr/include/*' -o main_coverage.info && \
    genhtml main_coverage.info --output-directory out 
# lcov  -c -d . -o main_coverage.info --no-external

opencov:
    open build/out/index.html
