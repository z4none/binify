include(FetchContent)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/winlamb/winlamb.hpp")
  add_library(winlamb INTERFACE)
  target_include_directories(winlamb SYSTEM INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/winlamb")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/winlamb/include/winlamb.hpp")
  add_library(winlamb INTERFACE)
  target_include_directories(winlamb SYSTEM INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/winlamb/include")
else()
  add_library(winlamb INTERFACE)
  message(WARNING "WinLamb headers were not found. Run: git submodule update --init --recursive")
endif()

FetchContent_Declare(nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
  URL_HASH SHA256=e39786088138f2749d64e9e90e0f9902daa77c4b8b1f8ca926cba99b2ff5f12a)
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

