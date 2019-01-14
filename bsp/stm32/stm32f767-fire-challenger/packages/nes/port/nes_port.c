#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "K6502.h"
#include "InfoNES.h"
#include "InfoNES_System.h"
#include "InfoNES_Mapper.h"
#include "InfoNES_pAPU.h"
#include "stm32746g_discovery.h"
#include "nes_game.h"

#define LCD_AER(x)         *(unsigned int *)(0xC0000000 + x)
void     BSP_LCD_DrawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t pixel);
#define SELECT_GAME        0            /* 选择要运行的游戏  */

// Palette data
WORD NesPalette[ 64 ] =
{
    0x39ce, 0x1071, 0x0015, 0x2013, 0x440e, 0x5402, 0x5000, 0x3c20,
    0x20a0, 0x0100, 0x0140, 0x00e2, 0x0ceb, 0x0000, 0x0000, 0x0000,
    0x5ef7, 0x01dd, 0x10fd, 0x401e, 0x5c17, 0x700b, 0x6ca0, 0x6521,
    0x45c0, 0x0240, 0x02a0, 0x0247, 0x0211, 0x0000, 0x0000, 0x0000,
    0x7fff, 0x1eff, 0x2e5f, 0x223f, 0x79ff, 0x7dd6, 0x7dcc, 0x7e67,
    0x7ae7, 0x4342, 0x2769, 0x2ff3, 0x03bb, 0x0000, 0x0000, 0x0000,
    0x7fff, 0x579f, 0x635f, 0x6b3f, 0x7f1f, 0x7f1b, 0x7ef6, 0x7f75,
    0x7f94, 0x73f4, 0x57d7, 0x5bf9, 0x4ffe, 0x0000, 0x0000, 0x0000
    
};


void nesStart(void)
{
    WorkFrame = (WORD *)0xc0260000;
    
    if(0 != InfoNES_Load("SuperMario")) {

        return;
    }
    
    InfoNES_Main();
}



/*===================================================================*/
/*                                                                   */
/*                  InfoNES_Menu() : Menu screen                     */
/*                                                                   */
/*===================================================================*/
int InfoNES_Menu()
{
/*
 *  Menu screen
 *
 *  Return values
 *     0 : Normally
 *    -1 : Exit InfoNES
 */

  if ( PAD_PUSH( PAD_System, PAD_SYS_QUIT) )
    return -1;	 	

  // Nothing to do here
  return 0;
}


