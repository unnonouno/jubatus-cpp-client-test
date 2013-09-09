VERSION = '0.0.1'
APPNAME = 'jubatus-cpp-client-test'

def options(opt):
    opt.load('compiler_cxx')
    opt.load('unittest_gtest')

def configure(conf):
    conf.env.CXXFLAGS += ['-O2', '-Wall', '-g', '-pipe']
    conf.load('compiler_cxx')
    conf.load('unittest_gtest')

    conf.check_cfg(package = 'pficommon', args = '--cflags --libs')
    conf.check_cfg(package = 'jubatus-client', args = '--cflags --libs')
    # conf.check_cxx(lib='libname', header_name = 'header.h')

def build(bld):
    bld.recurse('src')
