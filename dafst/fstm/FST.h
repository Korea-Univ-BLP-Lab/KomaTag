#ifndef __FST__
#define __FST__

#define NOT_EXIST (-1) // �������� �ʴ� Ű�� ���� �ؽ���

// read/write ���� ���ο� FST �����
// �Ʒ��� �Լ��� ������ pTransducer�� NewTransducer()�� ��ȯ����
void *NewTransducer(void);

// FST�� �Ҵ�� �޸𸮸� free
void FreeTransducer(void *pTransducer);

// FST�� ���ο� Ű �����ϱ�
// key : ������ Ű
bool RegisterKey(void *pTransducer, const char *key);

// FST���� Ű�� �����ϱ�
// Key : ������ Ű
// Key�� ������ �ƹ��� �۾��� ���� ����(��ȯ���� true)
// Hash : ������ Ű�� �ؽ���
//   -> �ߺ��� Ű�� ���� ���� ��� Hash���� �ش��ϴ� Ű�� ����
// Hash�� NOT_EXIST�̰ų� Key�� �ؽ����� �ƴϸ� �ߺ��� Ű�� ��� ����
// ���� ���� ������ �߻��ϸ� false�� ��ȯ
bool DeleteKey(void *pTransducer, const char *Key, int Hash);

// FST�� ��� Ű�� ����(�׽�Ʈ��)
bool DeleteAll(void *pTransducer);

// FST�� �ε��ϱ�
// ContentFilename : FST ���� �̸�
// ContentFilename�� �ش��ϴ� ������ ������ NULL ��ȯ
// HashFilename : �ΰ� ������ �����ϴ� ���� �̸�
// HashFilename�� NULL�̸� read only ���� �ε�
// HashFilename�� NULL�� �ƴϸ� read/write ���� �ε�
// ��ȯ�� : ���� ����
void *LoadTransducer(const char *ContentFilename, const char *HashFilename);

// �ε��� FST�� ���Ϸ� �����ϱ�
// ContentFilename : FST ���� �̸�
// HashFilename : �ΰ� ������ �����ϴ� ���� �̸�
// FST�� read only ���� �ε��Ǿ��ų� 
// HashFilename�� NULL�̸� �ΰ� ������ �������� ����
// ��ȯ�� : ���� ����
bool SaveTransducer(void *pTransducer, const char *ContentFilename, 
		    const char *HashFilename);

// FST�� ����ִ� ��Ʈ���� ����
int GetNumberOfEntry(void *pTransducer);

// FST�� �ִ� ��� Ű�� ���� ���������� Callback�� ȣ���ϴ� �Լ�
// pParam : Callback�� ȣ���� �� ���޵� ù��° ����
// Callback : ������ Ű�� ���� ȣ��Ǵ� callback �Լ�
//            s : Ű
//            nItem : s�� �ش��ϴ� Ű�� ����
void TraverseTransducer(void *pTransducer, void *pParam,
			void (*Callback)(void *pParam, const char *s, int Hash, int nItem));

// FST�� �ùٸ��� �˻��ϴ� �Լ�
// ��ȯ�� : FST�� �ùٸ��� ����
bool CheckTransducer(void *pTransducer);

/* ���ڿ� -> �ؽ��� */
/* *nItem : String�� ��ġ�ϴ� ��Ʈ���� ����, nItem�� NULL�̸� ���� */
/* ��ȯ�� : String�� ��ġ�ϴ� ù��° ��Ʈ���� Hash Value �Ǵ� NULL_INDEX(�˻� ����) */
int String2Hash(void *pTransducer, const char *String, int *nItem);

/* ���ڿ� -> ���� ��ġ Ű�� �ؽ��� */
/* nItem : String�� ���� ��ġ�ϴ� ��Ʈ���� ���� */
/* ��ȯ�� : String�� ���� ��ġ�ϴ� ù��° ��Ʈ���� Hash Value �Ǵ� NULL_INDEX(�˻� ����) */
/* ���� : Last�� ���� ���� */
int String2LongestMatchedHash(void *pTransducer, const char *String, int *nItem);

/* ���ڿ� -> ���� ������ Ű�� �ؽ��� */
/* nItem : String�� ���� ������ ��Ʈ���� ���� */
/* ��ȯ�� : String�� ���� ������ ù��° ��Ʈ���� Hash Value �Ǵ� NULL_INDEX(�˻� ����) */
/* ���� : Last�� ���� ���� */
int String2MostSimilarHash(void *pTransducer, const char *String, int *nItem);

/* �ؽ��� -> ���ڿ� */
/* ��ȯ�� : �ؽ����� �������� ��� NULL, �׷��� ������ String�� ��ȯ */
char *Hash2String(void *pTransducer, int HashValue, char *String);

/* ���Ͽ� ��ġ�ϴ� Ű Ž�� */
/* Pattern : �˻��� ��Ʈ���� ����(���ϵ�ī�� *, ? ��� ����) */
/* pParam : Callback�� ȣ���� �� ù��° ���ڷ� ���޵� �� */
/* Callback : Pattern�� ���յǴ� ��Ʈ���� Ž���� ������ 
              �� ��Ʈ���� ��Ʈ���� �ؽ����� ������ ȣ��Ǵ� callback */
/* ��ȯ�� : pattern�� ��ġ�Ǵ� ��Ʈ���� ���� */
int Pattern2Hash(void *pTransducer, const char *Pattern, void *pParam,
		 bool (*Callback)(void *pParam, const char *s, int Hash));

/* �κ� ���ڿ�(prefix) Ž�� */
/* String : Ž���Ϸ��� ���ڿ� */
/* Hash : �κ� ���ڿ��� �ؽ���(String�� ���̸�ŭ�� ���� �迭) */
/* nItem : �κ� ���ڿ��� �ش��ϴ� Ű�� ����(String�� ���̸�ŭ�� ���� �迭) */
/* ȣ�� ��� String[0]~String[n]�� �ش��ϴ� �κ� ���ڿ��� 
   fst�� ��ϵǾ� ������ Hash[n]�� �κ� ���ڿ��� �ؽ���, 
                         nItem[n]�� �ߺ�Ű�� ������
   �׷��� ������ Hash[n]�� NULL_INDEX, 
                 nItem[n]�� 0���� ���õ� */
void SubString2Hash(void *pTransducer, const char *String, int *Hash, int *nItem);

/* tabular parsing�� ���� ���̺� ���� */
/* fst�� ��ϵ� Ű���� ������� String�� tabular parsing�ϱ� ���� table�� ���� */
/* String : tabular parsing�� ������ ���ڿ� */
/* pParam : Callback�� ȣ���� �� ù��° ���ڷ� ���޵� �� */
/* Callback : String[From]~String[From+Length-1]�� �ش��ϴ� �κ� ���ڿ��� 
              FST�� ��ϵǾ� ������
              �� �κ� ���ڿ��� �ؽ����� Hash�� �����Ͽ� ȣ��Ǵ� callback */
/*            Size : String�� ���� */
void String2Tabular(void *pTransducer, const char *String, void *pParam, 
		    void (*Callback)(void *pParam, int Size, int From, int Length, int Value));

/* 1���� �迭�� ������ Table�� cell�� �����ϴ� ��ũ�� 
   From�� 0���� ����, Length�� 1���� ���� */
#define CELL(Table, Size, From, Length) \
((Table)[(From)*(2*(Size)-(From)+1)/2+(Length)-1])

#endif