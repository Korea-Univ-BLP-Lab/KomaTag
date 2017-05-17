#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ANCommon.h"
#include "FST.h"

#ifdef WIN32
#pragma warning(disable:4018) // signed/unsigned mismatch
#endif

#ifndef _DEBUG
#define _DEBUG
#endif

/*************/
/* constants */
/*************/

// size of input character set
#define MAX_LETTER 256

// index of information edge 
// which contains information associated with each state
#define INFO_EDGE 0

// index which represents null
#define NULL_INDEX (0xffffff)

// number of bytes needed for saving transducer header
#define HEADER_LENGTH (sizeof(int)*5)

/* ------------------------------------- */

/****************************/
/* internal data structures */
/****************************/

// Terminology
// equivalent state :
// same-hash state

// data structure for edge
typedef struct {
  unsigned int Letter:8; // input character
  unsigned int Value:24; // transition output
  unsigned int Dummy:8; // reserved
  unsigned int Next:24; // transition target
} CEdge;

// data structure for infomation of a state
typedef struct {
  unsigned int Letter:8; // input character
  unsigned int Brother:24; // index for next same-hash state
  unsigned int Final:8; // number of string that is finalized on this state
  unsigned int InBound:24; // number of inbound edge onto this state
} CInfoEdge;

// data structure for state
// a state consists of MAX_LETTER edges
typedef CEdge *CState;

// temporal data structure for state
typedef struct {
  int Index; // index of St
  int STS; // size of subtree whose root node is St
  CState St; // a state
} CInternalState;

// data structure of hash information of a state
typedef struct {
  int Hash1; // first order hash value
  int Hash2; // second order hash value
  int Pos; // position in the second order hash table
  int LastIndex; // index of Last state
  int CurrentIndex; // index of Current state
  CState Last; // the last state whose BT link points to the equivalent state
  CState Current; // the equivalent state
} CHashInfo;

// data structure for a second order hash table
typedef struct {
  int Hash2; // second order hash value
  int Index; // index of the head of same-hash states
} CSecondHashTable;

// data structure for a first order hash table
typedef struct {
  int Size; // size of the second order hash table
  CSecondHashTable *Table; // second order hash table
} CFirstHashTable;

// data structure for a transducer
typedef struct {
  // transducer header
  int StartState; // index of the start state
  int MaxFilled; // size of edge pool except for a meaningless edges
  int EmptyHead; // index of the head of empty edge list

  int EmptyStart;
  int EmptyAge;

  int MaxEntry; // number of entry
  int MaxState; // number of state
  // fields built on run time
  int MaxPool; // size of edge pool
  bool bReadOnly; // flag which represents 
                  // whether the transducer is loaded for read-only mode
  // transducer data
  CEdge *EdgePool; // edge pool
  CFirstHashTable HashTable[MAX_LETTER]; // hash table
} CTransducer;

// data type for the result of hash lookup
typedef enum {
  NO_FIRST_HASH, // there was no state whose first order hash is the same
  NO_SECOND_HASH, // there was no state whose first order hash is the same
  UNREGISTERED_VALID_HASH, // there is some same-hash states, 
                           // but no equivalent state
  REGISTERED_VALID_HASH // there is an equivalent state
} HashResultType;

/* ------------------------------------- */

/*******************/
/* internal macros */
/*******************/

// index of the next same-hash state 
#define BT(State) (((CInfoEdge *)(State))[INFO_EDGE].Brother)
// inbound of the state
#define IB(State) (((CInfoEdge *)(State))[INFO_EDGE].InBound)
// number of strings which are finalized at this state
#define FI(State) (((CInfoEdge *)(State))[INFO_EDGE].Final)

// the input character of ch-th edge
#define LT(State, ch) ((State)[ch].Letter)
// the transition output of ch-th edge
#define VL(State, ch) ((State)[ch].Value)
// the dummy field of ch-th edge
#define DM(State, ch) ((State)[ch].Dummy)
// the transition target of ch-th edge
#define NT(State, ch) ((State)[ch].Next)

// check if i-th edge is information edge
#define IsInfoEdge(i) ((i)==INFO_EDGE)

// check if i-th edge of the state is valid
#define IsValidEdge(State, i) (State&&!IsInfoEdge(i)&&(LT(State, i)==i)&&(NT(State, i)!=NULL_INDEX))

// check if i-th edge of the state contains a meaningful value
#define IsInfoOrValidEdge(State, i) (IsInfoEdge(i)||(State&&(LT(State, i)==i)&&(NT(State, i)!=NULL_INDEX)))

// convert index into state
#define ID2ST(Transducer, Index) (((Index)==NULL_INDEX)?NULL:((CState)((Transducer).EdgePool+(Index))))

// convert state into index
#define ST2ID(Transducer, State) ((State)?((State)-(Transducer).EdgePool):NULL_INDEX)

/**********************/
/* internal functions */
/**********************/

// print a hash table whose first order hash value is Hash1
void PrintHashTable(CTransducer *pTransducer, int Hash1);

// print information of a state
void PrintState(CState State);

// compare two states
int StateCmp(const CState x, const CState y)
{

  if (FI(x)!=FI(y)) return FI(x)-FI(y);

  for (int i=0; i<MAX_LETTER; i++) {
    if (IsInfoEdge(i)) continue;
    int Delta=IsValidEdge(x, i)-IsValidEdge(y, i);
    if (Delta) return Delta;
    if (IsValidEdge(x, i)) {
      Delta=NT(x, i)-NT(y, i);
      if (Delta) return Delta;
    }
  }

  return 0;
}

// calculate first order hash
int FirstHashFunction(const CState State)
{

  for (int i=0; i<MAX_LETTER; i++) {
    if (IsInfoEdge(i)) continue;
    if (IsValidEdge(State, i))
      return (i+NT(State, i)+FI(State))%MAX_LETTER;
  }
  return FI(State);
}

// calculate second order hash
int SecondHashFunction(const CState State)
{

  for (int i=MAX_LETTER-1; i>=0; i--) {
    if (IsInfoEdge(i)) continue;
    if (IsValidEdge(State, i))
      return (i+NT(State, i)+FI(State))%MAX_LETTER;
  }
  return FI(State);
}

// allocate memory for a new state and initialize
CState NewState(void)
{
  CState State;

  if ((State=(CState)ANMalloc(sizeof(CEdge)*MAX_LETTER))==NULL) return NULL;
  memset(State, 0xff, sizeof(CEdge)*MAX_LETTER);
  
  LT(State, INFO_EDGE)=INFO_EDGE;
  BT(State)=NULL_INDEX;
  IB(State)=1;
  FI(State)=0;

  return State;
}

// free memory for a state
void FreeState(CState State)
{
  ANFree(State);
}

// clone this state except for Brother(NULL_INDEX) and InBound(1)
// bModifyIB : flag which represents whether to modify inbound or not
CState CloneState(CTransducer *pTransducer, CState Origin, bool bModifyIB)
{
  CState State;

  if ((State=(CState)ANMalloc(sizeof(CEdge)*MAX_LETTER))==NULL) return NULL;

#ifdef _DEBUG
  if (State) {
#endif
    memcpy(State, Origin, sizeof(CEdge)*MAX_LETTER);
    if (bModifyIB) {
      IB(State)=1;
      IB(Origin)--;
      for (int i=0; i<MAX_LETTER; i++)
  if (IsValidEdge(Origin, i))
    IB(ID2ST(*pTransducer, NT(Origin, i)))++;
    }
      
    BT(State)=NULL_INDEX;
#ifdef _DEBUG
  }
  else {
    ErrorCallback("cannot clone state due to not enough memory");
  }
#endif
  return State;
}

// check if the edge is empty(not a valid edge)
bool IsEmptyEdge(CTransducer *pTransducer, CEdge *pEdge)
{
  CState State=pEdge-pEdge->Letter;

  if (State<pTransducer->EdgePool)
    return true;
  else return 
   (!IsInfoEdge(pEdge->Letter)&&(pEdge->Next==NULL_INDEX))||
   (LT(State, INFO_EDGE)!=INFO_EDGE);
}

