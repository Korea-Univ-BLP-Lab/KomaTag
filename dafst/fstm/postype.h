/* definition for value of part-of-speech information. */

#ifndef POSTYPE_H
#define POSTYPE_H

/* lexicon information: part-of-speech. */
enum ePOS 
{
  P_NNCG, /* nominal: noun: common: general */
  P_NNCV, /* nominal: noun: common: verb */
  P_NNCJ, /* nominal: noun: common: adjective */
  P_NNB,  /* nominal: noun: bound */
  P_NNBU, /* nominal: noun: bound: unit */
  P_NNP,  /* nominal: noun: proper */
  P_NPP,  /* nominal: pronoun: personal */
  P_NPI,  /* nominal: pronoun: indicative */
  P_NU, /* nominal: numeral */
  P_XSNN, /* affix: suffix: nominal: noun */
  P_XSNND,  /* affix: suffix: nominal: noun: adnominal -jeok- */
  P_XSNP, /* affix: suffix: nominal: pronoun */
  P_XSNU, /* affix: suffix: nominal: numeral */
  P_XSNPL,  /* affix: suffix: nominal: plural -deul- */
  P_XPNN, /* affix: prefix: nominal: noun */
  P_XPNU, /* affix: prefix: nominal: numeral */
  P_PS, /* particle: subjective */
  P_PC, /* particle: complementive */
  P_PO, /* particle: objective */
  P_PD, /* particle: determinative */
  P_PA, /* particle: adverbal */
  P_PV, /* particle: vocative */
  P_PN, /* particle: conjunctive */
  P_PX, /* particle: auxiliary */
  P_DA, /* adnoun: attributive */
  P_DI, /* adnoun: indicative */
  P_DU, /* adnoun: numberal */
  P_XSD,  /* affix: suffix: adnominal */
  P_AA, /* adverb: attributive */
  P_AP, /* adverb: predicative */
  P_AI, /* adverb: indicative */
  P_AC, /* adverb: conjunctive */
  P_AV, /* adverb: verb */
  P_AJ, /* adverb: adjective */
  P_XSA,  /* affix: suffix: adverbal */
  P_XSAH, /* affix: suffix: adverbal: -hi- */
  P_C,  /* independental(exclamation) */
  P_I,  /* -i- predicative particle stem */
  P_VVG,  /* verbal: verb: general */
  P_VVX,  /* verbal: verb: auxiliary */
  P_VJG,  /* verbal: adjective: general */
  P_VJX,  /* verbal: adjective: auxiliary */
  P_XSVV, /* affix: suffix: verbal: verb */
  P_XSVJ, /* affix: suffix: verbal: adjective */
  P_XSVJD,  /* affix: suffix: verbal: adjective: -dab- */
  P_XSVJB,  /* affix: suffix: verbal: adjective: -srub- -rob- */
  P_EFF,  /* ending: final: finished */
  P_EFC,  /* ending: final: conjunctive */
  P_EFN,  /* ending: final: nominal */
  P_EFD,  /* ending: final: determinative */
  P_EFA,  /* ending: final: adverbial */
  P_EP, /* ending: prefinal */
  P_UN, /* unknown: noun */
  P_UV, /* unknown: verb */
  P_SEVV, /* surface: ending+auxiliary verb */
  P_SEVJ, /* surface: ending+auxiliary adjective */
  P_SHO,  /* surface: 'ha' omit */
  P_SDB,  /* surface: adnoun+boundnoun */
  P_SDN,  /* surface: (2syllables)adnominal+noun */
  P_SBXV, /* surface: boundnoun+aux.verb */
  P_SBXJ, /* surface: boundnoun+aux.adjective */
  P_SCVV, /* surface: noun+verb */
  P_SCVJ, /* surface: noun+adjective */
};

/* lexicon information: feature. */
enum eFEATURE 
{
  F_N_IR,     /* nominal: irregular */
  F_N_CM,     /* nominal: compounding */
  F_VV_R,     /* verb conjugation: regular */
  F_VV_I,     /* verb conjugation: non-L-irregular */
  F_VV_L,     /* verb conjugation: L-irregular */
  F_VJ_R,     /* adjective conjugation: regular */
  F_VJ_I,     /* adjective conjugation: non-L-irregular */
  F_VJ_L,     /* adjective conjugation: L-irregular */
  F_V_NC,     /* verbal: non contract */
  F_P_R_I,      /* particle connectable: -i- predicative particle */
  F_P_V,      /* particle: vowel */
  F_P_C,      /* particle: consonant */
  F_P_L,      /* particle: l-consonant */
  F_P_IR,     /* particle: irregular */
  F_E_L_V,      /* ending connectable: verbal */
  F_E_L_J,      /* ending connectable: adjective */
  F_E_L_I,      /* ending connectable: -i- predicative particle */
  F_E_L_IX,     /* ending connectable: -i- omit */
  F_E_L_P,      /* ending connectable: prefinal ending */
  F_E_R_P,      /* ending connectable: particle */
  F_E_V,      /* ending: vowel */
  F_E_C,      /* ending: consonant */
  F_E_S,      /* ending: ss|bs-consonant */
  F_E_L,      /* ending: l-consonant */
  F_E_LX,     /* ending: l omit */
  F_E_HX,     /* ending: h omit */
  F_E_HC,     /* ending: -ha- contract */
  F_E_IR,     /* ending: irregular */
  F_E_RE,     /* ending: regular */
  F_UNINF,      /* uninflection */
  F_INF,      /* inflection */
  F_DEF,      /* default */
};

/* tag: part-of-speech tag. */
enum eTAG 
{
  T_S,  /*:기호 */
  T_F,  /*:외국어 */
  T_NC, /*:자립명사 */
  T_NB, /*:의존명사 */
  T_NP, /*:대명사 */
  T_NN, /*:수사 */
  T_PV, /*:동사 */
  T_PA, /*:형용사 */
  T_PX, /*:보조용언 */
  T_CO, /*:지정사 */
  T_MAG,  /*:일반부사 */
  T_MAJ,  /*:접속부사 */
  T_MM, /*:관형사 */
  T_II, /*:감탄사 */
  T_XP, /*:접두사 */
  T_XSN,  /*:명사 파생 접미사 */
  T_XSV,  /*:동사 파생 접미사 */
  T_XSM,  /*:형용사 파생 접미사 */
  T_JC, /*:격조사 */
  T_JX, /*:보조사 */
  T_JJ, /*:접속조사 */
  T_JM, /*:속격조사 */
  T_EP, /*:선어말어미 */
  T_EF, /*:종결어미 */
  T_EC, /*:연결어미 */
  T_ETN,  /*:명사형어미 */
  T_ETM,  /*:관형형어미 */
  T_UN, /* unknown */
  T_SPACE,  /* space */
};

#define LEN_POS   ((short)3)
#define NUM_POS   ((short)96)
#define NUM_TAG   (T_SPACE+1)

#endif
