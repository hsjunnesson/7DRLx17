protoc --cpp_out=generated_src .\proto\engine.proto
rem cmake -B .\build\ "-DCMAKE_TOOLCHAIN_FILE=C:/Users/Hans/Documents/GitHub/vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_BUILD_TYPE=Debug
