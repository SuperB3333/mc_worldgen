# Minecraft World Generation Python Package

This is a Python wrapper for the cubiomes library, providing Minecraft world generation capabilities.

## Installation

```bash
pip install .
```

## Usage

```python
import mc_worldgen

# Initialize the generator for a specific Minecraft version
g = mc_worldgen.Generator(mc=mc_worldgen.Version.MC_1_20)

# Apply a world seed and dimension
# DIM_NETHER = -1, DIM_OVERWORLD = 0, DIM_END = 1
g.apply_seed(mc_worldgen.Dimension.OVERWORLD, 123456789)

# Get a biome at specific block coordinates (scale 1)
biome_id = g.get_biome_at(1, x=100, y=64, z=200)
print(f"Biome ID at (100, 64, 200): {biome_id}")

# Compare with Biome constants
if biome_id == mc_worldgen.Biome.plains:
    print("It's a plains!")

# Generate biomes for a range at biome scale (scale 4)
# Returns a flat list of biome IDs
biomes = g.gen_biomes(scale=4, x=0, z=0, sx=16, sz=16)
```

## Constants

The module provides organized constants:
- `mc_worldgen.Version`: `MC_1_20`, `MC_1_21`, etc.
- `mc_worldgen.Biome`: `plains`, `desert`, `ocean`, etc.
- `mc_worldgen.Dimension`: `NETHER`, `OVERWORLD`, `END`.
- `mc_worldgen.Flag`: `LARGE_BIOMES`, etc.
