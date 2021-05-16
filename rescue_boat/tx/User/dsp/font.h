#ifndef __FONT_LL_H_
#define __FONT_LL_H_


extern const unsigned char icon_h[24];
extern const unsigned char icon_signal[32];
extern const unsigned char font_num16[11][16];
extern const unsigned char font_num8[15][6];
extern const unsigned char bat_full[18*2];
extern const unsigned char bat_80[18*2];
extern const unsigned char bat_50[18*2];
extern const unsigned char bat_20[18*2];
extern const unsigned char bat_low[18*2];
extern const unsigned char vertical_bar[24];
extern const unsigned char horizontal_bar[48];
extern const unsigned char font_char16[][16];
extern const unsigned char font_char16_B[26][16];
extern const unsigned char font_char16_L[26][16];
extern const unsigned char font_sm_index[16];
extern const unsigned char font_revert[32];
extern const char picScan_1[72];
extern const char picScan_2[72];
extern const char picScan_3[72];
extern const char picScan[3][72];
extern const char icon_l[24];
extern const char icon_m[24];
extern const char selLeft[40];
extern const char selRight[40];
extern const char icon_signal_err[32];
extern const char font_char32[][64];
extern const char testBitmap_100x48[600];
extern const char langSelTag[32];
extern const char font_bat_balance_err[20];
extern const char isLockIcon[110];
extern const unsigned char icon_lock[32];
extern const char icon_s[24];
extern const unsigned char icon_signal_1[32];
extern const unsigned char icon_signal_2[32];
extern const unsigned char icon_signal_3[32];
extern const unsigned char icon_signal_0[32];


typedef struct CHINEST_CHAR{
	const char font[32];
	const char* str;
}ChineseChar;

#define CN_CHAR_NUM 70
extern const ChineseChar font_chinese[CN_CHAR_NUM];



#endif

