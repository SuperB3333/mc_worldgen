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

static PyMethodDef Generator_methods[] = {
    {"apply_seed", (PyCFunction) Generator_apply_seed, METH_VARARGS | METH_KEYWORDS,
     "Apply a seed to the generator"},
    {"get_biome_at", (PyCFunction) Generator_get_biome_at, METH_VARARGS | METH_KEYWORDS,
     "Get biome at specific coordinates"},
    {"gen_biomes", (PyCFunction) Generator_gen_biomes, METH_VARARGS | METH_KEYWORDS,
     "Generate biomes for a range"},
    {NULL}  /* Sentinel */
};

static PyTypeObject GeneratorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mc_worldgen.Generator",
    .tp_doc = "Minecraft World Generator",
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
    .m_name = "mc_worldgen",
    .m_doc = "Python wrapper for cubiomes.",
    .m_size = -1,
};

#define ADD_INT_CONSTANT(module, name) PyModule_AddIntConstant(module, #name, name)

PyMODINIT_FUNC
PyInit_mc_worldgen(void)
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

    // Versions
    ADD_INT_CONSTANT(m, MC_B1_7);
    ADD_INT_CONSTANT(m, MC_B1_8);
    ADD_INT_CONSTANT(m, MC_1_0);
    ADD_INT_CONSTANT(m, MC_1_1);
    ADD_INT_CONSTANT(m, MC_1_2);
    ADD_INT_CONSTANT(m, MC_1_3);
    ADD_INT_CONSTANT(m, MC_1_4);
    ADD_INT_CONSTANT(m, MC_1_5);
    ADD_INT_CONSTANT(m, MC_1_6);
    ADD_INT_CONSTANT(m, MC_1_7);
    ADD_INT_CONSTANT(m, MC_1_8);
    ADD_INT_CONSTANT(m, MC_1_9);
    ADD_INT_CONSTANT(m, MC_1_10);
    ADD_INT_CONSTANT(m, MC_1_11);
    ADD_INT_CONSTANT(m, MC_1_12);
    ADD_INT_CONSTANT(m, MC_1_13);
    ADD_INT_CONSTANT(m, MC_1_14);
    ADD_INT_CONSTANT(m, MC_1_15);
    ADD_INT_CONSTANT(m, MC_1_16_1);
    ADD_INT_CONSTANT(m, MC_1_16);
    ADD_INT_CONSTANT(m, MC_1_17);
    ADD_INT_CONSTANT(m, MC_1_18);
    ADD_INT_CONSTANT(m, MC_1_19);
    ADD_INT_CONSTANT(m, MC_1_19_2);
    ADD_INT_CONSTANT(m, MC_1_19_4);
    ADD_INT_CONSTANT(m, MC_1_20);
    ADD_INT_CONSTANT(m, MC_1_21);
    ADD_INT_CONSTANT(m, MC_NEWEST);

    // Dimensions
    ADD_INT_CONSTANT(m, DIM_NETHER);
    ADD_INT_CONSTANT(m, DIM_OVERWORLD);
    ADD_INT_CONSTANT(m, DIM_END);

    // Flags
    ADD_INT_CONSTANT(m, LARGE_BIOMES);
    ADD_INT_CONSTANT(m, NO_BETA_OCEAN);
    ADD_INT_CONSTANT(m, FORCE_OCEAN_VARIANTS);

    // Biomes
    ADD_INT_CONSTANT(m, ocean);
    ADD_INT_CONSTANT(m, plains);
    ADD_INT_CONSTANT(m, desert);
    ADD_INT_CONSTANT(m, mountains);
    ADD_INT_CONSTANT(m, extremeHills);
    ADD_INT_CONSTANT(m, forest);
    ADD_INT_CONSTANT(m, taiga);
    ADD_INT_CONSTANT(m, swamp);
    ADD_INT_CONSTANT(m, swampland);
    ADD_INT_CONSTANT(m, river);
    ADD_INT_CONSTANT(m, nether_wastes);
    ADD_INT_CONSTANT(m, hell);
    ADD_INT_CONSTANT(m, the_end);
    ADD_INT_CONSTANT(m, sky);
    ADD_INT_CONSTANT(m, frozen_ocean);
    ADD_INT_CONSTANT(m, frozenOcean);
    ADD_INT_CONSTANT(m, frozen_river);
    ADD_INT_CONSTANT(m, frozenRiver);
    ADD_INT_CONSTANT(m, snowy_tundra);
    ADD_INT_CONSTANT(m, icePlains);
    ADD_INT_CONSTANT(m, snowy_mountains);
    ADD_INT_CONSTANT(m, iceMountains);
    ADD_INT_CONSTANT(m, mushroom_fields);
    ADD_INT_CONSTANT(m, mushroomIsland);
    ADD_INT_CONSTANT(m, mushroom_field_shore);
    ADD_INT_CONSTANT(m, mushroomIslandShore);
    ADD_INT_CONSTANT(m, beach);
    ADD_INT_CONSTANT(m, desert_hills);
    ADD_INT_CONSTANT(m, desertHills);
    ADD_INT_CONSTANT(m, wooded_hills);
    ADD_INT_CONSTANT(m, forestHills);
    ADD_INT_CONSTANT(m, taiga_hills);
    ADD_INT_CONSTANT(m, taigaHills);
    ADD_INT_CONSTANT(m, mountain_edge);
    ADD_INT_CONSTANT(m, extremeHillsEdge);
    ADD_INT_CONSTANT(m, jungle);
    ADD_INT_CONSTANT(m, jungle_hills);
    ADD_INT_CONSTANT(m, jungleHills);
    ADD_INT_CONSTANT(m, jungle_edge);
    ADD_INT_CONSTANT(m, jungleEdge);
    ADD_INT_CONSTANT(m, deep_ocean);
    ADD_INT_CONSTANT(m, deepOcean);
    ADD_INT_CONSTANT(m, stone_shore);
    ADD_INT_CONSTANT(m, stoneBeach);
    ADD_INT_CONSTANT(m, snowy_beach);
    ADD_INT_CONSTANT(m, coldBeach);
    ADD_INT_CONSTANT(m, birch_forest);
    ADD_INT_CONSTANT(m, birchForest);
    ADD_INT_CONSTANT(m, birch_forest_hills);
    ADD_INT_CONSTANT(m, birchForestHills);
    ADD_INT_CONSTANT(m, dark_forest);
    ADD_INT_CONSTANT(m, roofedForest);
    ADD_INT_CONSTANT(m, snowy_taiga);
    ADD_INT_CONSTANT(m, coldTaiga);
    ADD_INT_CONSTANT(m, snowy_taiga_hills);
    ADD_INT_CONSTANT(m, coldTaigaHills);
    ADD_INT_CONSTANT(m, giant_tree_taiga);
    ADD_INT_CONSTANT(m, megaTaiga);
    ADD_INT_CONSTANT(m, giant_tree_taiga_hills);
    ADD_INT_CONSTANT(m, megaTaigaHills);
    ADD_INT_CONSTANT(m, wooded_mountains);
    ADD_INT_CONSTANT(m, extremeHillsPlus);
    ADD_INT_CONSTANT(m, savanna);
    ADD_INT_CONSTANT(m, savanna_plateau);
    ADD_INT_CONSTANT(m, savannaPlateau);
    ADD_INT_CONSTANT(m, badlands);
    ADD_INT_CONSTANT(m, mesa);
    ADD_INT_CONSTANT(m, wooded_badlands_plateau);
    ADD_INT_CONSTANT(m, mesaPlateau_F);
    ADD_INT_CONSTANT(m, badlands_plateau);
    ADD_INT_CONSTANT(m, mesaPlateau);
    ADD_INT_CONSTANT(m, small_end_islands);
    ADD_INT_CONSTANT(m, end_midlands);
    ADD_INT_CONSTANT(m, end_highlands);
    ADD_INT_CONSTANT(m, end_barrens);
    ADD_INT_CONSTANT(m, warm_ocean);
    ADD_INT_CONSTANT(m, warmOcean);
    ADD_INT_CONSTANT(m, lukewarm_ocean);
    ADD_INT_CONSTANT(m, lukewarmOcean);
    ADD_INT_CONSTANT(m, cold_ocean);
    ADD_INT_CONSTANT(m, coldOcean);
    ADD_INT_CONSTANT(m, deep_warm_ocean);
    ADD_INT_CONSTANT(m, warmDeepOcean);
    ADD_INT_CONSTANT(m, deep_lukewarm_ocean);
    ADD_INT_CONSTANT(m, lukewarmDeepOcean);
    ADD_INT_CONSTANT(m, deep_cold_ocean);
    ADD_INT_CONSTANT(m, coldDeepOcean);
    ADD_INT_CONSTANT(m, deep_frozen_ocean);
    ADD_INT_CONSTANT(m, frozenDeepOcean);
    ADD_INT_CONSTANT(m, seasonal_forest);
    ADD_INT_CONSTANT(m, rainforest);
    ADD_INT_CONSTANT(m, shrubland);
    ADD_INT_CONSTANT(m, the_void);
    ADD_INT_CONSTANT(m, sunflower_plains);
    ADD_INT_CONSTANT(m, desert_lakes);
    ADD_INT_CONSTANT(m, gravelly_mountains);
    ADD_INT_CONSTANT(m, flower_forest);
    ADD_INT_CONSTANT(m, taiga_mountains);
    ADD_INT_CONSTANT(m, swamp_hills);
    ADD_INT_CONSTANT(m, ice_spikes);
    ADD_INT_CONSTANT(m, modified_jungle);
    ADD_INT_CONSTANT(m, modified_jungle_edge);
    ADD_INT_CONSTANT(m, tall_birch_forest);
    ADD_INT_CONSTANT(m, tall_birch_hills);
    ADD_INT_CONSTANT(m, dark_forest_hills);
    ADD_INT_CONSTANT(m, snowy_taiga_mountains);
    ADD_INT_CONSTANT(m, giant_spruce_taiga);
    ADD_INT_CONSTANT(m, giant_spruce_taiga_hills);
    ADD_INT_CONSTANT(m, modified_gravelly_mountains);
    ADD_INT_CONSTANT(m, shattered_savanna);
    ADD_INT_CONSTANT(m, shattered_savanna_plateau);
    ADD_INT_CONSTANT(m, eroded_badlands);
    ADD_INT_CONSTANT(m, modified_wooded_badlands_plateau);
    ADD_INT_CONSTANT(m, modified_badlands_plateau);
    ADD_INT_CONSTANT(m, bamboo_jungle);
    ADD_INT_CONSTANT(m, bamboo_jungle_hills);
    ADD_INT_CONSTANT(m, soul_sand_valley);
    ADD_INT_CONSTANT(m, crimson_forest);
    ADD_INT_CONSTANT(m, warped_forest);
    ADD_INT_CONSTANT(m, basalt_deltas);
    ADD_INT_CONSTANT(m, dripstone_caves);
    ADD_INT_CONSTANT(m, lush_caves);
    ADD_INT_CONSTANT(m, meadow);
    ADD_INT_CONSTANT(m, grove);
    ADD_INT_CONSTANT(m, snowy_slopes);
    ADD_INT_CONSTANT(m, jagged_peaks);
    ADD_INT_CONSTANT(m, frozen_peaks);
    ADD_INT_CONSTANT(m, stony_peaks);
    ADD_INT_CONSTANT(m, old_growth_birch_forest);
    ADD_INT_CONSTANT(m, old_growth_pine_taiga);
    ADD_INT_CONSTANT(m, old_growth_spruce_taiga);
    ADD_INT_CONSTANT(m, snowy_plains);
    ADD_INT_CONSTANT(m, sparse_jungle);
    ADD_INT_CONSTANT(m, stony_shore);
    ADD_INT_CONSTANT(m, windswept_hills);
    ADD_INT_CONSTANT(m, windswept_forest);
    ADD_INT_CONSTANT(m, windswept_gravelly_hills);
    ADD_INT_CONSTANT(m, windswept_savanna);
    ADD_INT_CONSTANT(m, wooded_badlands);
    ADD_INT_CONSTANT(m, deep_dark);
    ADD_INT_CONSTANT(m, mangrove_swamp);
    ADD_INT_CONSTANT(m, cherry_grove);
    ADD_INT_CONSTANT(m, pale_garden);

    return m;
}
