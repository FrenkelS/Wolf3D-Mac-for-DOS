typedef unsigned int Word;
typedef unsigned long LongWord;
#ifndef __MACTYPES__
typedef unsigned char Byte;
typedef unsigned char Boolean;
#define FALSE 0
#define TRUE  1
#endif

#define BLACK 255
#define DAMAGECOLOR 241

//#define __MAC__
#if BYTE_ORDER == LITTLE_ENDIAN

#elif BYTE_ORDER == BIG_ENDIAN
#define __BIGENDIAN__
#define SHORT(x)	(x)
#else
#error unknown byte order
#endif

#define SfxActive 1
#define MusicActive 2

#define VideoSize 64000
#define SetAuxType(x,y)
#define SetFileType(x,y)

extern Word KeyModifiers;
extern Word ScanCode;
extern Word KilledSong;
extern Word SystemState;
extern LongWord YTable[480];

void DLZSS(Byte *Dest, Byte *Src,LongWord Length);
void DLZB(Byte *Dest, Byte *Src,LongWord Length);
LongWord SwapLong(LongWord Val);
unsigned short SwapUShort(unsigned short Val);
short SwapShort(short Val);

void WaitTick(void);
void WaitTicks(Word TickCount);
Word WaitTicksEvent(Word TickCount);
Word WaitEvent(void);
LongWord ReadTick(void);

Word AllKeysUp(void);
Word WaitKey(void);
void FlushKeys(void);

void SoundOff(void);
void PlaySound(Word SndNum);
void StopSound(Word SndNum);
void PlaySong(Word SongNum);

void ClearTheScreen(void);
void ShowPic(Word PicNum);

void InitYTable(void);
void InstallAFont(Word FontNum);
void FontUseMask(void);
void FontUseZero(void);
void SetFontXY(Word x,Word y);
void FontSetColor(Word Index,Word Color);
void DrawAString(char *TextPtr);
void DrawAChar(Word Letter);
Word GetRandom(Word Range);
void Randomize(void);
void DrawRawFullScreen(Word RezNum);
void DrawShapeNum(Word x,Word y,Word RezNum);
void DrawXMShapeNum(Word x,Word y,Word RezNum);
void EraseMBShape(Word x,Word y, void *ShapePtr,void *BackPtr);
Word TestMShape(Word x,Word y,void *ShapePtr);
Word TestMBShape(Word x,Word y,void *ShapePtr,void *BackPtr);

void SetAPalette(Word PalNum);
void FadeTo(Word PalNum);
void FadeToBlack(void);

void KillAResource2(Word RezNum,LongWord Type);
void SaveJunk(void *AckPtr,Word Length);
