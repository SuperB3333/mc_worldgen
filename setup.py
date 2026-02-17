from setuptools import setup, Extension, find_packages

ext = Extension(
    "mc_worldgen._mc_worldgen",
    sources=[
        "python_wrapper.c",
        "biomenoise.c",
        "biomes.c",
        "finders.c",
        "generator.c",
        "layers.c",
        "noise.c",
        "quadbase.c",
        "util.c",
    ],
    extra_compile_args=["-O3"],
)

setup(
    name="mc_worldgen",
    version="0.1.0",
    packages=find_packages(),
    ext_modules=[ext],
    package_data={
        "mc_worldgen": ["*.pyi", "py.typed"],
    },
    zip_safe=False,
)
