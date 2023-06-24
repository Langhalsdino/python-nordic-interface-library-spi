from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from distutils.sysconfig import customize_compiler
import sys
import setuptools

# Version needs to be updaten in src/nordic_utils.cpp too
__version__ = '0.1.2'

class my_build_ext(build_ext):
    def build_extensions(self):
        customize_compiler(self.compiler)
        try:
            self.compiler.compiler_so.remove("-Wstrict-prototypes")
        except (AttributeError, ValueError):
            pass
        build_ext.build_extensions(self)

ext_modules = [
    Extension(
        'nordicUtils',
        include_dirs=[
			'inc/'
        ],
		sources=[
			'src/spi.cpp',
            'src/nordic_utils.cpp',
            'src/protocol.cpp'
		],
        extra_compile_args=['-Wno-error=unused-variable'],
        extra_link_args=[],
        language='c++'
    )
]

setup(
    name='nordicUtils',
    version=__version__,
    author='Thomas Hartmann, Matthias Heger',
    author_email='thomas.hartmann@apic.ai',
    description='A python wrapper to work with nordic',
    long_description='',
    ext_modules=ext_modules,
    zip_safe=False,
    cmdclass = {'build_ext': my_build_ext},
    data_files=[
        'inc/spi.h',
        'inc/spi_commands.h',
        'inc/protocol.h'
    ],
)
