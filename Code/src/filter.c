#include "filter.h"

int isLeaf(Quadtree* tree, int i, int level) {
    if (level == tree->levels - 1) return 1;
    if(tree->Pixels[i].u == 1) return 1;

    int firstChild = 4 * i + 1;
    if (firstChild >= tree->treesize) {
        return 1;
    }
    return 0;
}

int filtrage(Quadtree* tree, int i, int level, double sigma, double alpha)
{
    // 1. If node is already uniform, we consider it uniform
    if (tree->Pixels[i].u == 1) {
        return 1;
    }

    // 2. If this node is a leaf (no children below)
    if (isLeaf(tree, i, level)) {
        return 1;
    }

    // 3. Descend into 4 children (if they exist)
    int s = 0;  // how many children ended up uniform
    for (int childNum = 1; childNum <= 4; childNum++) {
        int childIndex = 4*i + childNum;
        if (childIndex < tree->treesize) {
            // Recursively filter child
            s += filtrage(tree, childIndex, level + 1, sigma * alpha, alpha);
        }
    }

    // 4. We unify the current node if:
    //    - All 4 children are uniform (s == 4)
    //    - The node's variance <= sigma
    double var = tree->Pixels[i].variance; // or from your parallel array
    if ((s < 4) || (var > sigma)) {
        return 0; // remain non-uniform
    }

    // 5. Force uniformity for this node
    tree->Pixels[i].u = 1;
    tree->Pixels[i].e = 0;  // you can decide how to handle e
    // m remains whatever was computed 
    return 1;
}

double computeBlockVariance(double parentMean,const double childMeans[4],const double childVars[4])
{
    double mu = 0.0;
    for (int k = 0; k < 4; k++) {
        double diff = parentMean - childMeans[k];
        mu += (childVars[k] * childVars[k]) + (diff * diff);
    }
    return sqrt(mu) / 4.0; 
}

double calculateSigmaStart(Quadtree* tree){
    double totalVar = 0.0;
    double maxVar   = 0.0;

    for (int i = 0; i < tree->treesize; i++) {
        double v = tree->Pixels[i].variance;
        totalVar += v;
        if (v > maxVar) {
            maxVar = v;
        }
    }

    double medvar = totalVar / (double)tree->treesize;


    double sigma_start = medvar / maxVar; 
    return sigma_start;
}