#ifndef FILTER_H
#define FILTER_H

#include "quad.h"

int isLeaf(Quadtree*, int, int);
int filtrage(Quadtree*, int, int, double, double);
double computeBlockVariance(double parentMean,const double childMeans[4],const double childVars[4]);
double calculateSigmaStart(Quadtree*);

#endif