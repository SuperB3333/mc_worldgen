from setuptools import setup, Extension

ext = Extension(
    "mc_worldgen",
    sources=["generatur.h", "util.h", "finders.h", "quadbase.h"],
    extra_compile_args=["-O3"], # optional
)

setup(
    name="mc_worldgen",
    ext_modules=[ext],
)
