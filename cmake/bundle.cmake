include(BundleUtilities)

# this override function tells fixup_bundle to ignore anything in
# the lib dir of the install tree
function(gp_resolved_file_type_override resolved_file type_var)
  if(resolved_file MATCHES "\\.\\./lib")
    set(${type_var} "local" PARENT_SCOPE)
  endif()
endfunction()

fixup_bundle(${CMAKE_INSTALL_PREFIX}/bin/testTranslator "" ${CMAKE_INSTALL_PREFIX}/lib)
