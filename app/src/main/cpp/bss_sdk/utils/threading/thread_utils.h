//
// Created by vnbk on 12/06/2023.
//

#ifndef LOCK_H
#define LOCK_H

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __linux
#endif

#define mutex sm_mutex

#define ENTER_CRITICAL(x)       ()
#define EXIT_CRITICAL(x)        ()

#ifdef __cplusplus
};
#endif

#endif //LOCK_H
