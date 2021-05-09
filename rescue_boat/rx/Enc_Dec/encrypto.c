// TriDesEnc.cpp : implementation file
//
//#include<iostream> 
#include "encrypto.h"
#include "EncTable.h"

#if(ENC_MODE==_ENC_MODE_AES_)
#include<stdio.h>
#include<string.h>

// AES only supports Nb=4
#define Nb 4			// number of columns in the State & expanded key
#define Nk 4			// number of columns in a key
#define Nr 10			// number of rounds in encryption

unsigned char w[11][4][4];

#endif



#if (ENC_MODE==_ENC_MODE_3DES_)
void memcpy(void *des,void *src,u32 n)  
{  
	ElemType *xdes=des;
	ElemType *xsrc=src;
	while(n--)
	{
		*xdes++=*xsrc++;
	}
}

int ByteToBit(ElemType ch, ElemType bit[8])
{
	int cnt;
	for(cnt = 0;cnt < 8; cnt++){
		*(bit+cnt) = (ch>>cnt)&1;
	}
	return 0;
}
  
int BitToByte(ElemType bit[8],ElemType *ch)
{
	int cnt;
	for(cnt = 0;cnt < 8; cnt++){
		*ch |= *(bit + cnt)<<cnt;
	}
	return 0;
}
  
int Char8ToBit64(ElemType ch[8],ElemType bit[64])
{
	int cnt;
	for(cnt = 0; cnt < 8; cnt++){  
		ByteToBit(*(ch+cnt),bit+(cnt<<3));
	}
	return 0;
}
  
int Bit64ToChar8(ElemType bit[64],ElemType ch[8])
{
	int cnt;
	memset(ch,0,8);
	for(cnt = 0; cnt < 8; cnt++){
		BitToByte(bit+(cnt<<3),ch+cnt);
	}
	return 0;
}
  
int DES_MakeSubKeys(ElemType key[64],ElemType subKeys[16][48])
{
	ElemType temp[56];
	int cnt;
	DES_PC1_Transform(key,temp);
	for(cnt = 0; cnt < 16; cnt++){
		DES_ROL(temp,MOVE_TIMES[cnt]);
		DES_PC2_Transform(temp,subKeys[cnt]);
	}
	return 0;
}
  

int DES_PC1_Transform(ElemType key[64], ElemType tempbts[56])
{
	int cnt; 
	for(cnt = 0; cnt < 56; cnt++){
		tempbts[cnt] = key[PC_1[cnt]];
	}
	return 0;
}

int DES_PC2_Transform(ElemType key[56], ElemType tempbts[48])
{
	int cnt;
	for(cnt = 0; cnt < 48; cnt++){
		tempbts[cnt] = key[PC_2[cnt]];
	}
	return 0;
}
  
int DES_ROL(ElemType data[56], int time)
{
	ElemType temp[56];
	
	memcpy(temp,data,time);
	memcpy(temp+time,data+28,time);
	
	memcpy(data,data+time,28-time);
	memcpy(data+28-time,temp,time);
	
	memcpy(data+28,data+28+time,28-time);
	memcpy(data+56-time,temp+time,time); 
	
	return 0;
}
  
int DES_IP_Transform(ElemType data[64])
{
	int cnt;
	ElemType temp[64];
	for(cnt = 0; cnt < 64; cnt++){
		temp[cnt] = data[IP_Table[cnt]];
	}
	memcpy(data,temp,64);
	return 0;
}
  
int DES_IP_1_Transform(ElemType data[64])
{
	int cnt;
	ElemType temp[64];
	for(cnt = 0; cnt < 64; cnt++){
		temp[cnt] = data[IP_1_Table[cnt]];
	}
	memcpy(data,temp,64);
	return 0;
}
  
int DES_E_Transform(ElemType data[48])
{
	int cnt;
	ElemType temp[48];
	for(cnt = 0; cnt < 48; cnt++){
		temp[cnt] = data[E_Table[cnt]];
	}
	memcpy(data,temp,48);
	return 0;
}
  
