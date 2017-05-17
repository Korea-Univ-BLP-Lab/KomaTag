#include "probability_tool.h"

/*****************************************************************************/
double map_get_probability1(PROB_MAP &probs, string num) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  ///**/fprintf(stdout, "in get_probability0\n");

  itr = probs.find(NULLSTRING); /* �и� ã�´�. */
  
  if ( itr != probs.end() ) { /* ������ */
    itr2 = itr->second.find(num); /* ���ڸ� ã�´�. */

    if (itr2 != itr->second.end() ) { /* ������ */
      return (itr2->second);
    }
    else { /* ���ڰ� ������ */
      ///**/fprintf(stdout, "return 0\n");
      return ALMOST_ZERO; /* ���� 0 */
    }
  }
  else { /* �и� ������ */
    ///**/fprintf(stdout, "return 0\n");
    return ALMOST_ZERO;
  }
}

/*****************************************************************************/
double map_get_probability2(PROB_MAP &probs, string num, string denom) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  ///**/fprintf(stdout, "in get_probability1\n");

  itr = probs.find(denom); /* �и� ã�´�. */
  
  if ( itr != probs.end() ) { /* ������ */
    itr2 = itr->second.find(num); /* ���ڸ� ã�´�. */

    if (itr2 != itr->second.end() ) { /* ������ */
      return (itr2->second);
    }
    else { /* ���ڰ� ������ */
      return map_get_probability1(probs, num);
    }
  }
  else { /* �и� ������ */
    return map_get_probability1(probs, num);
  }
}

/*****************************************************************************/
double map_get_probability3(PROB_MAP &probs, string num, string t1, string t2) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  ///**/fprintf(stdout, "in get_probability2\n");

  itr = probs.find(t1+t2); /* �и� ã�´�. */

  if ( itr != probs.end() ) { /* ������ */
    itr2 = itr->second.find(num); /* ���ڸ� ã�´�. */

    if (itr2 != itr->second.end() ) { /* ������ */
      return (itr2->second);
    }
    else { /* ���ڰ� ������ */
      return map_get_probability2(probs, num, t2);
    }
  }
  else { /* �и� ������ */
    return map_get_probability2(probs, num, t2);
  }
}

/*****************************************************************************/
double map_get_probability4(PROB_MAP &probs, string num, string t1, string t2, string t3) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  ///**/fprintf(stdout, "in get_probability3\n");

  itr = probs.find(t1+t2+t3); /* �и� ã�´�. */

  if ( itr != probs.end() ) { /* ������ */
    itr2 = itr->second.find(num); /* ���ڸ� ã�´�. */

    if (itr2 != itr->second.end() ) { /* ������ */
      return (itr2->second);
    }
    else { /* ���ڰ� ������ */
      return map_get_probability3(probs, num, t2, t3);
    }
  }
  else { /* �и� ������ */
    return map_get_probability3(probs, num, t2, t3);
  }
}

/*****************************************************************************/
/* num : ���� */
/* t1+t2+t3+t4 : �и� */
double map_get_probability5(PROB_MAP &probs, string num, string t1, string t2, string t3, string t4) {

  PROB_MAP::iterator itr;
  NUM_PROB::iterator itr2;

  itr = probs.find(t1+t2+t3+t4); /* �и� ã�´�. */

  ///**/fprintf(stdout, "in get_probability4\n");

  if ( itr != probs.end() ) { /* ������ */
    itr2 = itr->second.find(num); /* ���ڸ� ã�´�. */

    if (itr2 != itr->second.end() ) { /* ������ */
      return (itr2->second);
    }
    else { /* ���ڰ� ������ */
      return map_get_probability4(probs, num, t2, t3, t4);
    }
  }
  else { /* �и� ������ */
    return map_get_probability4(probs, num, t2, t3, t4);
  }
}