// replace the transition state for Letter with Next
// change OutBound and SubTreeSize of this pState
// change InBound of old state
// do not change InBound of Next
bool ReplaceChild(CTransducer *pTransducer, CInternalState *pState,
      unsigned char Letter, CInternalState *pNextState)
{
#ifdef _DEBUG
  // check if the edge already exists
  if (!IsValidEdge(pState->St, Letter)) {
    ErrorCallback("cannot find edge for replace");
    return false;
  }
#endif

  // get old edge
  CState OldState=ID2ST(*pTransducer, NT(pState->St, Letter));

  int i, Delta;

  // seek the next edge
  for (i=Letter+1; i<MAX_LETTER; i++) {
    if (IsInfoEdge(i)) continue;
    if (IsValidEdge(pState->St, i)) break;
  }

  // calculate Delta 
  // Delta is the difference of STS between new edge and old edge

  // there is no following edge
  if (i==MAX_LETTER) 
    Delta=pNextState->STS-(pState->STS-VL(pState->St, Letter));
  // there is the following edge(s)
  else Delta=pNextState->STS-(VL(pState->St, i)-VL(pState->St, Letter));

  if (Delta) {
    // adjust VL of following edges
    for (i=Letter+1; i<MAX_LETTER; i++) {
      if (IsInfoEdge(i)) continue;
      if (IsValidEdge(pState->St, i))
        VL(pState->St, i)+=Delta;
    }
    // adjust STS of this state
    pState->STS+=Delta;
  }

  // make a link
  NT(pState->St, Letter)=pNextState->Index;
  return true;
}

// add a transition to pState
// change OutBound and SubTreeSize of this pState
// do not change InBound of Next because it has already a valid value
bool AddChild(CTransducer *pTransducer, CInternalState *pState, 
        unsigned char Letter, CInternalState *pNextState)
{  
#ifdef _DEBUG
  // check if the edge is empty
  if (IsInfoOrValidEdge(pState->St, Letter)) {
    ErrorCallback("cannot add edge because it already exists");
    return false;
  }
#endif

  int i;

  // seek the next edge
  for (i=Letter+1; i<MAX_LETTER; i++) {
    if (IsInfoEdge(i)) continue;
    if (IsValidEdge(pState->St, i)) break;
  }

  // calculate VL of new edge

  // there is no following edge
  if (i==MAX_LETTER) 
    VL(pState->St, Letter)=pState->STS;
  // there is any following edge(s)
  else {
    VL(pState->St, Letter)=VL(pState->St, i);
    // adjust VL of following edge(s)
    for (i=Letter+1; i<MAX_LETTER; i++) {
      if (IsInfoEdge(i)) continue;
      if (IsValidEdge(pState->St, i)) 
        VL(pState->St, i)+=pNextState->STS;
    }
  }

  // make a link
  LT(pState->St, Letter)=Letter;
  NT(pState->St, Letter)=pNextState->Index;
  pState->STS+=pNextState->STS;

  return true;
}

// calculate hash functions and lookup hash table
HashResultType LookupHash(CTransducer *pTransducer, CState State, 
        CHashInfo *pInfo)
{

  // initialize result
  pInfo->CurrentIndex=NULL_INDEX;
  pInfo->Current=NULL;
  pInfo->LastIndex=NULL_INDEX;
  pInfo->Last=NULL;

  // calculate hash values
  pInfo->Hash1=FirstHashFunction(State);
  pInfo->Hash2=SecondHashFunction(State);

  // there's no state corresponding Hash1
  if (pTransducer->HashTable[pInfo->Hash1].Size==0)
    return NO_FIRST_HASH;

  // binary search in the second order hash table
  int Left=0, Right=pTransducer->HashTable[pInfo->Hash1].Size-1, Mid, Cmp=-1;
  while (Left<=Right) {
    Mid=(Left+Right)/2;
    Cmp=pInfo->Hash2-pTransducer->HashTable[pInfo->Hash1].Table[Mid].Hash2;
    if (Cmp==0) {
      Left=Mid;
      break;
    }
    else if (Cmp<0) Right=Mid-1;
    else Left=Mid+1;
  }

  pInfo->Pos=Left;

  // if there is at least one same-hash state
  if (Cmp==0) {
    pInfo->CurrentIndex=pTransducer->HashTable[pInfo->Hash1].Table[Left].Index;
    pInfo->Current=ID2ST(*pTransducer, pInfo->CurrentIndex);
    pInfo->LastIndex=NULL_INDEX;
    pInfo->Last=NULL;

    // sequential search in the same-hash state list
    while (pInfo->Current&&((Cmp=StateCmp(pInfo->Current, State))>0)) {
      pInfo->LastIndex=pInfo->CurrentIndex;
      pInfo->Last=pInfo->Current;
      pInfo->CurrentIndex=BT(pInfo->Current);
      pInfo->Current=ID2ST(*pTransducer, pInfo->CurrentIndex);
    }

    if (pInfo->Current&&(Cmp==0)) 
      return REGISTERED_VALID_HASH;
    else return UNREGISTERED_VALID_HASH;
  }
  else return NO_SECOND_HASH;
}

// delete a state from hash table
bool DeregisterState(CTransducer *pTransducer, CState State)
{
  CHashInfo Info;

#ifdef _DEBUG
  // check if the state is already registered
  if ((LookupHash(pTransducer, State, &Info)!=REGISTERED_VALID_HASH)||
      (Info.Current!=State)) {
    ErrorCallback("cannot deregister unregistered state %d %d %d", 
    ST2ID(*pTransducer, Info.Current), 
    ST2ID(*pTransducer, State),
    pTransducer->MaxFilled-MAX_LETTER);
    fprintf(stderr, "%d %d %d\n", LT(State, 0), VL(State, 0), NT(State, 0));
    PrintHashTable(pTransducer, Info.Hash1);
    return false;
  }
#else
  LookupHash(pTransducer, State, &Info);
#endif

  // if the state is registered as the first item of hash list
  if (Info.Last==NULL) {
    // if there is only one state on this hash list, remove the list
    if (BT(State)==NULL_INDEX) {
      if (Info.Pos<pTransducer->HashTable[Info.Hash1].Size-1)
        memmove(pTransducer->HashTable[Info.Hash1].Table+Info.Pos,
                pTransducer->HashTable[Info.Hash1].Table+Info.Pos+1,
                sizeof(CSecondHashTable)*
                (pTransducer->HashTable[Info.Hash1].Size-Info.Pos-1));
      (pTransducer->HashTable[Info.Hash1].Size)--;
      if (pTransducer->HashTable[Info.Hash1].Size==0) {
        ANFree(pTransducer->HashTable[Info.Hash1].Table);
        pTransducer->HashTable[Info.Hash1].Table=NULL;
      }
    }
    // modify head of hash list
    else pTransducer->HashTable[Info.Hash1].Table[Info.Pos].Index=BT(State);
  }
  // remove this state from hash list
  else BT(Info.Last)=BT(State);

  BT(State)=NULL_INDEX;
  return true;
}

// delete a state from transducer
void RemoveState(CTransducer *pTransducer, CState State)
{
  int i, LastPos;

  if (pTransducer->EmptyHead>=ST2ID(*pTransducer, State))
    LastPos=-1;
  else for (LastPos=ST2ID(*pTransducer, State)-1; LastPos>=0; LastPos--)
    if (IsEmptyEdge(pTransducer, pTransducer->EdgePool+LastPos)) 
      break;

  // remove the state from flatten array
  for (i=0; i<MAX_LETTER; i++) 
    if (IsInfoOrValidEdge(State, i)||IsEmptyEdge(pTransducer, State+i)) {
      memset(State+i, 0xff, sizeof(CEdge));
      if (LastPos==-1) 
        pTransducer->EmptyHead=ST2ID(*pTransducer, State)+i;
      else pTransducer->EdgePool[LastPos].Value=ST2ID(*pTransducer, State)+i;
      LastPos=ST2ID(*pTransducer, State)+i;
    }

  for (i=LastPos+1; i<pTransducer->MaxPool; i++)
    if (IsEmptyEdge(pTransducer, pTransducer->EdgePool+i)) break;

  if (i<pTransducer->MaxPool) 
    pTransducer->EdgePool[LastPos].Value=i;
  else pTransducer->EdgePool[LastPos].Value=NULL_INDEX;

  (pTransducer->MaxState)--;

  // Modified for decreasing flattening time
  if (pTransducer->EmptyStart>0)
    if (rand()%pTransducer->EmptyStart>ST2ID(*pTransducer, State)) {
      pTransducer->EmptyAge=0;
      if (rand()%pTransducer->MaxFilled>pTransducer->EmptyStart)
        pTransducer->EmptyStart=pTransducer->EmptyHead;
      else pTransducer->EmptyStart=ST2ID(*pTransducer, State);
    }
}

// deregister and delete a state and make the same temporal state
CState MakeAsTemporalState(CTransducer *pTransducer, CState State)
{
  
  // remove from hash table
  if (!DeregisterState(pTransducer, State)) return NULL;

  // make new state and clone
  CState NewSt=CloneState(pTransducer, State, false);
  if (NewSt==NULL) return NULL;
  //  IB(NewSt)=IB(State);
  BT(NewSt)=BT(State);

  RemoveState(pTransducer, State);

  return NewSt;
}

