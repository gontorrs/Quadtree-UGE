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
    if (tree->Pixels[i].u == 1) {
        return 1;
    }

    if (isLeaf(tree, i, level)) {
        return 1;
    }

    int s = 0;
    for (int childNum = 1; childNum <= 4; childNum++) {
        int childIndex = 4*i + childNum;
        if (childIndex < tree->treesize) {
            s += filtrage(tree, childIndex, level + 1, sigma * alpha, alpha);
        }
    }

    double var = tree->Pixels[i].variance;
    if ((s < 4) || (var > sigma)) {
        return 0;
    }
    
    tree->Pixels[i].u = 1;
    tree->Pixels[i].e = 0;
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
    printf("totalvar : %f tree size: %lld\n", totalVar,tree->treesize);
    double medvar = totalVar / (double)tree->treesize;


    double sigma_start = medvar / maxVar; 
    return sigma_start;
}