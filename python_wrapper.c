#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "generator.h"
#include "biomes.h"
#include "util.h"
#include "structmember.h"

typedef struct {
    PyObject_HEAD
    Generator g;
} GeneratorObject;

static void
Generator_dealloc(GeneratorObject *self)
{
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
Generator_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    GeneratorObject *self;
    self = (GeneratorObject *) type->tp_alloc(type, 0);
    if (self == NULL) {
        return NULL;
    }
    return (PyObject *) self;
}

static int
Generator_init(GeneratorObject *self, PyObject *args, PyObject *kwds)
{
    int mc = MC_NEWEST;
    unsigned int flags = 0;
    static char *kwlist[] = {"mc", "flags", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iI", kwlist,
                                     &mc, &flags))
        return -1;

    setupGenerator(&self->g, mc, flags);
    return 0;
}

static PyObject *
Generator_apply_seed(GeneratorObject *self, PyObject *args, PyObject *kwds)
{
    int dim;
    unsigned long long seed;
    static char *kwlist[] = {"dim", "seed", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "iK", kwlist,
                                     &dim, &seed))
        return NULL;

    applySeed(&self->g, dim, seed);
    Py_RETURN_NONE;
}

static PyObject *
Generator_get_biome_at(GeneratorObject *self, PyObject *args, PyObject *kwds)
{
    int scale, x, y, z;
    static char *kwlist[] = {"scale", "x", "y", "z", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "iiii", kwlist,
                                     &scale, &x, &y, &z))
        return NULL;

    int biome = getBiomeAt(&self->g, scale, x, y, z);
    return PyLong_FromLong(biome);
}

static PyObject *
Generator_gen_biomes(GeneratorObject *self, PyObject *args, PyObject *kwds)
{
    int scale, x, z, sx, sz, y = 0, sy = 1;
    static char *kwlist[] = {"scale", "x", "z", "sx", "sz", "y", "sy", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "iiiii|ii", kwlist,
                                     &scale, &x, &z, &sx, &sz, &y, &sy))
        return NULL;

    Range r;
    r.scale = scale;
    r.x = x;
    r.z = z;
    r.sx = sx;
    r.sz = sz;
    r.y = y;
    r.sy = sy;

    size_t cacheSize = getMinCacheSize(&self->g, scale, sx, sy, sz);
    int *cache = (int *)malloc(cacheSize * sizeof(int));
    if (cache == NULL) {
        return PyErr_NoMemory();
    }

    if (genBiomes(&self->g, cache, r) != 0) {
        free(cache);
        PyErr_SetString(PyExc_RuntimeError, "genBiomes failed");
        return NULL;
    }

    PyObject *list = PyList_New(cacheSize);
    if (!list) {
        free(cache);
        return NULL;
    }

    for (size_t i = 0; i < cacheSize; i++) {
        PyObject *val = PyLong_FromLong(cache[i]);
        if (!val) {
            Py_DECREF(list);
            free(cache);
            return NULL;
        }
        PyList_SET_ITEM(list, i, val);
    }

    free(cache);
    return list;
}

PyDoc_STRVAR(apply_seed_doc,
"apply_seed(dim, seed)\n"
"--\n\n"
"Apply a world seed and dimension to the generator.\n\n"
"Args:\n"
"    dim (int): The dimension (Dimension.OVERWORLD, Dimension.NETHER, Dimension.END).\n"
"    seed (int): The 64-bit integer world seed.");

PyDoc_STRVAR(get_biome_at_doc,
"get_biome_at(scale, x, y, z)\n"
"--\n\n"
"Get the biome ID at a specific coordinate and scale.\n\n"
"Args:\n"
"    scale (int): The horizontal scale factor (1, 4, 16, 64, or 256).\n"
"    x (int): The X coordinate.\n"
"    y (int): The Y coordinate.\n"
"    z (int): The Z coordinate.\n\n"
"Returns:\n"
"    int: The Biome ID.");

PyDoc_STRVAR(gen_biomes_doc,
"gen_biomes(scale, x, z, sx, sz, y=0, sy=1)\n"
"--\n\n"
"Generate biomes for a given range.\n\n"
"Args:\n"
"    scale (int): The horizontal scale factor (1, 4, 16, 64, or 256).\n"
"    x (int): The starting X coordinate.\n"
"    z (int): The starting Z coordinate.\n"
"    sx (int): The horizontal size in X direction.\n"
"    sz (int): The horizontal size in Z direction.\n"
"    y (int, optional): The starting Y coordinate. Defaults to 0.\n"
"    sy (int, optional): The vertical size. Defaults to 1.\n\n"
"Returns:\n"
"    list[int]: A flat list of biome IDs.");