// add a state into transducer 
int Flattening(CTransducer *pTransducer, CState State)
{
  int i, Pos, LastPos;
  int nEdge=0;
  CEdge List[MAX_LETTER];

  for (i=0; i<MAX_LETTER; i++) 
    if (IsInfoOrValidEdge(State, i)) {
      memcpy(List+nEdge, State+i, sizeof(CEdge));
      nEdge++;
    }

  // realloc memory if it needs
  if (pTransducer->MaxPool<pTransducer->MaxFilled+MAX_LETTER) {
    CEdge *OldPool=pTransducer->EdgePool;
    if ((pTransducer->EdgePool=(CEdge *)ANRealloc(pTransducer->EdgePool,
                                                  sizeof(CEdge)*(pTransducer->MaxPool+MAX_LETTER)))==NULL) {
      // exception handling
      pTransducer->EdgePool=OldPool;
      FreeState(State);
      return NULL_INDEX;
    }
    else {
      memset(pTransducer->EdgePool+pTransducer->MaxPool, 0xff, 
       sizeof(CEdge)*MAX_LETTER);

      // for fast flattening
      for (i=0; i<MAX_LETTER-1; i++)
        VL(pTransducer->EdgePool+pTransducer->MaxPool, i)=pTransducer->MaxPool+i+1;
      VL(pTransducer->EdgePool+pTransducer->MaxPool, MAX_LETTER-1)=NULL_INDEX;

      if (pTransducer->EmptyHead>=pTransducer->MaxPool)
        i=-1;
      else for (i=pTransducer->MaxPool-1; i>=0; i--)
        if (IsEmptyEdge(pTransducer, pTransducer->EdgePool+i))
          break;

      if (i<0) {
        pTransducer->EmptyHead=pTransducer->MaxPool;
        pTransducer->EmptyStart=pTransducer->EmptyHead;
      }
      else pTransducer->EdgePool[i].Value=pTransducer->MaxPool;

      pTransducer->MaxPool+=MAX_LETTER;
    }
  }

  // seek proper position
  if (pTransducer->EmptyHead==pTransducer->EmptyStart)
    LastPos=-1;
  else for (LastPos=pTransducer->EmptyStart-1; LastPos>=0; LastPos--)
    if (IsEmptyEdge(pTransducer, pTransducer->EdgePool+LastPos)) 
      break;

  for (Pos=pTransducer->EmptyStart; Pos<=pTransducer->MaxPool-MAX_LETTER;) {
    for (i=0; i<nEdge; i++) 
      if (!IsEmptyEdge(pTransducer, pTransducer->EdgePool+Pos+List[i].Letter)) 
        break;

    // if proper position detected, copy and break
    if (i==nEdge) {
      // remove the state from flatten array
      for (i=0; i<MAX_LETTER; i++) {
        if (IsInfoOrValidEdge(State, i)) {
          if (LastPos==-1) {
            pTransducer->EmptyHead=pTransducer->EdgePool[Pos+i].Value;
            pTransducer->EmptyStart=pTransducer->EmptyHead;
          }
          else {
            if (pTransducer->EmptyStart==Pos+i)
              pTransducer->EmptyStart=pTransducer->EdgePool[Pos+i].Value;
            pTransducer->EdgePool[LastPos].Value=
              pTransducer->EdgePool[Pos+i].Value;
          }
        memcpy(pTransducer->EdgePool+Pos+i, State+i, sizeof(CEdge));
      }
      else if (IsEmptyEdge(pTransducer, pTransducer->EdgePool+Pos+i)) 
        LastPos=Pos+i;
      }
      break;
    }
    else {
      LastPos=Pos;
      Pos=pTransducer->EdgePool[Pos].Value;
    }
  }

  // modified for decreasing flattening time
  if (Pos>pTransducer->EmptyStart) {
    (pTransducer->EmptyAge)++;
    if (pTransducer->EmptyAge>2) {
      while (rand()%pTransducer->MaxFilled<(pTransducer->MaxFilled-pTransducer->EmptyStart))
        if (pTransducer->EdgePool[pTransducer->EmptyStart].Value==NULL_INDEX)
          break;
        else pTransducer->EmptyStart=pTransducer->EdgePool[pTransducer->EmptyStart].Value;
      pTransducer->EmptyAge=0;
    }
  } 
  else pTransducer->EmptyAge=0;

  // modify MaxFilled field
  if (pTransducer->MaxFilled<Pos+MAX_LETTER)
    pTransducer->MaxFilled=Pos+MAX_LETTER;

  FreeState(State);

  return Pos;
}

// register a state onto hash table and add it into transducer
int RegisterOrOptimize(CTransducer *pTransducer, CState State, 
           bool bFlatten)
{
  CHashInfo Info;
  int RegIndex;
  CSecondHashTable *OldTable;
  HashResultType x;

  x=LookupHash(pTransducer, State, &Info);

  if (x!=REGISTERED_VALID_HASH) {
    if (bFlatten) RegIndex=ST2ID(*pTransducer, State);
    else {
      if ((RegIndex=Flattening(pTransducer, State))==NULL_INDEX) 
        return NULL_INDEX;
      (pTransducer->MaxState)++;
    }
  }

  Info.Current=ID2ST(*pTransducer, Info.CurrentIndex);
  Info.Last=ID2ST(*pTransducer, Info.LastIndex);

  switch (x) {
  case NO_FIRST_HASH:
    pTransducer->HashTable[Info.Hash1].Table=
      (CSecondHashTable *)ANMalloc(sizeof(CSecondHashTable));
    if (!pTransducer->HashTable[Info.Hash1].Table) return NULL_INDEX;
    pTransducer->HashTable[Info.Hash1].Table[0].Hash2=Info.Hash2;
    pTransducer->HashTable[Info.Hash1].Table[0].Index=RegIndex;
    pTransducer->HashTable[Info.Hash1].Size=1;
    break;
  case NO_SECOND_HASH:
    OldTable=pTransducer->HashTable[Info.Hash1].Table;
    if ((pTransducer->HashTable[Info.Hash1].Table=(CSecondHashTable *)
        ANRealloc(pTransducer->HashTable[Info.Hash1].Table, 
                  sizeof(CSecondHashTable)*
                  (pTransducer->HashTable[Info.Hash1].Size+1)))==NULL){
      pTransducer->HashTable[Info.Hash1].Table=OldTable;
      return NULL_INDEX;
    }
    if (Info.Pos<pTransducer->HashTable[Info.Hash1].Size)
      memmove(pTransducer->HashTable[Info.Hash1].Table+Info.Pos+1,
              pTransducer->HashTable[Info.Hash1].Table+Info.Pos,
              sizeof(CSecondHashTable)*
              (pTransducer->HashTable[Info.Hash1].Size-Info.Pos));
    pTransducer->HashTable[Info.Hash1].Table[Info.Pos].Index=RegIndex;
    pTransducer->HashTable[Info.Hash1].Table[Info.Pos].Hash2=Info.Hash2;
    (pTransducer->HashTable[Info.Hash1].Size)++;
    break;
  case UNREGISTERED_VALID_HASH:
    if (Info.Last==NULL) {
      BT(ID2ST(*pTransducer, RegIndex))=
        pTransducer->HashTable[Info.Hash1].Table[Info.Pos].Index;
      pTransducer->HashTable[Info.Hash1].Table[Info.Pos].Index=RegIndex;
    }
    else {    
      BT(ID2ST(*pTransducer, RegIndex))=BT(Info.Last);
      BT(Info.Last)=RegIndex;
    }
    break;
  case REGISTERED_VALID_HASH:
#ifdef _DEBUG
    if (Info.Current==State) {
      ErrorCallback("cannot optimize registered state %d\n", ST2ID(*pTransducer, State));
      return NULL_INDEX;
    }
    else {
#endif
      IB(Info.Current)+=IB(State);
      if (bFlatten) 
        RemoveState(pTransducer, State);
      else FreeState(State);
      for (int i=0; i<MAX_LETTER; i++)
        if (IsValidEdge(Info.Current, i)) 
          IB(ID2ST(*pTransducer, NT(Info.Current, i)))--;
#ifdef _DEBUG
    }
#endif
    return ST2ID(*pTransducer, Info.Current);
  }
  return RegIndex;
}