int DES_P_Transform(ElemType data[32])
{
	int cnt;
	ElemType temp[32];
	for(cnt = 0; cnt < 32; cnt++){
		temp[cnt] = data[P_Table[cnt]];
	}
	memcpy(data,temp,32);
	return 0;
}
  
int DES_XOR(ElemType R[48], ElemType L[48] ,int count)
{
	int cnt;
	for(cnt = 0; cnt < count; cnt++){
		R[cnt] ^= L[cnt];
	}
	return 0;
}
  
int DES_SBOX(ElemType data[48])
{
	int cnt;
	int line,row,output;
	int cur1,cur2;
	for(cnt = 0; cnt < 8; cnt++){
		cur1 = cnt*6;
		cur2 = cnt<<2;
		
		line = (data[cur1]<<1) + data[cur1+5];
		row = (data[cur1+1]<<3) + (data[cur1+2]<<2)
			+ (data[cur1+3]<<1) + data[cur1+4];
		output = S[cnt][line][row];
		
		data[cur2] = (output&0X08)>>3;
		data[cur2+1] = (output&0X04)>>2;
		data[cur2+2] = (output&0X02)>>1;
		data[cur2+3] = output&0x01;
 }
 return 0;
}
  
int DES_Swap(ElemType left[32], ElemType right[32])
{
	ElemType temp[32];
	memcpy(temp,left,32);
	memcpy(left,right,32);  
	memcpy(right,temp,32);
	return 0;
}
  
/*加密单个分组*/
int DES_EncryptBlock(ElemType plainBlock[8], ElemType subKeys[16][48], ElemType cipherBlock[8])
{
	ElemType plainBits[64];
	ElemType copyRight[48];
	int cnt;
	
	Char8ToBit64(plainBlock,plainBits);  
	/*初始置换（IP置换）*/
	DES_IP_Transform(plainBits);
	
	/*16轮迭代*/
	for(cnt = 0; cnt < 16; cnt++){ 
		memcpy(copyRight,plainBits+32,32);
		/*将右半部分进行扩展置换，从32位扩展到48位*/
		DES_E_Transform(copyRight);
		/*将右半部分与子密钥进行异或操作*/
		DES_XOR(copyRight,subKeys[cnt],48); 
		/*异或结果进入S盒，输出32位结果*/
		DES_SBOX(copyRight);
		/*P置换*/
		DES_P_Transform(copyRight);
		/*将明文左半部分与右半部分进行异或*/
		DES_XOR(plainBits,copyRight,32);
		if(cnt != 15){
			/*最终完成左右部的交换*/
			DES_Swap(plainBits,plainBits+32);
		}
	}
	/*逆初始置换（IP^1置换）*/
	DES_IP_1_Transform(plainBits);
	Bit64ToChar8(plainBits,cipherBlock);
	return 0;
}
  
/*解密单个分组*/
int DES_DecryptBlock(ElemType cipherBlock[8], ElemType subKeys[16][48],ElemType plainBlock[8])
{
	ElemType cipherBits[64];
	ElemType copyRight[48];
	int cnt;
	
	Char8ToBit64(cipherBlock,cipherBits); 
	/*初始置换（IP置换）*/
	DES_IP_Transform(cipherBits);
	
	/*16轮迭代*/
	for(cnt = 15; cnt >= 0; cnt--){
		memcpy(copyRight,cipherBits+32,32);
		/*将右半部分进行扩展置换，从32位扩展到48位*/
		DES_E_Transform(copyRight);
		/*将右半部分与子密钥进行异或操作*/
		DES_XOR(copyRight,subKeys[cnt],48);  
		/*异或结果进入S盒，输出32位结果*/
		DES_SBOX(copyRight);
		/*P置换*/
		DES_P_Transform(copyRight);  
		/*将明文左半部分与右半部分进行异或*/
		DES_XOR(cipherBits,copyRight,32);
		if(cnt != 0){
			/*最终完成左右部的交换*/
			DES_Swap(cipherBits,cipherBits+32);
		}
	}
	/*逆初始置换（IP^1置换）*/
	DES_IP_1_Transform(cipherBits);
	Bit64ToChar8(cipherBits,plainBlock);
	return 0;
}

