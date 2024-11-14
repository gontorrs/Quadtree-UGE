#include <stdio.h>
#include <stdlib.h>

typedef unsigned char graylvl;

typedef struct erruni{
    int e : 
}erruni;

typedef struct Pixnode{
    graylvl m;
    int n;
    int u;
}Pixnode;

typedef struct Quadtree{
    Pixnode* Pixels;
    int treesize;
}Quadtree;

int treelevel(int n){
    int res;
    int pow = 2;
    for(int i = 1; i<12;i++){
        pow = pow*i;
        if(pow == n){
            res = i+1;
            return res;
        }
    }
}

Quadtree* init_quadtree(int picture_size){

}