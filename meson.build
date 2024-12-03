project('valdrmort', 'cpp',
  version : '1.0.0',
  default_options : ['warning_level=1', 'cpp_std=c++17', 'buildtype=debugoptimized']
)

gst_req = '>= 1.8.0'
apiversion = '1.0'

# Required dependencies
glib_dep = dependency('glib-2.0')
gst_dep = dependency('gstreamer-1.0', version : gst_req)
gst_base_dep = dependency('gstreamer-base-1.0', version : gst_req)
libxml2_dep = dependency('libxml-2.0', required : true)
libcurl_dep = dependency('libcurl', required : true)
cjson_dep = dependency('libcjson', required : true)

# Version and configuration data
valdrmort_version = meson.project_version()
core_conf = configuration_data()
core_conf.set_quoted('VERSION', valdrmort_version)
core_conf.set_quoted('PACKAGE_NAME', 'valdrmort')
core_conf.set_quoted('PACKAGE', 'valdrmort')

# Generate configuration header
configure_file(output : 'config.h', configuration : core_conf)
configinc = include_directories('.')

# Plugin installation directory
plugins_install_dir = join_paths(get_option('libdir'), 'gstreamer-1.0')

# Platform-specific arguments
platform = get_option('platform')
if platform == 'realtek'
    add_project_arguments('-DPLATFORM_REALTEK', language : 'cpp')
elif platform == 'amlogic'
    add_project_arguments('-DPLATFORM_AMLOGIC', language : 'cpp')
elif platform == 'broadcom'
    add_project_arguments('-DPLATFORM_BROADCOM', language : 'cpp')
else
    error('Invalid platform. Must be one of: realtek, amlogic, broadcom.')
endif

# Subdirectories
subdir('gst/src')
subdir('app/src')

# Summary of dependencies
summary({
  'Project version' : valdrmort_version,
  'GStreamer' : gst_dep.version(),
  'GLib' : glib_dep.version(),
  'libxml2' : libxml2_dep.version(),
  'libcurl' : libcurl_dep.version(),
  'cJSON' : cjson_dep.version(),
})
