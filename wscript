# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

def configure(conf):
    conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('on-off-keying-module', ['core','network', 'mpi'])
    module.source = [
        'model/on-off-keying-module.cc',
        'helper/on-off-keying-module-helper.cc',
        'model/on-off-keying-channel.cc',
        'model/on-off-keying-net-device.cc',

        ]

    module_test = bld.create_ns3_module_test_library('on-off-keying-module')
    module_test.source = [
        'test/on-off-keying-module-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'on-off-keying-module'
    headers.source = [
        'model/on-off-keying-module.h',
        'helper/on-off-keying-module-helper.h',
        'model/on-off-keying-net-device.h',
        'model/on-off-keying-channel.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    bld.ns3_python_bindings()