// decide whether to make a new state, to clone a state, or to use itself
// and do it
bool AdjustHead(CTransducer *pTransducer, CInternalState *pOldHead,
    CInternalState *pNewHead, const unsigned char *s, bool bClone)
{
  int i;

  // if there is no head, create new head
  if (pOldHead->St==NULL) {
    pNewHead->St=NewState();
    if (pNewHead->St==NULL) return false;
    pNewHead->Index=NULL_INDEX; // unflatten state
    pNewHead->STS=0;
  }
  // if head should be cloned, do it
  else if (bClone) {
    // exception handling
    if ((pNewHead->St=CloneState(pTransducer, pOldHead->St, true))==NULL) 
      return false;
    pNewHead->Index=NULL_INDEX; // unflatten state
    pNewHead->STS=pOldHead->STS;
  }
  // otherwise use existing state
  else {
    // if no room is needed
    if ((s[0]==0)||(LT(pOldHead->St, s[0])==s[0])) {
      if (!DeregisterState(pTransducer, pOldHead->St)) return false;
      memcpy(pNewHead, pOldHead, sizeof(CInternalState));
    }
    //if there is a room for child on flatten array
    else if (IsEmptyEdge(pTransducer, pOldHead->St+s[0])) {
      if (!DeregisterState(pTransducer, pOldHead->St)) return false;

      if (pTransducer->EmptyHead>=ST2ID(*pTransducer, pOldHead->St)+s[0])
        i=-1;
      else for (i=ST2ID(*pTransducer, pOldHead->St)+s[0]-1; i>=0; i--)
        if (IsEmptyEdge(pTransducer, pTransducer->EdgePool+i)) 
          break;

      if (ST2ID(*pTransducer, pOldHead->St)+s[0]==pTransducer->EmptyStart)
        pTransducer->EmptyStart=VL(pOldHead->St, s[0]);

      if (i<0) {
#ifdef _DEBUG
        if (pTransducer->EmptyHead!=ST2ID(*pTransducer, pOldHead->St)+s[0]) {
          ErrorCallback("invalid empty edge %d %d", pTransducer->EmptyHead, VL(pOldHead->St, s[0]));
          ErrorCallback("%d %d %d", pTransducer->EdgePool[pTransducer->EmptyHead].Letter,
                                    pTransducer->EdgePool[pTransducer->EmptyHead].Value,
                                    pTransducer->EdgePool[pTransducer->EmptyHead].Next);
        }
#endif
        pTransducer->EmptyHead=VL(pOldHead->St, s[0]);
      }
      else {
#ifdef _DEBUG
        if (pTransducer->EdgePool[i].Value!=
            ST2ID(*pTransducer, pOldHead->St)+s[0]) {
          ErrorCallback("invalid next empty edge %d %d %d",
                        i, pTransducer->EdgePool[i].Value, 
                        VL(pOldHead->St, s[0]));
          ErrorCallback("%d %d %d", 
                        pTransducer->EdgePool[i].Letter,
                        pTransducer->EdgePool[i].Value,
                        pTransducer->EdgePool[i].Next);
          ErrorCallback("%d %d %d", 
                        pTransducer->EdgePool[pTransducer->EdgePool[i].Value].Letter,
                        pTransducer->EdgePool[pTransducer->EdgePool[i].Value].Value,
                        pTransducer->EdgePool[pTransducer->EdgePool[i].Value].Next);
        }
#endif
        pTransducer->EdgePool[i].Value=VL(pOldHead->St, s[0]);
      }

      memcpy(pNewHead, pOldHead, sizeof(CInternalState));
      LT(pNewHead->St, s[0])=s[0];
      VL(pNewHead->St, s[0])=NULL_INDEX; // represent this is temporal edge
      NT(pNewHead->St, s[0])=0; // any value except NULL_INDEX
    }
    else {
      if ((pNewHead->St=MakeAsTemporalState(pTransducer, pOldHead->St))==NULL) 
        return false;
      pNewHead->Index=NULL_INDEX; // unflatten state
      pNewHead->STS=pOldHead->STS;
    }
  }

  return true;
}

// add a partial key into pHead
CState AddPartialKey(CTransducer *pTransducer, CInternalState *pHead, 
        const unsigned char *s, bool bClone)
{
  CInternalState NewHead, NextSt, NewNextSt;
  int i;

  // set bClone
  if ((pHead->St==NULL)||(IB(pHead->St)>1))
    bClone=true;

  if (!AdjustHead(pTransducer, pHead, &NewHead, s, bClone))
    return NULL;

  if (s[0]==0) {
    FI(NewHead.St)++;
    NewHead.STS++;
    for (i=0; i<MAX_LETTER; i++)
      if (IsValidEdge(NewHead.St, i))
        VL(NewHead.St, i)++;
  }
  else {
    // if it is temporal state
    if (((NewHead.Index==NULL_INDEX)||(VL(NewHead.St, s[0])!=NULL_INDEX))&&
        IsValidEdge(NewHead.St, s[0]))
      NextSt.Index=NT(NewHead.St, s[0]);
    else NextSt.Index=NULL_INDEX;

    if (NextSt.Index==NULL_INDEX)
      NextSt.STS=0;
    else {
      for (i=s[0]+1; i<MAX_LETTER; i++) {
        if (IsInfoEdge(i)) continue;
        if (IsValidEdge(NewHead.St, i)) break;
      }
      // there is no next edge
      if (i==MAX_LETTER) 
        NextSt.STS=NewHead.STS-VL(NewHead.St, s[0]);
      else NextSt.STS=VL(NewHead.St, i)-VL(NewHead.St, s[0]);
    }
    NextSt.St=ID2ST(*pTransducer, NextSt.Index);

    if ((NewNextSt.St=AddPartialKey(pTransducer, &NextSt, s+1, bClone))==NULL) {
      if (NewHead.Index==NULL_INDEX) FreeState(NewHead.St);
      return NULL;
    }
    NewNextSt.Index=ST2ID(*pTransducer, NewNextSt.St);
    NewNextSt.STS=NextSt.STS+1;

    // adjust state
    if (NewHead.Index!=NULL_INDEX) 
      NewHead.St=ID2ST(*pTransducer, NewHead.Index);
    //    if (NextSt.Index!=NULL_INDEX)
    //  NextSt.St=ID2ST(*pTransducer, NextSt.Index);

    // if NextState is not changed
    if (NewNextSt.Index==NextSt.Index) {
      (NewHead.STS)++;
      for (i=s[0]+1; i<MAX_LETTER; i++) {
        if (IsInfoEdge(i)) continue;
        if (IsValidEdge(NewHead.St, i)) 
          VL(NewHead.St, i)++;
      }
    }
    else {
      if ((NextSt.Index==NULL_INDEX)&&(VL(NewHead.St, s[0])==NULL_INDEX)) 
        memset(NewHead.St+s[0], 0xff, sizeof(CEdge));
      if (!((NextSt.Index!=NULL_INDEX)?
            ReplaceChild(pTransducer, &NewHead, s[0], &NewNextSt):
            AddChild(pTransducer, &NewHead, s[0], &NewNextSt))) {
        if (NewHead.Index==NULL_INDEX) FreeState(NewHead.St);
          return NULL;
      }
    }
  }

  int St=RegisterOrOptimize(pTransducer, NewHead.St, 
          (NewHead.Index!=NULL_INDEX));

  return ID2ST(*pTransducer, St);
}

// number of bytes required for saving hash table
int HashTableSize(void *pTransducer)
{
  CTransducer *x=(CTransducer *)pTransducer;
  int i, Size=0;

  for (i=0; i<MAX_LETTER; i++)
    Size+=x->HashTable[i].Size*sizeof(int);
  Size+=MAX_LETTER*sizeof(int);
  return Size;
}

// number of bytes required for saving a transducer and its hash table
int TransducerSize(void *pTransducer)
{
  CTransducer *x=(CTransducer *)pTransducer;
  return HashTableSize(pTransducer)+sizeof(CEdge)*x->MaxFilled+HEADER_LENGTH;
}

// load a transducer content from a file and return it
void *LoadTransducerContent(const char *Filename)
{
  CTransducer *x;
  FILE *f;
  int FileSize;

  if ((f=fopen(Filename, "rb"))==NULL) return NULL;

  if ((x=(CTransducer *)NewTransducer())==NULL) return NULL;

  ANFseek(f, 0, 2);
  FileSize=ftell(f);
  if (FileSize<HEADER_LENGTH) {
#ifdef _DEBUG
    ErrorCallback("Invalid FST size : %d", FileSize);
#endif
    FreeTransducer(x);
    fclose(f);
    return NULL;
  }

  ANFseek(f, FileSize-HEADER_LENGTH, 0);
  ANFread(&(x->StartState), sizeof(int), f);
  ANFread(&(x->MaxFilled), sizeof(int), f);
  ANFread(&(x->EmptyHead), sizeof(int), f);
  ANFread(&(x->MaxEntry), sizeof(int), f);
  ANFread(&(x->MaxState), sizeof(int), f);

  if (sizeof(CEdge)*x->MaxFilled+HEADER_LENGTH!=FileSize) {
#ifdef _DEBUG
    ErrorCallback("Invalid FST size : %d... %d was expected", 
                  FileSize, sizeof(CEdge)*x->MaxFilled+HEADER_LENGTH);
#endif
    FreeTransducer(x);
    fclose(f);
    return NULL;
  }

  if ((x->EdgePool=(CEdge *)ANMalloc(sizeof(CEdge)*x->MaxFilled))==NULL) {
    FreeTransducer(x);
    return NULL;
  }

  ANFseek(f, 0, 0);
  ANFread(x->EdgePool, sizeof(CEdge)*x->MaxFilled, f);

  fclose(f);

  x->MaxPool=x->MaxFilled;
  x->bReadOnly=true;

  x->EmptyStart=x->EmptyHead;
  x->EmptyAge=0;

  return x;
}

