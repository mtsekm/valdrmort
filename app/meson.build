sources = [
  'main.cpp',
  'Application.cpp',
  'MPDManifestParser.cpp',
  'M3U8ManifestParser.cpp',
  'PipelineManager.cpp',
  'LicenseRequestHelper.cpp'
]

inc_dirs = include_directories('.', '../include')

executable('valdrmort',
  sources,
  include_directories : inc_dirs,
  dependencies : [gst_dep, glib_dep, libxml2_dep],
  install : true
)
