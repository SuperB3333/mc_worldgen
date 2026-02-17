import mc_worldgen
from enum import IntEnum

def test_enum_returns():
    print("Testing Enum return types...")

    # Check if classes are IntEnums
    assert issubclass(mc_worldgen.Biome, IntEnum)
    assert issubclass(mc_worldgen.Version, IntEnum)
    assert issubclass(mc_worldgen.Dimension, IntEnum)

    g = mc_worldgen.Generator(mc=mc_worldgen.Version.MC_1_20)
    g.apply_seed(mc_worldgen.Dimension.OVERWORLD, 12345)

    # Test get_biome_at
    biome = g.get_biome_at(1, 0, 64, 0)
    print(f"get_biome_at returned: {biome!r} (type: {type(biome)})")
    assert isinstance(biome, mc_worldgen.Biome)
    assert biome.name == 'ocean'  # 0 is ocean

    # Test gen_biomes
    biomes = g.gen_biomes(4, 0, 0, 2, 2)
    print(f"gen_biomes returned: {biomes!r}")
    assert isinstance(biomes, list)
    assert all(isinstance(b, mc_worldgen.Biome) for b in biomes)

    print("Enum returns PASSED")

if __name__ == "__main__":
    test_enum_returns()
