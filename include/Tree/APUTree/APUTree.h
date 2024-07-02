#ifndef TREE_APUTREE_APUTREE_H_
#define TREE_APUTREE_APUTREE_H_

#include <Emulator.h>
#include <Tree/APUTree/PulseTree.h>
#include <Tree/APUTree/TriangleTree.h>
#include <Tree/APUTree/NoiseTree.h>
#include <Tree/APUTree/DMCTree.h>
#include <Tree/APUTree/FrameCounterTree.h>
#include <Tree/Tree.h>

typedef struct _APUTree{
    Tree *tree;

    PulseTree pulse1;

    PulseTree pulse2;

    TriangleTree triangle;

    NoiseTree noise;

    DMCTree dmc;

    FrameCounterTree frame_counter;
}APUTree;

APUTree* APUTree_Create(APU *apu,TreeResources *resources,WG_FontAtlas *atlas);

void APUTree_Free(APUTree *apu_tree);

#endif