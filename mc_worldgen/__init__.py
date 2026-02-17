from ._mc_worldgen import Generator as _Generator
from . import _mc_worldgen
from enum import IntEnum

def _make_enum(name, source_cls):
    constants = {k: v for k, v in source_cls.__dict__.items() if not k.startswith('_')}
    return IntEnum(name, constants)

Version = _make_enum('Version', _mc_worldgen.Version)
Dimension = _make_enum('Dimension', _mc_worldgen.Dimension)
Flag = _make_enum('Flag', _mc_worldgen.Flag)
Biome = _make_enum('Biome', _mc_worldgen.Biome)

class Generator(_Generator):
    def get_biome_at(self, scale: int, x: int, y: int, z: int) -> Biome:
        return Biome(super().get_biome_at(scale, x, y, z))

    get_biome_at.__doc__ = _Generator.get_biome_at.__doc__

    def gen_biomes(self, scale: int, x: int, z: int, sx: int, sz: int, y: int = 0, sy: int = 1) -> list[Biome]:
        return [Biome(b) for b in super().gen_biomes(scale, x, z, sx, sz, y, sy)]

    gen_biomes.__doc__ = _Generator.gen_biomes.__doc__

Generator.__doc__ = _Generator.__doc__
