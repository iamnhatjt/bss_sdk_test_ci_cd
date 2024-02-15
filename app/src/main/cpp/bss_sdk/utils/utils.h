//
// Created by vnbk on 18/08/2023.
//

#ifndef BSS_SDK_UTILS_H
#define BSS_SDK_UTILS_H

#include <cstdlib>

static inline int getRandom(){
    return rand();
}

static inline int getRandom(int _lower, int _upper){
    return _lower + (random()%(_upper+1));
}


#endif //BSS_SDK_UTILS_H