static PyMethodDef Generator_methods[] = {
    {"apply_seed", (PyCFunction) Generator_apply_seed, METH_VARARGS | METH_KEYWORDS,
     apply_seed_doc},
    {"get_biome_at", (PyCFunction) Generator_get_biome_at, METH_VARARGS | METH_KEYWORDS,
     get_biome_at_doc},
    {"gen_biomes", (PyCFunction) Generator_gen_biomes, METH_VARARGS | METH_KEYWORDS,
     gen_biomes_doc},
    {NULL}  /* Sentinel */
};

PyDoc_STRVAR(generator_doc,
"Generator(mc=Version.MC_NEWEST, flags=0)\n"
"--\n\n"
"Minecraft World Generator object.\n\n"
"Args:\n"
"    mc (int): Minecraft version (use Version constants).\n"
"    flags (int): Generator flags (use Flag constants).");

static PyTypeObject GeneratorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mc_worldgen.Generator",
    .tp_doc = generator_doc,
    .tp_basicsize = sizeof(GeneratorObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Generator_new,
    .tp_init = (initproc) Generator_init,
    .tp_dealloc = (destructor) Generator_dealloc,
    .tp_methods = Generator_methods,
};

static PyModuleDef mc_worldgenmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "mc_worldgen._mc_worldgen",
    .m_doc = "Python wrapper for cubiomes Minecraft world generation library.",
    .m_size = -1,
};

static PyObject* create_enum_class(const char* name, const char* doc) {
    PyObject* dict = PyDict_New();
    PyObject* doc_str = PyUnicode_FromString(doc);
    PyDict_SetItemString(dict, "__doc__", doc_str);
    Py_DECREF(doc_str);

    PyObject* metaclass = (PyObject*)&PyType_Type;
    PyObject* bases = PyTuple_Pack(1, &PyBaseObject_Type);
    PyObject* args = Py_BuildValue("sOO", name, bases, dict);
    PyObject* cls = PyObject_Call(metaclass, args, NULL);
    Py_DECREF(dict);
    Py_DECREF(bases);
    Py_DECREF(args);
    return cls;
}