// load the hash table of a transducer from a file and return it
bool LoadHashTable(void *pTransducer, const char *Filename)
{
  CTransducer *x=(CTransducer *)pTransducer;
  FILE *f;
  int i, j;

  if ((f=fopen(Filename, "rb"))==NULL) 
    return x->MaxState==0;

  for (i=0; i<MAX_LETTER; i++) {
    ANFread(&(x->HashTable[i].Size), sizeof(int), f);
    if (x->HashTable[i].Size) {
      if ((x->HashTable[i].Table=(CSecondHashTable *)
          ANMalloc(sizeof(CSecondHashTable)*x->HashTable[i].Size))==NULL)
        return false;
    }
    else x->HashTable[i].Table=NULL;
  }

  for (i=0; i<MAX_LETTER; i++) 
    for (j=0; j<x->HashTable[i].Size; j++) {
      ANFread(&(x->HashTable[i].Table[j].Index), sizeof(int), f);
      x->HashTable[i].Table[j].Hash2=
        SecondHashFunction(ID2ST(*x, x->HashTable[i].Table[j].Index));
    }

  fclose(f);
  x->bReadOnly=false;

  return true;
}

// save a transducer content to a file
bool SaveTransducerContent(void *pTransducer, const char *Filename)
{
  CTransducer *x=(CTransducer *)pTransducer;
  char TempFilename[MAX_PATH], *pDot;
  FILE *f;

  if ((pDot=strchr(Filename, '.'))==NULL)
    sprintf(TempFilename, "%s.fak", Filename);
  else {
    memcpy(TempFilename, Filename, pDot-Filename);
    TempFilename[pDot-Filename]=0;
    strcat(TempFilename, ".fak");
  }

  if ((f=ANFopen(TempFilename, "wb"))==NULL) return false;

  if (ANFwrite(x->EdgePool, sizeof(CEdge)*x->MaxFilled, f)!=sizeof(CEdge)*x->MaxFilled) {
#ifdef _DEBUG
    ErrorCallback("cannot write on line %d file %s %s %d", __LINE__, __FILE__, TempFilename, sizeof(CEdge)*x->MaxFilled);
#endif
    ANFclose(f);
    remove(TempFilename);
    return false;
  }

  if ((ANFwrite(&(x->StartState), sizeof(int), f)!=sizeof(int))||
      (ANFwrite(&(x->MaxFilled), sizeof(int), f)!=sizeof(int))||
      (ANFwrite(&(x->EmptyHead), sizeof(int), f)!=sizeof(int))||
      (ANFwrite(&(x->MaxEntry), sizeof(int), f)!=sizeof(int))||
      (ANFwrite(&(x->MaxState), sizeof(int), f)!=sizeof(int))) {
#ifdef _DEBUG
    ErrorCallback("cannot write on line %d file %s", __LINE__, __FILE__);
#endif
    ANFclose(f);
    remove(TempFilename);
    return false;
  }

  ANFclose(f);
  remove(Filename);
  rename(TempFilename, Filename);
  return true;
}

// save the hash table of a transducer to a file
bool SaveHashTable(void *pTransducer, const char *Filename)
{
  CTransducer *x=(CTransducer *)pTransducer;
  char TempFilename[MAX_PATH], *pDot;
  FILE *f;
  int i, j;

  if ((pDot=strchr(Filename, '.'))==NULL)
    sprintf(TempFilename, "%s.hak", Filename);
  else {
    memcpy(TempFilename, Filename, pDot-Filename);
    TempFilename[pDot-Filename]=0;
    strcat(TempFilename, ".hak");
  }

  if ((f=ANFopen(TempFilename, "wb"))==NULL) return false;

  for (i=0; i<MAX_LETTER; i++) 
    if (ANFwrite(&(x->HashTable[i].Size), sizeof(int), f)!=sizeof(int)) {
#ifdef _DEBUG
    ErrorCallback("cannot write on line %d file %s %s", __LINE__, __FILE__, TempFilename);
#endif
      ANFclose(f);
      remove(TempFilename);
      return false;
    }

  for (i=0; i<MAX_LETTER; i++)
    for (j=0; j<x->HashTable[i].Size; j++)
      if (ANFwrite(&(x->HashTable[i].Table[j].Index), sizeof(int), f)!=sizeof(int)) {
#ifdef _DEBUG
    ErrorCallback("cannot write on line %d file %s", __LINE__, __FILE__);
#endif
        ANFclose(f);
        remove(TempFilename);
        return false;
      }

  ANFclose(f);
  remove(Filename);
  rename(TempFilename, Filename);
  return true;
}

// print a hash table whose first order hash value is Hash1
void PrintHashTable(CTransducer *pTransducer, int Hash1)
{
  CState Current;

  for (int i=0; i<pTransducer->HashTable[Hash1].Size; i++) {
    printf("%d :", pTransducer->HashTable[Hash1].Table[i].Index);
    for (Current=ID2ST(*pTransducer, 
           pTransducer->HashTable[Hash1].Table[i].Index);
        Current; Current=ID2ST(*pTransducer, BT(Current))) 
      printf("%d(%d:%d) ", ST2ID(*pTransducer, Current),
        FirstHashFunction(Current),
        SecondHashFunction(Current));
    printf("\n");
  }
}

// print information of a state
void PrintState(CState State)
{
  for (int i=0; i<MAX_LETTER; i++) {
    if (IsInfoEdge(i)) continue;
    if (IsValidEdge(State, i))
      printf("%x %x %d ", i, VL(State, i), NT(State, i));
  }
  printf("\n");
}

// traverse subtree whose root is State
void DoTraverse(CTransducer *pTransducer, CState State, 
    char *Key, int Pos, int *pHash, void *pParam,
    void (*Callback)(void *pParam, const char *s, int Hash, int nItem))
{

  if (FI(State)>0) {
    Key[Pos]=0;
    Callback(pParam, Key, *pHash, FI(State));
    *pHash+=FI(State);
  }

  for (int i=0; i<MAX_LETTER; i++) {
    if (IsInfoEdge(i)) continue;
    if (IsValidEdge(State, i)) {
      Key[Pos]=i;
      Key[Pos+1]=0;
      DoTraverse(pTransducer, ID2ST(*pTransducer, NT(State, i)),
        Key, Pos+1, pHash, pParam, Callback);
    }
  }
}

// count number of states in subtree whose root is State
int CountTransducer(CTransducer *pTransducer, CState State)
{
  int Count=0;

  if (FI(State)>0) 
    Count+=FI(State);

  for (int i=0; i<MAX_LETTER; i++) {
    if (IsInfoEdge(i)) continue;
    if (IsValidEdge(State, i)) 
      Count+=CountTransducer(pTransducer, ID2ST(*pTransducer, NT(State, i)));
  }
  return Count;
}

// check whether empty edge list is consistent or not
bool CheckEmptyEdge(CTransducer *pTransducer)
{
  int i, Pos=pTransducer->EmptyHead;

  for (i=0; i<pTransducer->MaxFilled; i++) {
    if (IsEmptyEdge(pTransducer, pTransducer->EdgePool+i)) {
      if (i!=Pos) {
#ifdef _DEBUG
        ErrorCallback("Empty edges are mismatched : %d while %d is expected", i, Pos);
        ErrorCallback("next empty edge of %d is %d", i, pTransducer->EdgePool[i].Value);
#endif
        return false;
      }
      else Pos=pTransducer->EdgePool[Pos].Value;
    }
  }

  return true;
}

/* ------------------------------------- */

/*****************/
/* API functions */
/*****************/

