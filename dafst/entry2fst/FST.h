#ifndef __FST__
#define __FST__

#define NOT_EXIST (-1) // 존재하지 않는 키에 대한 해쉬값

// read/write 모드로 새로운 FST 만들기
// 아래의 함수들 인자중 pTransducer는 NewTransducer()의 반환값임
extern void *NewTransducer(void);

// FST에 할당된 메모리를 free
extern void FreeTransducer(void *pTransducer);

// FST에 새로운 키 삽입하기
// key : 삽입할 키
extern bool RegisterKey(void *pTransducer, const char *key);

// FST에서 키를 제거하기
// Key : 삭제할 키
// Key가 없으면 아무런 작업도 하지 않음(반환값은 true)
// Hash : 삭제할 키의 해쉬값
//   -> 중복된 키가 여러 개일 경우 Hash값에 해당하는 키만 삭제
// Hash가 NOT_EXIST이거나 Key의 해쉬값이 아니면 중복된 키를 모두 삭제
// 삭제 도중 오류가 발생하면 false를 반환
extern bool DeleteKey(void *pTransducer, const char *Key, int Hash);

// FST의 모든 키를 삭제(테스트용)
extern bool DeleteAll(void *pTransducer);

// FST를 로딩하기
// ContentFilename : FST 파일 이름
// ContentFilename에 해당하는 파일이 없으면 NULL 반환
// HashFilename : 부가 정보를 저장하는 파일 이름
// HashFilename이 NULL이면 read only 모드로 로딩
// HashFilename이 NULL이 아니면 read/write 모드로 로딩
// 반환값 : 성공 여부
extern void *LoadTransducer(const char *ContentFilename, const char *HashFilename);

// 로딩된 FST를 파일로 저장하기
// ContentFilename : FST 파일 이름
// HashFilename : 부가 정보를 저장하는 파일 이름
// FST가 read only 모드로 로딩되었거나 
// HashFilename이 NULL이면 부가 정보는 저장하지 않음
// 반환값 : 성공 여부
extern bool SaveTransducer(void *pTransducer, const char *ContentFilename, 
		    const char *HashFilename);

// FST에 들어있는 엔트리의 개수
extern int GetNumberOfEntry(void *pTransducer);

// FST에 있는 모든 키에 대해 순차적으로 Callback을 호출하는 함수
// pParam : Callback을 호출할 때 전달될 첫번째 인자
// Callback : 각각의 키에 대해 호출되는 callback 함수
//            s : 키
//            nItem : s에 해당하는 키의 개수
extern void TraverseTransducer(void *pTransducer, void *pParam,
			void (*Callback)(void *pParam, const char *s, int Hash, int nItem));

// FST가 올바른지 검사하는 함수
// 반환값 : FST가 올바른지 여부
extern bool CheckTransducer(void *pTransducer);

/* 문자열 -> 해쉬값 */
/* *nItem : String에 일치하는 엔트리의 개수, nItem이 NULL이면 무시 */
/* 반환값 : String에 일치하는 첫번째 엔트리의 Hash Value 또는 NULL_INDEX(검색 실패) */
extern int String2Hash(void *pTransducer, const char *String, int *nItem);

/* 문자열 -> 최장 일치 키의 해쉬값 */
/* nItem : String에 최장 일치하는 엔트리의 개수 */
/* 반환값 : String에 최장 일치하는 첫번째 엔트리의 Hash Value 또는 NULL_INDEX(검색 실패) */
/* 주의 : Last가 없는 버전 */
extern int String2LongestMatchedHash(void *pTransducer, const char *String, int *nItem);

/* 문자열 -> 가장 유사한 키의 해쉬값 */
/* nItem : String에 가장 유사한 엔트리의 개수 */
/* 반환값 : String에 가장 유사한 첫번째 엔트리의 Hash Value 또는 NULL_INDEX(검색 실패) */
/* 주의 : Last가 없는 버전 */
extern int String2MostSimilarHash(void *pTransducer, const char *String, int *nItem);

/* 해쉬값 -> 문자열 */
/* 반환값 : 해쉬값이 부적절할 경우 NULL, 그렇지 않으면 String을 반환 */
extern char *Hash2String(void *pTransducer, int HashValue, char *String);

/* 패턴에 일치하는 키 탐색 */
/* Pattern : 검색할 엔트리의 패턴(와일드카드 *, ? 사용 가능) */
/* pParam : Callback을 호출할 때 첫번째 인자로 전달될 값 */
/* Callback : Pattern에 부합되는 엔트리가 탐색될 때마다 
              그 엔트리의 스트링과 해쉬값을 가지고 호출되는 callback */
/* 반환값 : pattern에 일치되는 엔트리의 개수 */
extern int Pattern2Hash(void *pTransducer, const char *Pattern, void *pParam,
		 bool (*Callback)(void *pParam, const char *s, int Hash));

/* 부분 문자열(prefix) 탐색 */
/* String : 탐색하려는 문자열 */
/* Hash : 부분 문자열의 해쉬값(String의 길이만큼의 정수 배열) */
/* nItem : 부분 문자열에 해당하는 키의 개수(String의 길이만큼의 정수 배열) */
/* 호출 결과 String[0]~String[n]에 해당하는 부분 문자열이 
   fst에 등록되어 있으면 Hash[n]은 부분 문자열의 해쉬값, 
                         nItem[n]은 중복키의 개수로
   그렇지 않으면 Hash[n]은 NULL_INDEX, 
                 nItem[n]은 0으로 세팅됨 */
extern void SubString2Hash(void *pTransducer, const char *String, int *Hash, int *nItem);

/* tabular parsing을 위한 테이블 구축 */
/* fst에 등록된 키들을 기반으로 String을 tabular parsing하기 위한 table을 구축 */
/* String : tabular parsing을 수행할 문자열 */
/* pParam : Callback을 호출할 때 첫번째 인자로 전달될 값 */
/* Callback : String[From]~String[From+Length-1]에 해당하는 부분 문자열이 
              FST에 등록되어 있으면
              그 부분 문자열의 해쉬값을 Hash에 대입하여 호출되는 callback */
/*            Size : String의 길이 */
extern void String2Tabular(void *pTransducer, const char *String, void *pParam, 
		    void (*Callback)(void *pParam, int Size, int From, int Length, int Value));

/* 1차원 배열로 구성된 Table의 cell에 접근하는 매크로 
   From은 0부터 시작, Length는 1부터 시작 */
#define CELL(Table, Size, From, Length) \
((Table)[(From)*(2*(Size)-(From)+1)/2+(Length)-1])


/* added by dglee */

// 삼각테이블의 모양이 다르다.
// ***
// **
// *
#define Tab2Pos(Size, From, Length)   ((From)*(2*(Size)-(From)+1)/2+(Length)-1)   /* origin by leeho : Size는 불변, From = 시작위치, Length = 문자열 길이 */


// ***
//  **
//   *
#define TabPos2(x, y, n)  ((x)*(2*(n)-(x)-1)/2+(y)-1) /* from, size, length :From = 행번호, Length = 열번호 */



#define TabNum(x) ((x)*((x)+1)/2) /* added by dglee */

extern int build_fst(char *keyfilename, char *fstfilename, char *hashfilename);

#endif