#define ADD_TO_CLASS(cls, name, val) \
    do { \
        PyObject *o = PyLong_FromLong(val); \
        if (o) { \
            PyObject_SetAttrString(cls, #name, o); \
            Py_DECREF(o); \
        } \
    } while(0)

PyMODINIT_FUNC
PyInit__mc_worldgen(void)
{
    PyObject *m;
    if (PyType_Ready(&GeneratorType) < 0)
        return NULL;

    m = PyModule_Create(&mc_worldgenmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&GeneratorType);
    if (PyModule_AddObject(m, "Generator", (PyObject *) &GeneratorType) < 0) {
        Py_DECREF(&GeneratorType);
        Py_DECREF(m);
        return NULL;
    }

    PyObject *Version = create_enum_class("Version", "Minecraft versions");
    PyModule_AddObject(m, "Version", Version);
    ADD_TO_CLASS(Version, MC_B1_7, MC_B1_7);
    ADD_TO_CLASS(Version, MC_B1_8, MC_B1_8);
    ADD_TO_CLASS(Version, MC_1_0, MC_1_0);
    ADD_TO_CLASS(Version, MC_1_1, MC_1_1);
    ADD_TO_CLASS(Version, MC_1_2, MC_1_2);
    ADD_TO_CLASS(Version, MC_1_3, MC_1_3);
    ADD_TO_CLASS(Version, MC_1_4, MC_1_4);
    ADD_TO_CLASS(Version, MC_1_5, MC_1_5);
    ADD_TO_CLASS(Version, MC_1_6, MC_1_6);
    ADD_TO_CLASS(Version, MC_1_7, MC_1_7);
    ADD_TO_CLASS(Version, MC_1_8, MC_1_8);
    ADD_TO_CLASS(Version, MC_1_9, MC_1_9);
    ADD_TO_CLASS(Version, MC_1_10, MC_1_10);
    ADD_TO_CLASS(Version, MC_1_11, MC_1_11);
    ADD_TO_CLASS(Version, MC_1_12, MC_1_12);
    ADD_TO_CLASS(Version, MC_1_13, MC_1_13);
    ADD_TO_CLASS(Version, MC_1_14, MC_1_14);
    ADD_TO_CLASS(Version, MC_1_15, MC_1_15);
    ADD_TO_CLASS(Version, MC_1_16_1, MC_1_16_1);
    ADD_TO_CLASS(Version, MC_1_16, MC_1_16);
    ADD_TO_CLASS(Version, MC_1_17, MC_1_17);
    ADD_TO_CLASS(Version, MC_1_18, MC_1_18);
    ADD_TO_CLASS(Version, MC_1_19, MC_1_19);
    ADD_TO_CLASS(Version, MC_1_19_2, MC_1_19_2);
    ADD_TO_CLASS(Version, MC_1_19_4, MC_1_19_4);
    ADD_TO_CLASS(Version, MC_1_20, MC_1_20);
    ADD_TO_CLASS(Version, MC_1_21, MC_1_21);
    ADD_TO_CLASS(Version, MC_NEWEST, MC_NEWEST);

    PyObject *Dimension = create_enum_class("Dimension", "Minecraft dimensions");
    PyModule_AddObject(m, "Dimension", Dimension);
    ADD_TO_CLASS(Dimension, NETHER, DIM_NETHER);
    ADD_TO_CLASS(Dimension, OVERWORLD, DIM_OVERWORLD);
    ADD_TO_CLASS(Dimension, END, DIM_END);

    PyObject *Flag = create_enum_class("Flag", "Generator flags");
    PyModule_AddObject(m, "Flag", Flag);
    ADD_TO_CLASS(Flag, LARGE_BIOMES, LARGE_BIOMES);
    ADD_TO_CLASS(Flag, NO_BETA_OCEAN, NO_BETA_OCEAN);
    ADD_TO_CLASS(Flag, FORCE_OCEAN_VARIANTS, FORCE_OCEAN_VARIANTS);

    PyObject *Biome = create_enum_class("Biome", "Biome IDs");
    PyModule_AddObject(m, "Biome", Biome);
    ADD_TO_CLASS(Biome, ocean, ocean);
    ADD_TO_CLASS(Biome, plains, plains);
    ADD_TO_CLASS(Biome, desert, desert);
    ADD_TO_CLASS(Biome, mountains, mountains);
    ADD_TO_CLASS(Biome, extremeHills, extremeHills);
    ADD_TO_CLASS(Biome, forest, forest);
    ADD_TO_CLASS(Biome, taiga, taiga);
    ADD_TO_CLASS(Biome, swamp, swamp);
    ADD_TO_CLASS(Biome, swampland, swampland);
    ADD_TO_CLASS(Biome, river, river);
    ADD_TO_CLASS(Biome, nether_wastes, nether_wastes);
    ADD_TO_CLASS(Biome, hell, hell);
    ADD_TO_CLASS(Biome, the_end, the_end);
    ADD_TO_CLASS(Biome, sky, sky);
    ADD_TO_CLASS(Biome, frozen_ocean, frozen_ocean);
    ADD_TO_CLASS(Biome, frozenOcean, frozenOcean);
    ADD_TO_CLASS(Biome, frozen_river, frozen_river);
    ADD_TO_CLASS(Biome, frozenRiver, frozenRiver);
    ADD_TO_CLASS(Biome, snowy_tundra, snowy_tundra);
    ADD_TO_CLASS(Biome, icePlains, icePlains);
    ADD_TO_CLASS(Biome, snowy_mountains, snowy_mountains);
    ADD_TO_CLASS(Biome, iceMountains, iceMountains);
    ADD_TO_CLASS(Biome, mushroom_fields, mushroom_fields);
    ADD_TO_CLASS(Biome, mushroomIsland, mushroomIsland);
    ADD_TO_CLASS(Biome, mushroom_field_shore, mushroom_field_shore);
    ADD_TO_CLASS(Biome, mushroomIslandShore, mushroomIslandShore);
    ADD_TO_CLASS(Biome, beach, beach);
    ADD_TO_CLASS(Biome, desert_hills, desert_hills);
    ADD_TO_CLASS(Biome, desertHills, desertHills);
    ADD_TO_CLASS(Biome, wooded_hills, wooded_hills);
    ADD_TO_CLASS(Biome, forestHills, forestHills);
    ADD_TO_CLASS(Biome, taiga_hills, taiga_hills);
    ADD_TO_CLASS(Biome, taigaHills, taigaHills);
    ADD_TO_CLASS(Biome, mountain_edge, mountain_edge);
    ADD_TO_CLASS(Biome, extremeHillsEdge, extremeHillsEdge);
    ADD_TO_CLASS(Biome, jungle, jungle);
    ADD_TO_CLASS(Biome, jungle_hills, jungle_hills);
    ADD_TO_CLASS(Biome, jungleHills, jungleHills);
    ADD_TO_CLASS(Biome, jungle_edge, jungle_edge);
    ADD_TO_CLASS(Biome, jungleEdge, jungleEdge);
    ADD_TO_CLASS(Biome, deep_ocean, deep_ocean);
    ADD_TO_CLASS(Biome, deepOcean, deepOcean);
    ADD_TO_CLASS(Biome, stone_shore, stone_shore);
    ADD_TO_CLASS(Biome, stoneBeach, stoneBeach);
    ADD_TO_CLASS(Biome, snowy_beach, snowy_beach);
    ADD_TO_CLASS(Biome, coldBeach, coldBeach);
    ADD_TO_CLASS(Biome, birch_forest, birch_forest);
    ADD_TO_CLASS(Biome, birchForest, birchForest);
    ADD_TO_CLASS(Biome, birch_forest_hills, birch_forest_hills);
    ADD_TO_CLASS(Biome, birchForestHills, birchForestHills);
    ADD_TO_CLASS(Biome, dark_forest, dark_forest);
    ADD_TO_CLASS(Biome, roofedForest, roofedForest);
    ADD_TO_CLASS(Biome, snowy_taiga, snowy_taiga);
    ADD_TO_CLASS(Biome, coldTaiga, coldTaiga);
    ADD_TO_CLASS(Biome, snowy_taiga_hills, snowy_taiga_hills);
    ADD_TO_CLASS(Biome, coldTaigaHills, coldTaigaHills);
    ADD_TO_CLASS(Biome, giant_tree_taiga, giant_tree_taiga);
    ADD_TO_CLASS(Biome, megaTaiga, megaTaiga);
    ADD_TO_CLASS(Biome, giant_tree_taiga_hills, giant_tree_taiga_hills);
    ADD_TO_CLASS(Biome, megaTaigaHills, megaTaigaHills);
    ADD_TO_CLASS(Biome, wooded_mountains, wooded_mountains);
    ADD_TO_CLASS(Biome, extremeHillsPlus, extremeHillsPlus);
    ADD_TO_CLASS(Biome, savanna, savanna);
    ADD_TO_CLASS(Biome, savanna_plateau, savanna_plateau);
    ADD_TO_CLASS(Biome, savannaPlateau, savannaPlateau);
    ADD_TO_CLASS(Biome, badlands, badlands);
    ADD_TO_CLASS(Biome, mesa, mesa);
    ADD_TO_CLASS(Biome, wooded_badlands_plateau, wooded_badlands_plateau);
    ADD_TO_CLASS(Biome, mesaPlateau_F, mesaPlateau_F);
    ADD_TO_CLASS(Biome, badlands_plateau, badlands_plateau);
    ADD_TO_CLASS(Biome, mesaPlateau, mesaPlateau);
    ADD_TO_CLASS(Biome, small_end_islands, small_end_islands);
    ADD_TO_CLASS(Biome, end_midlands, end_midlands);
    ADD_TO_CLASS(Biome, end_highlands, end_highlands);
    ADD_TO_CLASS(Biome, end_barrens, end_barrens);
    ADD_TO_CLASS(Biome, warm_ocean, warm_ocean);
    ADD_TO_CLASS(Biome, warmOcean, warmOcean);
    ADD_TO_CLASS(Biome, lukewarm_ocean, lukewarm_ocean);
    ADD_TO_CLASS(Biome, lukewarmOcean, lukewarmOcean);
    ADD_TO_CLASS(Biome, cold_ocean, cold_ocean);
    ADD_TO_CLASS(Biome, coldOcean, coldOcean);
    ADD_TO_CLASS(Biome, deep_warm_ocean, deep_warm_ocean);
    ADD_TO_CLASS(Biome, warmDeepOcean, warmDeepOcean);
    ADD_TO_CLASS(Biome, deep_lukewarm_ocean, deep_lukewarm_ocean);
    ADD_TO_CLASS(Biome, lukewarmDeepOcean, lukewarmDeepOcean);
    ADD_TO_CLASS(Biome, deep_cold_ocean, deep_cold_ocean);
    ADD_TO_CLASS(Biome, coldDeepOcean, coldDeepOcean);
    ADD_TO_CLASS(Biome, deep_frozen_ocean, deep_frozen_ocean);
    ADD_TO_CLASS(Biome, frozenDeepOcean, frozenDeepOcean);
    ADD_TO_CLASS(Biome, seasonal_forest, seasonal_forest);
    ADD_TO_CLASS(Biome, rainforest, rainforest);
    ADD_TO_CLASS(Biome, shrubland, shrubland);
    ADD_TO_CLASS(Biome, the_void, the_void);
    ADD_TO_CLASS(Biome, sunflower_plains, sunflower_plains);
    ADD_TO_CLASS(Biome, desert_lakes, desert_lakes);
    ADD_TO_CLASS(Biome, gravelly_mountains, gravelly_mountains);
    ADD_TO_CLASS(Biome, flower_forest, flower_forest);
    ADD_TO_CLASS(Biome, taiga_mountains, taiga_mountains);
    ADD_TO_CLASS(Biome, swamp_hills, swamp_hills);
    ADD_TO_CLASS(Biome, ice_spikes, ice_spikes);
    ADD_TO_CLASS(Biome, modified_jungle, modified_jungle);
    ADD_TO_CLASS(Biome, modified_jungle_edge, modified_jungle_edge);
    ADD_TO_CLASS(Biome, tall_birch_forest, tall_birch_forest);
    ADD_TO_CLASS(Biome, tall_birch_hills, tall_birch_hills);
    ADD_TO_CLASS(Biome, dark_forest_hills, dark_forest_hills);
    ADD_TO_CLASS(Biome, snowy_taiga_mountains, snowy_taiga_mountains);
    ADD_TO_CLASS(Biome, giant_spruce_taiga, giant_spruce_taiga);
    ADD_TO_CLASS(Biome, giant_spruce_taiga_hills, giant_spruce_taiga_hills);
    ADD_TO_CLASS(Biome, modified_gravelly_mountains, modified_gravelly_mountains);
    ADD_TO_CLASS(Biome, shattered_savanna, shattered_savanna);
    ADD_TO_CLASS(Biome, shattered_savanna_plateau, shattered_savanna_plateau);
    ADD_TO_CLASS(Biome, eroded_badlands, eroded_badlands);
    ADD_TO_CLASS(Biome, modified_wooded_badlands_plateau, modified_wooded_badlands_plateau);
    ADD_TO_CLASS(Biome, modified_badlands_plateau, modified_badlands_plateau);
    ADD_TO_CLASS(Biome, bamboo_jungle, bamboo_jungle);
    ADD_TO_CLASS(Biome, bamboo_jungle_hills, bamboo_jungle_hills);
    ADD_TO_CLASS(Biome, soul_sand_valley, soul_sand_valley);
    ADD_TO_CLASS(Biome, crimson_forest, crimson_forest);
    ADD_TO_CLASS(Biome, warped_forest, warped_forest);
    ADD_TO_CLASS(Biome, basalt_deltas, basalt_deltas);
    ADD_TO_CLASS(Biome, dripstone_caves, dripstone_caves);
    ADD_TO_CLASS(Biome, lush_caves, lush_caves);
    ADD_TO_CLASS(Biome, meadow, meadow);
    ADD_TO_CLASS(Biome, grove, grove);
    ADD_TO_CLASS(Biome, snowy_slopes, snowy_slopes);
    ADD_TO_CLASS(Biome, jagged_peaks, jagged_peaks);
    ADD_TO_CLASS(Biome, frozen_peaks, frozen_peaks);
    ADD_TO_CLASS(Biome, stony_peaks, stony_peaks);
    ADD_TO_CLASS(Biome, old_growth_birch_forest, old_growth_birch_forest);
    ADD_TO_CLASS(Biome, old_growth_pine_taiga, old_growth_pine_taiga);
    ADD_TO_CLASS(Biome, old_growth_spruce_taiga, old_growth_spruce_taiga);
    ADD_TO_CLASS(Biome, snowy_plains, snowy_plains);
    ADD_TO_CLASS(Biome, sparse_jungle, sparse_jungle);
    ADD_TO_CLASS(Biome, stony_shore, stony_shore);
    ADD_TO_CLASS(Biome, windswept_hills, windswept_hills);
    ADD_TO_CLASS(Biome, windswept_forest, windswept_forest);
    ADD_TO_CLASS(Biome, windswept_gravelly_hills, windswept_gravelly_hills);
    ADD_TO_CLASS(Biome, windswept_savanna, windswept_savanna);
    ADD_TO_CLASS(Biome, wooded_badlands, wooded_badlands);
    ADD_TO_CLASS(Biome, deep_dark, deep_dark);
    ADD_TO_CLASS(Biome, mangrove_swamp, mangrove_swamp);
    ADD_TO_CLASS(Biome, cherry_grove, cherry_grove);
    ADD_TO_CLASS(Biome, pale_garden, pale_garden);

    return m;
}