// read/write 모드로 새로운 FST 만들기
// 아래의 함수들 인자중 pTransducer는 NewTransducer()의 반환값임
void *NewTransducer(void)
{
  CTransducer *pTransducer=(CTransducer *)ANMalloc(sizeof(CTransducer));

  if (pTransducer) {
    pTransducer->MaxState=0;
    pTransducer->MaxEntry=0;
    pTransducer->MaxPool=0;
    pTransducer->MaxFilled=0;
    pTransducer->EmptyHead=0;

    pTransducer->EmptyStart=0;
    pTransducer->EmptyAge=0;

    pTransducer->EdgePool=NULL;
    pTransducer->StartState=NULL_INDEX;
    for (int i=0; i<MAX_LETTER; i++) {
      pTransducer->HashTable[i].Size=0;
      pTransducer->HashTable[i].Table=NULL;
    }
    pTransducer->bReadOnly=false;
  }
  return pTransducer;
}

// FST에 할당된 메모리를 free
void FreeTransducer(void *pTransducer)
{
  CTransducer *x=(CTransducer *)pTransducer;
  if (!x->bReadOnly)
    for (int i=0; i<MAX_LETTER; i++)
      if (x->HashTable[i].Size&&x->HashTable[i].Table) {
        ANFree(x->HashTable[i].Table);
        x->HashTable[i].Size=0;
        x->HashTable[i].Table=NULL;
      }

  if (x->EdgePool) {
    ANFree(x->EdgePool);
    x->EdgePool=NULL;
  }
  ANFree(x);
}

// FST에 새로운 키 삽입하기
// key : 삽입할 키
bool RegisterKey(void *pTransducer, const char *key)
{
  CTransducer *x=(CTransducer *)pTransducer;
  CInternalState Head;
  
  if (x->bReadOnly) return false;

  Head.Index=x->StartState;
  Head.St=ID2ST(*x, x->StartState);
  Head.STS=x->MaxEntry;
  Head.St=AddPartialKey(x, &Head, (const unsigned char *)key, false);

  if (Head.St) {
    IB(Head.St)=0;
    x->StartState=ST2ID(*x, Head.St);
    (x->MaxEntry++);
    return true;
  }
  else return false;
}

// FST를 로딩하기
// ContentFilename : FST 파일 이름
// ContentFilename에 해당하는 파일이 없으면 새로 생성
// HashFilename : 부가 정보를 저장하는 파일 이름
// HashFilename이 NULL이면 read only 모드로 로딩
// HashFilename이 NULL이 아니면 read/write 모드로 로딩
// 반환값 : 성공 여부
void *LoadTransducer(const char *ContentFilename, const char *HashFilename)
{
  CTransducer *x=(CTransducer *)LoadTransducerContent(ContentFilename);
  if (x==NULL) return NULL;
  if ((HashFilename==NULL)||LoadHashTable(x, HashFilename)) return x;
  else {
    FreeTransducer(x);
    return NULL;
  }
}

// 로딩된 FST를 파일로 저장하기
// ContentFilename : FST 파일 이름
// HashFilename : 부가 정보를 저장하는 파일 이름
// FST가 read only 모드로 로딩되었거나 
// HashFilename이 NULL이면 부가 정보는 저장하지 않음
// 반환값 : 성공 여부
bool SaveTransducer(void *pTransducer, 
        const char *ContentFilename, const char *HashFilename)
{
  if (HashFilename&&!((CTransducer *)pTransducer)->bReadOnly)
    if (!SaveHashTable(pTransducer, HashFilename)) return false;

  return SaveTransducerContent(pTransducer, ContentFilename);
}

#define MAX_STRING 256

// FST에 있는 모든 키에 대해 순차적으로 Callback을 호출하는 함수
// pParam : Callback을 호출할 때 전달될 첫번째 인자
// Callback : 각각의 키에 대해 호출되는 callback 함수
//            s : 키
//            nItem : s에 해당하는 키의 개수
void TraverseTransducer(void *pTransducer, void *pParam,
      void (*Callback)(void *pParam, const char *s, int Hash, int nItem))
{
  char s[MAX_STRING];
  int Hash=0;
  CTransducer *x=(CTransducer *)pTransducer;
  DoTraverse(x, ID2ST(*x, x->StartState), s, 0, &Hash, pParam, Callback);
}

// FST가 올바른지 검사하는 함수
// 반환값 : FST가 올바른지 여부
bool CheckTransducer(void *pTransducer)
{
  CTransducer *x=(CTransducer *)pTransducer;
  int Count=CountTransducer(x, ID2ST(*x, x->StartState));

  if (Count!=x->MaxState) {
#ifdef _DEBUG
    ErrorCallback("max number of state is invalid\n");
    ErrorCallback("header information : %d\n", x->MaxState);
    ErrorCallback("actual number : %d\n", Count);
#endif
    return false;
  }
  return CheckEmptyEdge(x);
}

/* 문자열 -> 해쉬값 */
/* *nItem : String에 일치하는 엔트리의 개수, nItem이 NULL이면 무시 */
/* 반환값 : String에 일치하는 첫번째 엔트리의 Hash Value 또는 NULL_INDEX(검색 실패) */
int String2Hash(void *pTransducer, const char *String, int *nItem)
{
  CTransducer *x=(CTransducer *)pTransducer;
  unsigned char *p=(unsigned char *)String;
  CState CurrentState;
  int Hash=0;

  if (nItem) *nItem=0;
  if ((x==NULL)||(p==NULL)) return NOT_EXIST;

  CurrentState=ID2ST(*x, x->StartState);
  if (!CurrentState) return NOT_EXIST;

  while (*p!=0) {
    if (!IsValidEdge(CurrentState, *p)) return NOT_EXIST;
    else {
      Hash+=VL(CurrentState, *p);
      CurrentState=ID2ST(*x, NT(CurrentState, *p));
      p++;
    }
  }

  if (FI(CurrentState)>0) {
    if (nItem) *nItem=FI(CurrentState);
    return Hash;
  }
  else return NOT_EXIST;
}

/* 해쉬값 -> 문자열 */
/* 반환값 : 해쉬값이 부적절할 경우 NULL, 그렇지 않으면 String을 반환 */
char *Hash2String(void *pTransducer, int HashValue, char *String)
{
  CTransducer *x=(CTransducer *)pTransducer;
  unsigned char *p=(unsigned char *)String;
  CState CurrentState;
  unsigned int i;

  if (x==NULL) { *p=0; return NULL; }
  CurrentState=ID2ST(*x, x->StartState);
  if (!CurrentState) return NULL;

  while (HashValue>=FI(CurrentState)) {
    *p=0;
    for (i=0; i<MAX_LETTER; i++)
      if (IsValidEdge(CurrentState, i))
        if (VL(CurrentState, i)>HashValue) break;
        else *p=i;
    if (*p==0) { String[0]=0; return NULL; }
    HashValue-=VL(CurrentState, *p);
    CurrentState=ID2ST(*x, NT(CurrentState, *p));
    p++;
  }

  *p=0;
  return String;
}

int FindAll(CTransducer *x, unsigned char *p, CState CurrentState, int Hash,
            int IncompleteChar, char *s0, char *s,
            void *pParam, bool Callback(void *pParam, const char *s, int Hash))
{
  unsigned int i, j;
  int n=0, m;

  if (CurrentState==NULL) return 0;

  if (!IncompleteChar&&(*p==0)) {
    *s=0;
    if (FI(CurrentState)>0) 
      for (j=0; j<FI(CurrentState); j++) 
        if (!Callback(pParam, s0, Hash+j))
          return -1;
    return FI(CurrentState);
  }

  if (!IncompleteChar&&(*p=='*'))
    if ((m=FindAll(x, p+1, CurrentState, Hash, 0, s0, s, pParam, Callback))==-1)
      return -1;
    else n+=m;

  for (i=0; i<MAX_LETTER; i++)
    if ((*p=='*')||(*p=='?')||(*p==i)) 
      if (IsValidEdge(CurrentState, i)) {
        *s=i;
        if (*p=='*') {
          if ((m=FindAll(x, p, ID2ST(*x, NT(CurrentState, i)), Hash+VL(CurrentState, i), 
                         (!IncompleteChar)&&(i>=0x80), s0, s+1, pParam, Callback))==-1) 
            return -1;
          else n+=m;
        }
        else {
          if ((*p=='?')&&!IncompleteChar&&(*(p+1)!='?')&&(i>=0x80))
            continue;
          if (IncompleteChar) {
            if ((m=FindAll(x, p+1, ID2ST(*x, NT(CurrentState, i)), Hash+VL(CurrentState, i),
                           0, s0, s+1, pParam, Callback))==-1)
              return -1;
            else n+=m;
          }
          else {
            if ((m=FindAll(x, p+1, ID2ST(*x, NT(CurrentState, i)), Hash+VL(CurrentState, i),
                           i>=0x80, s0, s+1, pParam, Callback))==-1)
              return -1;
            else n+=m;
          }
        }
      }

  return n;
}

