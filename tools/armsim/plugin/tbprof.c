// QEMU TCG plugin for the ARMSIM build: counts how often each translated block runs. At exit
// it writes one line per block, "start end n_insns count" (addresses in hex, start and end are
// the first and last instruction). tools/armsim/profile.py turns this into per-function
// instruction and estimated cycle counts. Built and used by tools/armsim/profile.sh (it lives in
// its own folder because make ARMSIM=1 compiles every .c file in tools/armsim).
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <qemu-plugin.h>

QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

struct block {
    uint64_t start, end;
    uint32_t n_insns;
    struct qemu_plugin_scoreboard *count;
};

static GHashTable *blocks;
static GMutex lock;
static char out_path[1024] = "tbprof.txt";

static guint block_hash(gconstpointer p) {
    const struct block *b = p;
    return (guint) (b->start ^ (b->end * 0x9E3779B1u));
}

static gboolean block_equal(gconstpointer a, gconstpointer b) {
    const struct block *x = a, *y = b;
    return x->start == y->start && x->end == y->end;
}

static void tb_trans(struct qemu_plugin_tb *tb, void *userdata) {
    const size_t n = qemu_plugin_tb_n_insns(tb);
    if (n == 0) {
        return;
    }
    struct block key = {
        .start = qemu_plugin_tb_vaddr(tb),
        .end = qemu_plugin_insn_vaddr(qemu_plugin_tb_get_insn(tb, n - 1)),
    };
    g_mutex_lock(&lock);
    struct block *b = g_hash_table_lookup(blocks, &key);
    if (b == NULL) {
        b = g_new0(struct block, 1);
        *b = key;
        b->n_insns = (uint32_t) n;
        b->count = qemu_plugin_scoreboard_new(sizeof(uint64_t));
        g_hash_table_add(blocks, b);
    }
    g_mutex_unlock(&lock);
    qemu_plugin_register_vcpu_tb_exec_inline_per_vcpu(tb, QEMU_PLUGIN_INLINE_ADD_U64,
                                                      qemu_plugin_scoreboard_u64(b->count), 1);
}

static void at_exit(void *userdata) {
    FILE *f = fopen(out_path, "w");
    if (f == NULL) {
        qemu_plugin_outs("tbprof: can't write the output file\n");
        return;
    }
    GHashTableIter it;
    gpointer k;
    g_hash_table_iter_init(&it, blocks);
    while (g_hash_table_iter_next(&it, &k, NULL)) {
        const struct block *b = k;
        const uint64_t count = qemu_plugin_u64_sum(qemu_plugin_scoreboard_u64(b->count));
        if (count != 0) {
            fprintf(f, "%" PRIx64 " %" PRIx64 " %" PRIu32 " %" PRIu64 "\n", b->start, b->end,
                    b->n_insns, count);
        }
    }
    fclose(f);
}

QEMU_PLUGIN_EXPORT int qemu_plugin_install(qemu_plugin_id_t id, const qemu_info_t *info,
                                           int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        if (strncmp(argv[i], "out=", 4) == 0) {
            g_strlcpy(out_path, argv[i] + 4, sizeof(out_path));
        }
    }
    blocks = g_hash_table_new(block_hash, block_equal);
    qemu_plugin_register_vcpu_tb_trans_cb(id, tb_trans, NULL);
    qemu_plugin_register_atexit_cb(id, at_exit, NULL);
    return 0;
}
