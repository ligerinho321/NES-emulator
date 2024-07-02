#ifndef TREE_APUTREE_ENVELOPETREE_H_
#define TREE_APUTREE_ENVELOPETREE_H_

#include <Emulator.h>
#include <Tree/Tree.h>

typedef struct _EnvelopeTree{
    TreeNode *envelope;
    struct{
        TreeNode *start_flag;
        TreeNode *devider;
        TreeNode *counter;
        TreeNode *loop_flag;
        TreeNode *constant_volume;
        TreeNode *volume;
    }children;
}EnvelopeTree;

void EnvelopeTree_Init(EnvelopeTree *envelope_tree,Envelope *envelope,const uint16_t *name,Tree *tree,TreeNode *parent);

#endif