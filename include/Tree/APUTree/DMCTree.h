#ifndef TREE_APUTREE_DMCTREE_H_
#define TREE_APUTREE_DMCTREE_H_

#include <Emulator.h>
#include <Tree/Tree.h>

typedef struct _DMCTree{
    TreeNode *dmc;
    struct{
        TreeNode *channel_enabled;
        TreeNode *irq_enabled;
        TreeNode *loop_flag;
        TreeNode *period;
        TreeNode *timer;
        TreeNode *sample_address;
        TreeNode *sample_length;

        TreeNode *read_memory;
        struct{
            TreeNode *sample_buffer_filled;
            TreeNode *sample_buffer;
            TreeNode *current_address;
            TreeNode *bytes_remaining;
        }read_memory_children;

        TreeNode *output_unit;
        struct{
            TreeNode *output_level;
            TreeNode *shift;
            TreeNode *bits_remaining;
            TreeNode *silence;
        }output_unit_children;
    }children;
}DMCTree;

void DMCTree_Init(DMCTree *dmc_tree,DMC *dmc,const uint16_t *name,Tree *tree,TreeNode *parent);

#endif