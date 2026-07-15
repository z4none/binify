function(binify_apply_project_options target_name)
  target_compile_features(${target_name} PUBLIC cxx_std_20)

  if(MSVC)
    target_compile_options(${target_name} PRIVATE
      /W4
      /permissive-
      /utf-8
      /guard:cf)
    target_compile_definitions(${target_name} PRIVATE
      UNICODE
      _UNICODE
      NOMINMAX
      WIN32_LEAN_AND_MEAN)
    target_link_options(${target_name} PRIVATE /guard:cf)
  endif()
endfunction()

