set(ITK3P_INSTALL_EXPORT_NAME "${ITKNetlib-targets}")
set(ITK3P_INSTALL_INCLUDE_DIR "${ITKNetlib_INSTALL_INCLUDE_DIR}")
set(ITK3P_INSTALL_RUNTIME_DIR "${ITKNetlib_INSTALL_RUNTIME_DIR}")
set(ITK3P_INSTALL_LIBRARY_DIR "${ITKNetlib_INSTALL_LIBRARY_DIR}")
set(ITK3P_INSTALL_ARCHIVE_DIR "${ITKNetlib_INSTALL_ARCHIVE_DIR}")
add_subdirectory(netlib)
itk_module_target(itkNetlibSlatec NO_INSTALL)