#define MaxPatternLength 1024

/* 패턴에 일치하는 키 탐색 */
/* Pattern : 검색할 엔트리의 패턴(와일드카드 *, ? 사용 가능) */
/* pParam : Callback을 호출할 때 첫번째 인자로 전달될 값 */
/* Callback : Pattern에 부합되는 엔트리가 탐색될 때마다 
              그 엔트리의 스트링과 해쉬값을 가지고 호출되는 callback */
/* 반환값 : pattern에 일치되는 엔트리의 개수 */
int Pattern2Hash(void *pTransducer, const char *Pattern, void *pParam,
     bool (*Callback)(void *pParam, const char *s, int Hash))
{
  CTransducer *x=(CTransducer *)pTransducer;
  char s[MaxPatternLength];

  return FindAll(x, (unsigned char *)Pattern, ID2ST(*x, x->StartState),
     0, 0, s, s, pParam, Callback);
}

/* 부분 문자열(prefix) 탐색 */
/* String : 탐색하려는 문자열 */
/* Hash : 부분 문자열의 해쉬값(String의 길이만큼의 정수 배열) */
/* nItem : 부분 문자열에 해당하는 키의 개수(String의 길이만큼의 정수 배열) */
/* 호출 결과 String[0]~String[n]에 해당하는 부분 문자열이 
   fst에 등록되어 있으면 Hash[n]은 부분 문자열의 해쉬값, 
                         nItem[n]은 중복키의 개수로
   그렇지 않으면 Hash[n]은 NULL_INDEX, 
                 nItem[n]은 0으로 세팅됨 */
void SubString2Hash(void *pTransducer, const char *String, int *Hash, int *nItem)
{
  CTransducer *x=(CTransducer *)pTransducer;
  CState CurrentState;
  int EachHash=0, i, n=strlen(String);
  unsigned char *p=(unsigned char *)String;
  
  for (i=0; i<n; i++) {
    Hash[i]=NOT_EXIST;
    nItem[i]=0;
  }

  if (x==NULL) return;
  CurrentState=ID2ST(*x, x->StartState);
  if (!CurrentState) return;

  // do transition
  while (*p!=0) {
    if (!IsValidEdge(CurrentState, *p)) return;
    else {
      EachHash+=VL(CurrentState, *p);
      CurrentState=ID2ST(*x, NT(CurrentState, *p));
      p++;
    }
    if (FI(CurrentState)>0) {
      Hash[p-(unsigned char *)String-1]=EachHash;
      nItem[p-(unsigned char *)String-1]=FI(CurrentState);
    }
  }
}

/* tabular parsing을 위한 테이블 구축 */
/* fst에 등록된 키들을 기반으로 String을 tabular parsing하기 위한 table을 구축 */
/* String : tabular parsing을 수행할 문자열 */
/* pParam : Callback을 호출할 때 첫번째 인자로 전달될 값 */
/* Callback : String[From]~String[From+Length-1]에 해당하는 부분 문자열이 
              FST에 등록되어 있으면,
              그 부분 문자열의 해쉬값을 Hash에 대입하여 호출되는 callback */
/*            Size : String의 길이 */
void String2Tabular(void *pTransducer, const char *String, void *pParam, 
        void (*Callback)(void *pParam, int Size, int From, int Length, int Hash))
{
  CTransducer *x=(CTransducer *)pTransducer;
  CState CurrentState;
  int Hash, i, j, n=strlen(String);
  unsigned char *p=(unsigned char *)String;

  if (x==NULL) return;

  for (i=0; i<n; i++) { // i += 2 // 2바이트 문자열 처리를 위해서
    CurrentState=ID2ST(*x, x->StartState);
    if (!CurrentState) continue;
    Hash=0;
    p=(unsigned char *)String+i;
    // do transition
    while (*p!=0) {
      if (!IsValidEdge(CurrentState, *p)) break;
      else {
        Hash+=VL(CurrentState, *p);
        CurrentState=ID2ST(*x, NT(CurrentState, *p));
        p++;
      }
      if (FI(CurrentState)>0)
        for (j=0; j<FI(CurrentState); j++)
          Callback(pParam, n, i, (p-(unsigned char *)String)-i, Hash+j); // 이 부분만 수정하면 2바이트 문자열에 대한 처리 가능
    }
  }
}

// newly created by dglee based on String2Tabular
/* tabular parsing을 위한 테이블 구축 */
/* fst에 등록된 키들을 기반으로 String을 tabular parsing하기 위한 table을 구축 */
/* String : tabular parsing을 수행할 문자열 */
/* pParam : Callback을 호출할 때 첫번째 인자로 전달될 값 */
/* Callback : String[From]~String[From+Length-1]에 해당하는 부분 문자열이 
              FST에 등록되어 있으면,
              그 부분 문자열의 해쉬값을 Hash에 대입하여 호출되는 callback */
/*            Size : String의 길이 */
void String2Tabular_2byte(void *pTransducer, const char *String, void *pParam, 
        void (*Callback)(void *pParam, int Size, int From, int Length, int Hash))
{
  CTransducer *x=(CTransducer *)pTransducer;
  CState CurrentState;
  int Hash, i, j, n=strlen(String);
  unsigned char *p=(unsigned char *)String;

  if (x==NULL) return;

  for (i=0; i<n; i+=2) { // 2바이트 문자열 처리를 위해서
    CurrentState=ID2ST(*x, x->StartState);
    if (!CurrentState) continue;
    Hash=0;
    p=(unsigned char *)String+i;
    // do transition
    while (*p!=0) {
      if (!IsValidEdge(CurrentState, *p)) break;
      else {
        Hash+=VL(CurrentState, *p);
        CurrentState=ID2ST(*x, NT(CurrentState, *p));
        p++;
      }
      if (FI(CurrentState)>0)
        for (j=0; j<FI(CurrentState); j++)
          Callback(pParam, n/2, i/2, ((p-(unsigned char *)String)-i)/2, Hash+j); // 이 부분만 수정하면 2바이트 문자열에 대한 처리 가능
    }
  }
}


/* 문자열 -> 최장 일치 키의 해쉬값 */
/* nItem : String에 최장 일치하는 엔트리의 개수 */
/* 반환값 : String에 최장 일치하는 첫번째 엔트리의 Hash Value 또는 NULL_INDEX(검색 실패) */
/* 주의 : Last가 없는 버전 */
int String2LongestMatchedHash(void *pTransducer, const char *String, int *nItem)
{
  CTransducer *x=(CTransducer *)pTransducer;
  unsigned char *p=(unsigned char *)String;
  CState CurrentState;
  int Hash=0, Result=NOT_EXIST;

  if (nItem) *nItem=0;
  if ((x==NULL)||(p==NULL)) return NOT_EXIST;

  CurrentState=ID2ST(*x, x->StartState);
  if (!CurrentState) return NOT_EXIST;

  while (*p!=0) {
    if (!IsValidEdge(CurrentState, *p)) return Result;
    else {
      Hash+=VL(CurrentState, *p);
      CurrentState=ID2ST(*x, NT(CurrentState, *p));
      p++;
    }
    if (FI(CurrentState)>0) {
      Result=Hash;
      if (nItem) *nItem=FI(CurrentState);
    }
  }

  if (FI(CurrentState)>0) {
    if (nItem) *nItem=FI(CurrentState);
    return Hash;
  }
  else return Result;
}