/*===================================================================*/
/*                                                                   */
/*               InfoNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*===================================================================*/
int InfoNES_ReadRom( const char *pszFileName )
{
/*
 *  Read ROM image file
 *
 *  Parameters
 *    const char *pszFileName          (Read)
 *
 *  Return values
 *     0 : Normally
 *    -1 : Error
 */

//  FILE *fp;

  /* Open ROM file */
//  fp = fopen( pszFileName, "rb" );
//  if ( fp == NULL )
//    return -1;

  /* Read ROM Header */
  nesReadFile(&NesHeader, sizeof NesHeader, 1, &nes_game[SELECT_GAME]);
  
//  fread( &NesHeader, sizeof NesHeader, 1, fp );
  if ( memcmp( NesHeader.byID, "NES\x1a", 4 ) != 0 )
  {
    /* not .nes file */
//    fclose( fp );
    return -1;
  }

  /* Clear SRAM */
  memset( SRAM, 0, SRAM_SIZE );

  /* If trainer presents Read Triner at 0x7000-0x71ff */
  if ( NesHeader.byInfo1 & 4 )
  {
//    fread( &SRAM[ 0x1000 ], 512, 1, fp );
      nesReadFile(&SRAM[ 0x1000 ], 512, 1, &nes_game[SELECT_GAME]);
  }

  /* Allocate Memory for ROM Image */
//  ROM = (BYTE *)malloc( NesHeader.byRomSize * 0x4000 );

  /* Read ROM Image */
//  fread( ROM, 0x4000, NesHeader.byRomSize, fp );
  nesReadFile(ROM, 0x4000, NesHeader.byRomSize, &nes_game[SELECT_GAME]);
  
  if ( NesHeader.byVRomSize > 0 )
  {
    /* Allocate Memory for VROM Image */
    VROM = (BYTE *)malloc( NesHeader.byVRomSize * 0x2000 );

    /* Read VROM Image */
//    fread( VROM, 0x2000, NesHeader.byVRomSize, fp );
    nesReadFile(VROM, 0x2000, NesHeader.byVRomSize, &nes_game[SELECT_GAME]);
  }

//  /* File close */
//  fclose( fp );

  /* Successful */
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*           InfoNES_ReleaseRom() : Release a memory for ROM         */
/*                                                                   */
/*===================================================================*/
void InfoNES_ReleaseRom()
{
/*
 *  Release a memory for ROM
 *
 */

  if ( ROM )
  {
//    free( ROM );
//    ROM = NULL;
  }

  if ( VROM )
  {
    free( VROM );
    VROM = NULL;
  }
}

/*===================================================================*/
/*                                                                   */
/*      InfoNES_LoadFrame() :                                        */
/*           Transfer the contents of work frame on the screen       */
/*                                                                   */
/*===================================================================*/
void InfoNES_LoadFrame()
{
/*
 *  Transfer the contents of work frame on the screen
 *
 */
    int x, y, lcd_x = 0, lcd_y = 0;
    unsigned int r, g, b;
    WORD wColor;
    unsigned int LCD_Color888;

    /* Exchange 16-bit to 24-bit  RGB555 to RGB888*/
    lcd_y = 32;
    lcd_x = 239;
    
    for ( y = 0; y < NES_DISP_HEIGHT; y++ ) {
        for ( x = 0; x < NES_DISP_WIDTH; x++ ) {
            wColor = WorkFrame[ ( y << 8 ) + x ];
            r = (wColor & 0x7c00) << 9;
            g = (wColor & 0x03e0) << 6;
            b = (wColor & 0x001f) << 3;
            
//            r = (wColor & 0xf800) << 8;
//            g = (wColor & 0x07e0) << 5;
//            b = (wColor & 0x001f) << 3;
            
            LCD_Color888 = 0xff000000 | r | g | b;
            BSP_LCD_DrawPixel(lcd_x,lcd_y-25,LCD_Color888);
//            LCD_AER((lcd_y * 240 + lcd_x) * 4) = LCD_Color888;
            
            lcd_y ++;
            if (lcd_y == 288) {
                lcd_x --;
                lcd_y = 32;
            }
        }
    }
}


uint32_t BSP_PB_GetState(Button_TypeDef);
char NES_AcquireTouchButtons(void);

#define NONE 0
#define RIGHT 1
#define LEFT 2
#define UP 3
#define DOWN 4
/*===================================================================*/
/*                                                                   */
/*             InfoNES_PadState() : Get a joypad state               */
/*                                                                   */
/*===================================================================*/
void InfoNES_PadState( DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem )
{
    char state=NES_AcquireTouchButtons();
		*pdwPad1 = 0;
		switch(state)
		{
			case RIGHT:
				*pdwPad1 |= (1<<7);break;
			case LEFT:
				*pdwPad1 |= (1<<6);break;
			case UP:
				*pdwPad1 |= (1<<0);break;
			case DOWN:
				*pdwPad1 |= (1<<1);break;
		}
    
    if (BSP_PB_GetState(BUTTON_KEY)) {
        *pdwPad1 |= (1 << 3); // Key_S
    }

}



/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemoryCopy() : memcpy                         */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemoryCopy( void *dest, const void *src, int count )
{
/*
 *  memcpy
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the copied block's destination
 *
 *    const void *src                  (Read)
 *      Points to the starting address of the block of memory to copy
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to copy
 *
 *  Return values
 *    Pointer of destination
 */

  memcpy( dest, src, count );
  return dest;
}


/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemorySet() : memset                          */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemorySet( void *dest, int c, int count )
{
/*
 *  memset
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the block of memory to fill
 *
 *    int c                            (Read)
 *      Specifies the byte value with which to fill the memory block
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to fill
 *
 *  Return values
 *    Pointer of destination
 */

  memset( dest, c, count);  
  return dest;
}


/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundInit() : Sound Emulation Initialize           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundInit( void ) 
{
//  sound_fd = 0;
}



/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*===================================================================*/
int InfoNES_SoundOpen( int samples_per_sync, int sample_rate ) 
{
 
  /* Successful */
  return 1;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundClose( void ) 
{
//  if ( sound_fd ) 
//  {
//    close(sound_fd);
//  }
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_SoundOutput() : Sound Output 5 Waves           */           
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundOutput( int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5 )
{

}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_Wait() : Wait Emulation if required            */
/*                                                                   */
/*===================================================================*/
void InfoNES_Wait() {}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_MessageBox() : Print System Message            */
/*                                                                   */
/*===================================================================*/
void InfoNES_MessageBox( char *pszMsg, ... )
{
//    va_list args;
//    va_start( args, pszMsg );
//    printf( pszMsg, args );	
//    va_end( args );
}


