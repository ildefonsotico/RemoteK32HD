/*******************************************************************************
'*  EMPRESA    : DATASINC                                                      *
'*  DESCRIÇÃO  : Configuração do hardware                                      *
'*  PRODUTO    : DS-K32HD - Comutador da matriz HD (DS-M32HD e DS-M32As)       *
'*  PROJETISTAS: Ildefonso Ferreira e Vinicius Arruda                          *
'*  ARQUIVO    : Comm_v00     ESQUEMA: DS-K32HD.PrjPCb                         *
'*  COMPILADOR : Microchip C30                                                 *
'*  OBSERVAÇÃO : # Microcontrolador = PIC24HJ128GP206-I/PT                     *
'*             : Todos os direitos reservados                                  *
'==============================================================================*
'*                            Revision history                                 *
'*-----------------------------------------------------------------------------*
'*   rev    :   date    :     resp     :             description               *
'*-----------------------------------------------------------------------------*
'*   0.0    :jul/17/2012: Ildefonso F. : Initial issue                         *
'*          :           :              :                                       *
'*          :           :              :                                       *
'******************************************************************************/
#ifndef __HARDWARE_PROFILE_H
#define __HARDWARE_PROFILE_H

#define DSK32HD

// Set configuration fuses (but only once)
#if defined(THIS_IS_STACK_APPLICATION)

//_FOSCSEL(FNOSC_PRIPLL)         // PLL enabled  microchip
_FOSCSEL(FNOSC_PRI & IESO_OFF)      // mcpu
//_FOSC(OSCIOFNC_OFF & POSCMD_XT)   // XT Osc microchip
_FOSC(FCKSM_CSDCMD&OSCIOFNC_ON&POSCMD_HS)   // Clock switch disabled, OSC2 is digital I/O, HS oscillator mcpu
_FWDT(FWDTEN_OFF)            // Disable Watchdog timer
// JTAG should be disabled as well

#endif // Prevent more than one set of config fuse definitions

#define GetSystemClock()         (25000000ul)      // Hz
#define GetInstructionClock()    (GetSystemClock()/2)
#define GetPeripheralClock()     GetInstructionClock()


#if defined(DSK32HD)


/*----------------------------- Pinos de I/O ---------------------------------*/

// Definição dos botões

#define Col_Sw1      _LATD6
#define Col_Sw2      _LATG3
#define Col_Sw3      _LATD5
#define Col_Sw4      _LATD1
#define Col_Sw5      _LATB4
#define Col_Sw6      _LATG13
#define Col_Sw8      _LATG1
#define Col_Sw7      _LATG0


#define Row_Sw1_TRIS    (TRISBbits.TRISB5)
#define Row_Sw1         (PORTBbits.RB5)
#define Row_Sw2_TRIS    (TRISGbits.TRISG15)
#define Row_Sw2         (PORTGbits.RG15)
#define Row_Sw3_TRIS    (TRISBbits.TRISB13)
#define Row_Sw3         (PORTBbits.RB13)
#define Row_Sw4_TRIS    (TRISFbits.TRISF3)
#define Row_Sw4         (PORTFbits.RF3)

// Definição para os LED's
// LED's Gerais
#define Col_Led1     _LATD2
#define Col_Led2     _LATF4
#define Col_Led3     _LATB1
#define Col_Led4     _LATD0
#define Col_Led5     _LATB9
#define Col_Led6     _LATG12
#define Col_Led7     _LATG14
#define Col_Led8     _LATF0

#define Row_Vi1      _LATF5      // Os LED's vermelhos sinalizam o vídeo
#define Row_Vi2      _LATB0
#define Row_Vi3      _LATG2
#define Row_Vi4      _LATB8

//#define Row_Au1_TRIS (TRISFbits.TRISF2)
#define Row_Au1      _LATD7   //_LATF2      // Os LED's verdes sinalizam áudio
#define Row_Au2      _LATB14
#define Row_Au3      _LATB10
#define Row_Au4      _LATD4

// Definição do botão Audio/Video
#define AuVi_TRIS       (TRISDbits.TRISD8)
#define AuVi            (PORTDbits.RD8)
#define LEDVi           _LATB2
#define LEDAu           _LATB3

// Definição do botão Enable
#define EnableINT_TRIS  (TRISDbits.TRISD9)
#define EnableINT       (PORTDbits.RD9)
#define LEDEn           _LATB12
#define LEDDis          _LATD3

// Definição do botão Take
#define Take_TRIS       (TRISFbits.TRISF6)
#define Take            (PORTFbits.RF6)
#define LEDTakeRed      _LATB15
#define LEDTakeGreen    _LATB11

// Definição de pinos para comunicação
#define CSenc1          _LATC1
#define CSenc2          _LATC2

// Definição do botão Config
#define Config_TRIS     (TRISBbits.TRISB7)
#define Config          (PORTBbits.RB7)
#define LEDConfRed      _LATB6
#define LEDConfGreen    _LATF1

// ENC28J60 I/O pins
//#define ENC_RST_TRIS      (TRISDbits.TRISD15)   // Not connected by default
//#define ENC_RST_IO        (PORTDbits.RD15)
#define ENC_CS_TRIS       (TRISCbits.TRISC2)
#define ENC_CS_IO         (PORTCbits.RC2)


#define ENC_SPI_IF        (IFS2bits.SPI2IF)
#define ENC_SSPBUF        (SPI2BUF)
#define ENC_SPISTAT       (SPI2STAT)
#define ENC_SPISTATbits   (SPI2STATbits)
#define ENC_SPICON1       (SPI2CON1)
#define ENC_SPICON1bits   (SPI2CON1bits)
#define ENC_SPICON2       (SPI2CON2)


#endif

#endif
