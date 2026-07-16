cmake_minimum_required(VERSION 3.28)

project(binify
  VERSION 0.1.0
  DESCRIPTION "A lightweight Windows command entry manager"
  LANGUAGES CXX RC)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CompilerOptions)
include(Dependencies)

add_library(binify_core STATIC
  src/core/bat_wrapper.cpp
  src/core/command_conflict.cpp
  src/core/command_name.cpp
  src/core/config.cpp
  src/core/context_menu_command.cpp
  src/core/creation_policy.cpp
  src/core/error.cpp
  src/core/path_utils.cpp
  src/core/path_value.cpp
  src/core/text_encoding.cpp)
add_library(binify::core ALIAS binify_core)
target_include_directories(binify_core PUBLIC src)
target_link_libraries(binify_core PUBLIC nlohmann_json::nlohmann_json)
binify_apply_project_options(binify_core)

add_library(binify_app STATIC
  src/app/add_command_creation.cpp
  src/app/add_command_workflow.cpp
  src/app/application.cpp
  src/app/entry_transaction.cpp
  src/app/settings_workflow.cpp
  src/app/uninstall_workflow.cpp)
add_library(binify::app ALIAS binify_app)
target_include_directories(binify_app PUBLIC src)
target_link_libraries(binify_app PUBLIC binify::core)
binify_apply_project_options(binify_app)

add_library(binify_windows STATIC
  src/platform/windows/config_store.cpp
  src/platform/windows/context_menu_service.cpp
  src/platform/windows/environment_broadcast.cpp
  src/platform/windows/link_service.cpp
  src/platform/windows/path_service.cpp
  src/platform/windows/platform.cpp)
add_library(binify::windows ALIAS binify_windows)
target_include_directories(binify_windows PUBLIC src)
target_link_libraries(binify_windows PUBLIC binify::app)
target_link_libraries(binify_windows PRIVATE advapi32 shell32 user32)
binify_apply_project_options(binify_windows)

add_executable(binify WIN32
  src/main.cpp
  resources/binify.rc)
target_link_libraries(binify PRIVATE binify::windows winlamb)
binify_apply_project_options(binify)

enable_testing()

add_executable(binify_unit_tests
  tests/unit/add_command_creation_tests.cpp
  tests/unit/add_command_workflow_tests.cpp
  tests/unit/bat_wrapper_tests.cpp
  tests/unit/command_conflict_tests.cpp
  tests/unit/command_name_tests.cpp
  tests/unit/config_serialization_tests.cpp
  tests/unit/config_model_tests.cpp
  tests/unit/context_menu_command_tests.cpp
  tests/unit/creation_policy_tests.cpp
  tests/unit/entry_transaction_tests.cpp
  tests/unit/error_tests.cpp
  tests/unit/path_utils_tests.cpp
  tests/unit/path_value_tests.cpp
  tests/unit/result_tests.cpp
  tests/unit/settings_workflow_tests.cpp
  tests/unit/smoke_tests.cpp
  tests/unit/text_encoding_tests.cpp
  tests/unit/uninstall_workflow_tests.cpp)
target_link_libraries(binify_unit_tests PRIVATE binify::app GTest::gtest_main)
binify_apply_project_options(binify_unit_tests)
include(GoogleTest)
gtest_discover_tests(binify_unit_tests)

add_executable(binify_integration_tests
  tests/integration/config_store_tests.cpp
  tests/integration/context_menu_service_tests.cpp
  tests/integration/path_service_tests.cpp
  tests/integration/link_service_tests.cpp
  tests/integration/smoke_tests.cpp)
target_link_libraries(binify_integration_tests PRIVATE binify::windows GTest::gtest_main)
binify_apply_project_options(binify_integration_tests)
gtest_discover_tests(binify_integration_tests)
