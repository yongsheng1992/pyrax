from distutils.core import setup, Extension

setup(
    name='pyrax',
    ext_modules=[
        Extension(
            name='pyrax',
            sources=['src/pyrax.c', 'src/rax.c'],
            include_dirs=['./src'],
            library_dirs = ['/usr/local/lib']
        )
    ]
)