int DES_Encrypt_Section(char *plainBlock, char *keyStr,char *cipherBlock)
{
	//ElemType cipherBlock[8],
	ElemType keyBlock[8];
	ElemType bKey[64];
	ElemType subKeys[16][48];
	memcpy(keyBlock,keyStr,8);
	Char8ToBit64(keyBlock,bKey);
	DES_MakeSubKeys(bKey,subKeys);
	DES_EncryptBlock(plainBlock,subKeys,cipherBlock);
}

int DES_Decrypt_Section(char *cipherBlock, char *keyStr,char *plainBlock)
{
	//ElemType cipherBlock[8],keyBlock[8];
	ElemType keyBlock[8];
	ElemType bKey[64];
	ElemType subKeys[16][48];
	memcpy(keyBlock,keyStr,8);
	Char8ToBit64(keyBlock,bKey);
	DES_MakeSubKeys(bKey,subKeys);
	DES_DecryptBlock(cipherBlock,subKeys,plainBlock);	
}

void Boot_Encrypt(char *plainBlock, char *keyStr,char *cipherBlock, u32 size)
{
	u32 i=0;
	for(i=0;i<size;i+=8)
	{
		DES_Encrypt_Section(plainBlock+i,keyStr,cipherBlock+i);
	}
}

void Boot_Decrypt(char *cipherBlock, char *keyStr,char *plainBlock,u32 size)
{
	u32 i=0;
	for(i=0;i<size;i+=8)
	{
		DES_Decrypt_Section(plainBlock+i,keyStr,cipherBlock+i);
	}
}
#endif

#if(ENC_MODE==_ENC_MODE_AES_)
void KeyExpansion(unsigned char* key, unsigned char w[][4][4])
{
	int i,j,r,c;
	unsigned char rc[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};
	for(r=0; r<4; r++)
	{
		for(c=0; c<4; c++)
		{
			w[0][r][c] = key[r+c*4];
		}
	}
	for(i=1; i<=10; i++)
	{
		for(j=0; j<4; j++)
		{
			unsigned char t[4];
			for(r=0; r<4; r++)
			{
				t[r] = j ? w[i][r][j-1] : w[i-1][r][3];
			}
			if(j == 0)
			{
				unsigned char temp = t[0];
				for(r=0; r<3; r++)
				{
					t[r] = sBox[t[(r+1)%4]];
				}
				t[3] = sBox[temp];
				t[0] ^= rc[i-1];
			}
			for(r=0; r<4; r++)
			{
				w[i][r][j] = w[i-1][r][j] ^ t[r];
			}
		}
	}
}

unsigned char FFmul(unsigned char a, unsigned char b)
{
	unsigned char bw[4];
	unsigned char res=0;
	int i;
	bw[0] = b;
	for(i=1; i<4; i++)
	{
		bw[i] = bw[i-1]<<1;
		if(bw[i-1]&0x80)
		{
			bw[i]^=0x1b;
		}
	}
	for(i=0; i<4; i++)
	{
		if((a>>i)&0x01)
		{
			res ^= bw[i];
		}
	}
	return res;
}

void SubBytes(unsigned char state[][4])
{
	int r,c;
	for(r=0; r<4; r++)
	{
		for(c=0; c<4; c++)
		{
			state[r][c] = sBox[state[r][c]];
		}
	}
}

void ShiftRows(unsigned char state[][4])
{
	unsigned char t[4];
	int r,c;
	for(r=1; r<4; r++)
	{
		for(c=0; c<4; c++)
		{
			t[c] = state[r][(c+r)%4];
		}
		for(c=0; c<4; c++)
		{
			state[r][c] = t[c];
		}
	}
}

