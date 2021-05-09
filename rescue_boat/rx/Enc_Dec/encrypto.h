#ifndef _TRI_DES_H_
#define _TRI_DES_H_

typedef char ElemType; 
typedef unsigned int u32;
//int DES_EncryptBlock(ElemType plainBlock[8], ElemType subKeys[16][48], ElemType cipherBlock[8]);
//int DES_DecryptBlock(ElemType cipherBlock[8], ElemType subKeys[16][48],ElemType plainBlock[8]);

void Boot_Encrypt(char *plainBlock, char *keyStr,char *cipherBlock,u32 size);
void Boot_Decrypt(char *cipherBlock, char *keyStr,char *plainBlock,u32 size);


#endif


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