/* 문자열 -> 가장 유사한 키의 해쉬값 */
/* nItem : String에 가장 유사한 엔트리의 개수 */
/* 반환값 : String에 가장 유사한 첫번째 엔트리의 Hash Value 또는 NULL_INDEX(검색 실패) */
/* 주의 : Last가 없는 버전 */
int String2MostSimilarHash(void *pTransducer, const char *String, int *nItem)
{
  CTransducer *x=(CTransducer *)pTransducer;
  unsigned char *p=(unsigned char *)String;
  CState CurrentState;
  int c, Hash=0, Result=NOT_EXIST;
  int Flag=0/* 0 : 정상적으로 일치, 
               1 : 이후의 상태에서 가장 큰 입력을 받는 상태로 전이, 
               2 : 이후의 상태에서 가장 작은 입력을 받는 상태로 전이 */;

  if (nItem) *nItem=0;
  if ((x==NULL)||(p==NULL)) return NOT_EXIST;

  CurrentState=ID2ST(*x, x->StartState);
  if (!CurrentState) return NOT_EXIST;

  // 정상적으로 일치되는 상태가 있는 동안
  while ((*p!=0)&&(Flag==0)) {
    c=p[0];
    if (!IsValidEdge(CurrentState, c)) {
      for (c=p[0]+1; c<MAX_LETTER; c++)
        if (IsValidEdge(CurrentState, c)) {
          Flag=2;
          break;
        }
      if (c==MAX_LETTER)
        for (c=p[0]-1; c>=0; c--)
          if (IsValidEdge(CurrentState, c)) {
            Flag=1;
            break;
          }
    }
    if (c<0)
      return Result;
    Hash+=VL(CurrentState, c);
    CurrentState=ID2ST(*x, NT(CurrentState, c));
    if (FI(CurrentState)>0) {
      if (nItem) *nItem=FI(CurrentState);
      Result=Hash;
    }
    p++;
  }

  while (1) {
    switch (Flag) {
    case 1: // 가장 큰 입력을 받는 전이를 탐색
      if (FI(CurrentState)>0) {
        if (nItem) *nItem=FI(CurrentState);
        Result=Hash;
      }
      for (c=MAX_LETTER-1; c>=0; c--)
        if (IsValidEdge(CurrentState, c))
          break;
      if (c<0) 
        return Result;
      break;
    case 0:
    case 2: // 가장 작은 입력을 받는 전이를 탐색
      if (FI(CurrentState)>0) {
        if (nItem) *nItem=FI(CurrentState);
        return Hash;
      }
      for (c=0; c<MAX_LETTER; c++)
        if (IsValidEdge(CurrentState, c))
          break;
      if (c==MAX_LETTER) 
        return Result;
      break;
    }
    Hash+=VL(CurrentState, c);
    CurrentState=ID2ST(*x, NT(CurrentState, c));
  }

  if (nItem) *nItem=1;
  return 0;
}


bool DeleteMain(CTransducer *pTransducer, CInternalState *pState,
    const unsigned char *s, int No)
{
  CInternalState Child;

  // clone a state if its inbound is greater than 2
  if (IB(pState->St)>1) {
    if ((pState->St=CloneState(pTransducer, pState->St, true))==NULL) 
      return false;
    pState->Index=NULL_INDEX; // notify the state is cloned
  }
  else if (!DeregisterState(pTransducer, pState->St)) return false;

  // if final character
  if (s[0]==0) {
    bool DeleteOne=(No>=0)&&(No<FI(pState->St));
    // adjust the transition value of child links
    for (int i=0; i<MAX_LETTER; i++)
      if (IsValidEdge(pState->St, i))
        VL(pState->St, i)-=(DeleteOne?1:FI(pState->St));
    pState->STS-=(DeleteOne?1:FI(pState->St));
    FI(pState->St)-=(DeleteOne?1:FI(pState->St));
  }
  else {
    // build internal representation of child state
    Child.Index=NT(pState->St, s[0]);
    Child.St=ID2ST(*pTransducer, Child.Index);
    int NextLink, STS;
    for (NextLink=s[0]+1; NextLink<MAX_LETTER; NextLink++)
      if (IsValidEdge(pState->St, NextLink)) break;
    if (NextLink==MAX_LETTER)
      Child.STS=pState->STS-VL(pState->St, s[0]);
    else 
      Child.STS=VL(pState->St, NextLink)-VL(pState->St, s[0]);
    STS=Child.STS;

    // do deletion on child state
    if (!DeleteMain(pTransducer, &Child, s+1, No)) return false;

    // adjust pState->St because realloc may have been called during DeleteMain
    if (pState->Index!=NULL_INDEX) 
      pState->St=ID2ST(*pTransducer, pState->Index);

    // if the child node was deleted
    if (Child.Index==NULL_INDEX) {
      // clear the link for the child
      memset(pState->St+s[0], 0xff, sizeof(CEdge));
      // if this state is flattened, adjust link for empty edges
      if (pState->Index!=NULL_INDEX) {
        int LastPos;
        if (pTransducer->EmptyHead>=(ST2ID(*pTransducer, pState->St)+s[0]))
          LastPos=-1;
        else for (LastPos=ST2ID(*pTransducer, pState->St)+s[0]-1; 
                  LastPos>=0; LastPos--)
          if (IsEmptyEdge(pTransducer, pTransducer->EdgePool+LastPos)) 
            break;
        if (LastPos==-1) {
          VL(pState->St, s[0])=pTransducer->EmptyHead;
          pTransducer->EmptyHead=ST2ID(*pTransducer, pState->St)+s[0];
        }
        else {
          VL(pState->St, s[0])=pTransducer->EdgePool[LastPos].Value;
          pTransducer->EdgePool[LastPos].Value=
            ST2ID(*pTransducer, pState->St)+s[0];
        }
      }
    }

    // modify link
    NT(pState->St, s[0])=Child.Index;
    // adjust the transditon value of following links
    for (; NextLink<MAX_LETTER; NextLink++)
      if (IsValidEdge(pState->St, NextLink))
        VL(pState->St, NextLink)-=(STS-Child.STS);
    pState->STS-=(STS-Child.STS);
  }

  if (pState->STS==0) {
    if (pState->Index==NULL_INDEX) 
      FreeState(pState->St);
    else RemoveState(pTransducer, pState->St);
    pState->St=NULL;
    pState->Index=NULL_INDEX;
    pState->STS=0;
  }
  else {
    pState->Index=RegisterOrOptimize(pTransducer, pState->St, 
             pState->Index!=NULL_INDEX);
    if (pState->Index==NULL_INDEX) return false;
    pState->St=ID2ST(*pTransducer, pState->Index);
  }
  return true;
}

// FST에서 키를 제거하기
// Key : 삭제할 키
// Key가 없으면 아무런 작업도 하지 않음(반환값은 true)
// Hash : 삭제할 키의 해쉬값
//   -> 중복된 키가 여러 개일 경우 Hash값에 해당하는 키만 삭제
// Hash가 NOT_EXIST이거나 Key의 해쉬값이 아니면 중복된 키를 모두 삭제
// 삭제 도중 오류가 발생하면 false를 반환
bool DeleteKey(void *pTransducer, const char *Key, int Hash)
{
  CTransducer *x=(CTransducer *)pTransducer;
  CInternalState State;
  int STS, No, nItem;

  if ((No=String2Hash(pTransducer, Key, &nItem))!=NOT_EXIST) {
    if ((Hash>=No)&&(Hash<No+nItem))
      No=Hash-No;
    else No=NOT_EXIST;
    State.Index=x->StartState;
    State.St=ID2ST(*x, State.Index);
    State.STS=x->MaxEntry;
    STS=State.STS;
    if (!DeleteMain(x, &State, (const unsigned char *)Key, No)) return false;
    x->StartState=State.Index;
    if (State.St) IB(State.St)=0;
    x->MaxEntry-=(STS-State.STS);
  }
  
  return true;
}

// FST의 모든 키를 삭제(테스트용)
bool DeleteAll(void *pTransducer)
{
  CTransducer *x=(CTransducer *)pTransducer;
  int Hash;
  char s[100];

  while (x->MaxEntry) {
    Hash=rand()%x->MaxEntry;
    Hash2String(x, Hash, s);
    if (!DeleteKey(x, s, Hash)) {
#ifdef _DEBUG
      ErrorCallback("cannot delete %s %d", s, Hash);
#endif
      return false;
    }
  }
  return true;
}

int GetNumberOfEntry(void *pTransducer)
{
  return ((CTransducer *)pTransducer)->MaxEntry;
}


// added by dglee
// FST 만들기
int build_fst(char *keyfilename, char *fstfilename, char *hashfilename) {
  void *x;
  FILE *f;
  char s[1024];
  int n=0;

  if ((f=fopen(keyfilename, "rb"))==NULL) {
    fprintf(stderr, "ERROR :: cannot open key file %s\n", keyfilename);
    return 0;
  }

  if ((x=NewTransducer())==NULL) {
    fprintf(stderr, "ERROR :: cannot make new FST\n");
    fclose(f);
    return 0;
  }

  while (ANFgetsWithTrim(s, 1024, f)) {
    if (s[0]==0) {
      fprintf(stderr, "WARNING :: empty key was detected on line %d\n", n);
      continue;
    }
    if (!RegisterKey(x, s)) {
      fprintf(stderr, "ERROR :: cannot register key %s\n", s);
      FreeTransducer(x);
      fclose(f);
      return 0;
    }
    n++;
    if (n%1000==0)
      fprintf(stderr, "\radded %d keys", n);
  }
  fprintf(stderr, "\radded total %d keys\n", n);

  fprintf(stderr, "saving files...\n");
  if (!SaveTransducer(x, fstfilename, hashfilename)) 
    fprintf(stderr, "ERROR :: cannot save FST to %s %s\n", fstfilename, hashfilename);

  FreeTransducer(x);
  fclose(f);
  fprintf(stderr, "done\n");
  return 1; 
}
