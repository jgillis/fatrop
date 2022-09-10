import setuptools
from Cython.Build import cythonize

with open("README.md", 'r') as f:
    long_description = f.read()

fatrop_extension = setuptools.Extension(
    name="fatropy",
    sources=["src/fatrop/fatropy/fatropy.pyx"],
    libraries=["fatrop"],
    library_dirs=["../build/fatrop"],
    include_dirs=["../fatrop/ocp","../fatrop/aux","../fatrop/solver","../fatrop/blasfeo_wrapper","../fatrop"],
    language="c++",
    define_macros=[("LEVEL1_DCACHE_LINE_SIZE","64")]
)
setuptools.setup(
    package_dir={"": "src"},
    long_description=long_description,
    packages=setuptools.find_packages(where='src',exclude=["fatropy"]),
    ext_modules=cythonize([fatrop_extension],compiler_directives={'language_level' : "3"})
)
