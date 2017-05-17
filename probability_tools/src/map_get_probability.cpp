#include "probability_tool.h"

/*****************************************************************************/
double map_get_probability1(PROB_MAP &probs, string num) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  ///**/fprintf(stdout, "in get_probability0\n");

  itr = probs.find(NULLSTRING); /* 분모를 찾는다. */
  
  if ( itr != probs.end() ) { /* 있으면 */
    itr2 = itr->second.find(num); /* 분자를 찾는다. */

    if (itr2 != itr->second.end() ) { /* 있으면 */
      return (itr2->second);
    }
    else { /* 분자가 없으면 */
      ///**/fprintf(stdout, "return 0\n");
      return ALMOST_ZERO; /* 거의 0 */
    }
  }
  else { /* 분모가 없으면 */
    ///**/fprintf(stdout, "return 0\n");
    return ALMOST_ZERO;
  }
}

/*****************************************************************************/
double map_get_probability2(PROB_MAP &probs, string num, string denom) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  ///**/fprintf(stdout, "in get_probability1\n");

  itr = probs.find(denom); /* 분모를 찾는다. */
  
  if ( itr != probs.end() ) { /* 있으면 */
    itr2 = itr->second.find(num); /* 분자를 찾는다. */

    if (itr2 != itr->second.end() ) { /* 있으면 */
      return (itr2->second);
    }
    else { /* 분자가 없으면 */
      return map_get_probability1(probs, num);
    }
  }
  else { /* 분모가 없으면 */
    return map_get_probability1(probs, num);
  }
}

/*****************************************************************************/
double map_get_probability3(PROB_MAP &probs, string num, string t1, string t2) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  ///**/fprintf(stdout, "in get_probability2\n");

  itr = probs.find(t1+t2); /* 분모를 찾는다. */

  if ( itr != probs.end() ) { /* 있으면 */
    itr2 = itr->second.find(num); /* 분자를 찾는다. */

    if (itr2 != itr->second.end() ) { /* 있으면 */
      return (itr2->second);
    }
    else { /* 분자가 없으면 */
      return map_get_probability2(probs, num, t2);
    }
  }
  else { /* 분모가 없으면 */
    return map_get_probability2(probs, num, t2);
  }
}

/*****************************************************************************/
double map_get_probability4(PROB_MAP &probs, string num, string t1, string t2, string t3) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  ///**/fprintf(stdout, "in get_probability3\n");

  itr = probs.find(t1+t2+t3); /* 분모를 찾는다. */

  if ( itr != probs.end() ) { /* 있으면 */
    itr2 = itr->second.find(num); /* 분자를 찾는다. */

    if (itr2 != itr->second.end() ) { /* 있으면 */
      return (itr2->second);
    }
    else { /* 분자가 없으면 */
      return map_get_probability3(probs, num, t2, t3);
    }
  }
  else { /* 분모가 없으면 */
    return map_get_probability3(probs, num, t2, t3);
  }
}

/*****************************************************************************/
/* num : 분자 */
/* t1+t2+t3+t4 : 분모 */
double map_get_probability5(PROB_MAP &probs, string num, string t1, string t2, string t3, string t4) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  itr = probs.find(t1+t2+t3+t4); /* 분모를 찾는다. */

  ///**/fprintf(stdout, "in get_probability4\n");

  if ( itr != probs.end() ) { /* 있으면 */
    itr2 = itr->second.find(num); /* 분자를 찾는다. */

    if (itr2 != itr->second.end() ) { /* 있으면 */
      return (itr2->second);
    }
    else { /* 분자가 없으면 */
      return map_get_probability4(probs, num, t2, t3, t4);
    }
  }
  else { /* 분모가 없으면 */
    return map_get_probability4(probs, num, t2, t3, t4);
  }
}


