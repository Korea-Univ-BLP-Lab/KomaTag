#ifndef _ME_NEOHANTAG_H_
#define _ME_NEOHANTAG_H_

#include "maxentmodel.hpp"
using namespace maxent;

extern int me_neohantag_open(MaxentModel &head_m, MaxentModel &tail_m, 
                             MaxentModel &dummy_head_m,
                             WORD_FREQ &full_morpheme_map);


#endif
