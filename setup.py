from setuptools import setup, Extension

ext = Extension(
    "mc_worldgen",
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
    ext_modules=[ext],
)
