sources = [
  'valdrmort.cpp',
  'valdrmort_element.cpp',
  'DRMBackend.cpp',
  'DRMBackendFactory.cpp',
  'logger.cpp',
  'SecurityCompliance.cpp',
  'SecAPIManager.cpp',
  'ClearKeyBackend.cpp',
]

inc_dirs = include_directories('../include')

shared_library(
  'gstvaldrmort',
  sources,
  include_directories: [inc_dirs, configinc],
  dependencies : [gst_dep, gst_base_dep, libxml2_dep, libcurl_dep, cjson_dep],
  install : true,
  install_dir : plugins_install_dir
)