void MixColumns(unsigned char state[][4])
{
	unsigned char t[4];
	int r,c;
	for(c=0; c< 4; c++)
	{
		for(r=0; r<4; r++)
		{
			t[r] = state[r][c];
		}
		for(r=0; r<4; r++)
		{
			state[r][c] = FFmul(0x02, t[r])
				^ FFmul(0x03, t[(r+1)%4])
				^ FFmul(0x01, t[(r+2)%4])
				^ FFmul(0x01, t[(r+3)%4]);
		}
	}
}

void AddRoundKey(unsigned char state[][4], unsigned char k[][4])
{
	int r,c;
	for(c=0; c<4; c++)
	{
		for(r=0; r<4; r++)
		{
			state[r][c] ^= k[r][c];
		}
	}
}

void InvSubBytes(unsigned char state[][4])
{
	int r,c;
	for(r=0; r<4; r++)
	{
		for(c=0; c<4; c++)
		{
			state[r][c] = invsBox[state[r][c]];
		}
	}
}

void InvShiftRows(unsigned char state[][4])
{
	unsigned char t[4];
	int r,c;
	for(r=1; r<4; r++)
	{
		for(c=0; c<4; c++)
		{
			t[c] = state[r][(c-r+4)%4];
		}
		for(c=0; c<4; c++)
		{
			state[r][c] = t[c];
		}
	}
}

void InvMixColumns(unsigned char state[][4])
{
	unsigned char t[4];
	int r,c;
	for(c=0; c< 4; c++)
	{
		for(r=0; r<4; r++)
		{
			t[r] = state[r][c];
		}
		for(r=0; r<4; r++)
		{
			state[r][c] = FFmul(0x0e, t[r])
				^ FFmul(0x0b, t[(r+1)%4])
				^ FFmul(0x0d, t[(r+2)%4])
				^ FFmul(0x09, t[(r+3)%4]);
		}
	}
}


void Aes_Encrypt(unsigned char* input)
{
	unsigned char state[4][4];
	int i,r,c;

	for(r=0; r<4; r++)
	{
		for(c=0; c<4 ;c++)
		{
			state[r][c] = input[c*4+r];
		}
	}

	AddRoundKey(state,w[0]);

	for(i=1; i<=10; i++)
	{
		SubBytes(state);
		ShiftRows(state);
		if(i!=10)MixColumns(state);
		AddRoundKey(state,w[i]);
	}

	for(r=0; r<4; r++)
	{
		for(c=0; c<4 ;c++)
		{
			input[c*4+r] = state[r][c];
		}
	}

	//return input;
}

void Aes_Decrypt(unsigned char* input)
{
	unsigned char state[4][4];
	int i,r,c;

	for(r=0; r<4; r++)
	{
		for(c=0; c<4 ;c++)
		{
			state[r][c] = input[c*4+r];
		}
	}

	AddRoundKey(state, w[10]);
	for(i=9; i>=0; i--)
	{
		InvShiftRows(state);
		InvSubBytes(state);
		AddRoundKey(state, w[i]);
		if(i)
		{
			InvMixColumns(state);
		}
	}

	for(r=0; r<4; r++)
	{
		for(c=0; c<4 ;c++)
		{
			input[c*4+r] = state[r][c];
		}
	}

//	return input;
}


void Boot_Encrypt(char *plainBlock, char *keyStr,char *cipherBlock, u32 size)
{
	u32 i=0;
	KeyExpansion(keyStr, w);
	for(i=0;i<size;i+=16)
	{
		//DES_Encrypt_Section(plainBlock+i,keyStr,cipherBlock+i);
		//Aes_Encrypt(plainBlock+i,keyStr,cipherBlock+i);
		Aes_Encrypt(plainBlock+i);
	}
}

void Boot_Decrypt(char *cipherBlock, char *keyStr,char *plainBlock,u32 size)
{
	u32 i=0;
	KeyExpansion(keyStr, w);
	
	for(i=0;i<size;i+=16)
	{
		//DES_Decrypt_Section(plainBlock+i,keyStr,cipherBlock+i);
		//Aes_Decrypt(cipherBlock+i,keyStr,plainBlock+i);
		Aes_Decrypt(cipherBlock+i);
	}
}

#endif


#if _TRI_DES_FILE_
/*加密文件*/
int DES_Encrypt(char *plainFile, char *keyStr,char *cipherFile)
{
	FILE *plain,*cipher;
	int count;
	ElemType plainBlock[8],cipherBlock[8],keyBlock[8];
	ElemType bKey[64];
	ElemType subKeys[16][48];
	if((plain = fopen(plainFile,"rb")) == NULL){
		return PLAIN_FILE_OPEN_ERROR;
	}

	int size = filelength(fileno(plain));
	int prog = 0;

//	int size = plain.GetLength();

	if((cipher = fopen(cipherFile,"wb")) == NULL){
		return CIPHER_FILE_OPEN_ERROR;
	}
	/*设置密钥*/
	memcpy(keyBlock,keyStr,8);
	/*将密钥转换为二进制流*/
	Char8ToBit64(keyBlock,bKey);
	/*生成子密钥*/
	DES_MakeSubKeys(bKey,subKeys);
	
	while(!feof(plain)){
		/*每次读8个字节，并返回成功读取的字节数*/
		if((count = fread(plainBlock,sizeof(char),8,plain)) == 8){
			DES_EncryptBlock(plainBlock,subKeys,cipherBlock);
			fwrite(cipherBlock,sizeof(char),8,cipher);  
		}
	}
	if(count){
		/*填充*/
		memset(plainBlock + count,'\0',7 - count);
		/*最后一个字符保存包括最后一个字符在内的所填充的字符数量*/
		plainBlock[7] = 8 - count;
		DES_EncryptBlock(plainBlock,subKeys,cipherBlock);
		fwrite(cipherBlock,sizeof(char),8,cipher);
	}
	
	//g_progressValue = (++prog)/size;

	fclose(plain);
	fclose(cipher);
	return OK;
}
  
/*解密文件*/
int TriDesEnc::DES_Decrypt(char *cipherFile, char *keyStr,char *plainFile)
{
	FILE *plain, *cipher;
	int count,times = 0;
	long fileLen;
	ElemType plainBlock[8],cipherBlock[8],keyBlock[8];
	ElemType bKey[64];
	ElemType subKeys[16][48];
	if((cipher = fopen(cipherFile,"rb")) == NULL){
		return CIPHER_FILE_OPEN_ERROR;
	}
	if((plain = fopen(plainFile,"wb")) == NULL){
		return PLAIN_FILE_OPEN_ERROR;
	}
	
	/*设置密钥*/
	memcpy(keyBlock,keyStr,8);
	/*将密钥转换为二进制流*/
	Char8ToBit64(keyBlock,bKey);
	/*生成子密钥*/
	DES_MakeSubKeys(bKey,subKeys);
	
	/*取文件长度 */
	fseek(cipher,0,SEEK_END);/*将文件指针置尾*/
	fileLen = ftell(cipher); /*取文件指针当前位置*/
	rewind(cipher); /*将文件指针重指向文件头*/
	while(1){
		/*密文的字节数一定是8的整数倍*/
		fread(cipherBlock,sizeof(char),8,cipher);
		DES_DecryptBlock(cipherBlock,subKeys,plainBlock);  
		times += 8;
		if(times < fileLen){
			fwrite(plainBlock,sizeof(char),8,plain);
		}
		else{
			break;
		}
	}
	/*判断末尾是否被填充*/
	if(plainBlock[7] < 8){
		for(count = 8 - plainBlock[7]; count < 7; count++){
			if(plainBlock[count] != '\0'){
				break;
			}
		}
	}
	if(count == 7){/*有填充*/
		fwrite(plainBlock,sizeof(char),8 - plainBlock[7],plain);
	}
	else{/*无填充*/
		fwrite(plainBlock,sizeof(char),8,plain);
	}
	
	fclose(plain);
	fclose(cipher);
	return OK;
} 
#endif

/////////////////////////////////////////////////////////////////////////////
// TriDesEnc diagnostic

/////////////////////////////////////////////////////////////////////////////
// TriDesEnc message handlers
