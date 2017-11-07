/*******************************************************************************
'*  EMPRESA    : DATASINC                                                      *
'*  DESCRIÇÃO  : Definições das variáveis globais                              *
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


/*------------------- Definições das Variáveis Globais -----------------------*/
//TCP-IP

IP_ADDR IP_MATRIZ_VIDEO;
IP_ADDR IP_MATRIZ_AUDIO;

//MACROS PARA PINOS DAS TECLAS:
#define TECLA_AV 		_RD8
#define TECLA_TAKE 		_RF6
#define TECLA_ENABLE	_RD9
#define TECLA_CONFIG	_RB7
#define TECLA_ROW1 		_RB5
#define TECLA_ROW2 		_RG15
#define TECLA_ROW3 		_RB13
#define TECLA_ROW4 		_RF3
#define TECLA_COL1 		_LATD6
#define TECLA_COL2 		_LATG3
#define TECLA_COL3 		_LATD5
#define TECLA_COL4 		_LATD1
#define TECLA_COL5 		_LATB4
#define TECLA_COL6 		_LATG13
#define TECLA_COL7 		_LATG0
#define TECLA_COL8 		_LATG1

//MACROS PARA PINOS LEDS:
#define LEDGREEN_AV 		_LATB3
#define LEDRED_AV 	_LATB2
#define LEDGREEN_TAKE 	_LATB11
#define LEDRED_TAKE	_LATB15
#define LEDGREEN_ENABLE 	_LATB12
#define LEDRED_ENABLE	_LATD3
#define LEDGREEN_CONFIG 	_LATF1
#define LEDRED_CONFIG	_LATB6
#define LEDGREEN_ROW1 	_LATD7
#define LEDRED_ROW1 	_LATF5
#define LEDGREEN_ROW2 	_LATB14
#define LEDRED_ROW2 	_LATB0
#define LEDGREEN_ROW3 	_LATB10
#define LEDRED_ROW3 	_LATG2
#define LEDGREEN_ROW4 	_LATD4
#define LEDRED_ROW4 	_LATB8
#define LED_COL1	 	_LATD2
#define LED_COL2	 	_LATF4
#define LED_COL3	 	_LATB1
#define LED_COL4	 	_LATD0
#define LED_COL5	 	_LATB9
#define LED_COL6	 	_LATG12
#define LED_COL7	 	_LATG14
#define LED_COL8	 	_LATF0

//ESTADOS ENUM:
enum STATUS_ENUM{AUDIO=0,VIDEO,AUDIO_VIDEO};
enum COR_ENUM{RED=0,GREEN,YELLOW};
enum TECLAS_ENUM{AV=32,TAKE,ENABLE,CONFIG};

//VARIÁVEIS GLOBAIS:
int STATUS_AV = 0;	
int SAIDA = -1;//Representa a saída que o remote atua
int SAIDA_TEMP=-1;
BOOL STATUS_CONFIG;
BOOL STATUS_TAKE;
BOOL STATUS_ENABLE;
#define DELAY_TECLA 60
long long CONTA_DELAY_TECLA;
#define DELAY_ENABLE 2500
long long CONTA_DELAY_ENABLE;
int BORDA_AV,BORDA_TAKE,BORDA_ENABLE,BORDA_CONFIG,BORDA_ENTRADAS;
int ESTADO_AV,ESTADO_CONFIG,ESTADO_TAKE,ESTADO_ENABLE;
#define PING_TIME_OUT 10000
long long CONTA_PING_TIME_OUT;
BOOL PING_AUDIO, PING_VIDEO;
long long contador_brilho_aux;
long long Contador_Time_Out_Comutacao = 0;
BOOL contsai=1;            //Contador de saídas
BOOL flagAu=0;             //Sinaliza que existe comando de atualização de áudio proveniente da MCPU Vídeo
BOOL flagVi=0;             //Sinaliza que existe comando de atualização de vídeo proveniente da MCPU Vídeo
BOOL flagupdate=0;         //S
BOOL flagupdateAu=0;           //Sinaliza que está no loop de inicialização com a MCPU Vídeo    
BOOL flagSend=0;           //Sinaliza que existe comando de escrita a ser enviado a MCPU Vídeo
BOOL flagSendAu=0;
BOOL flagEnable_Permanente = 0; //Flag para sinalizar que o foi pressionado o enable por mais de 3 segundos e que ele ficará ativo permanentemente
unsigned char ent[2], saida[2];     //ent - entrada controlada pelo remote
                                    //saida - saida controlada pelo remote 
unsigned char entAu[2], entVi[2];   //entAu - entrada de áudio controlada pelo remote
                                    //entVi - entrade de vídeo controlada pelo remote                                    
unsigned char t[9];        //Armazena string a ser enviada
unsigned char s[11];       //Armazena string recebida
unsigned char a,f;       //Variáveis auxiliares
long long i,j,k;
int Tecla_Apertada_Anterior = -1;
int Tecla_Apertada = -1;
int Entrada_Audio_Temp = -1;
int Entrada_Video = -1;//Armazena a entrada de video configurada
int Entrada_Video_Take = 0;//Armazena entrada de vídeo configurada antes de pressionar a tecla take
int Entrada_Video_Temp = 0;
int Entrada_Audio = -1;//Armazena a entrada de audio configurada
int Entrada_Audio_Take = -1;//Armazena entrada de audio configurada antes de pressionar a tecla take
int Entrada_Audio_Take_Temp = -1;
int Entrada_Video_Take_Temp = -1;
int Saida_Take = -1;
int Entrada_Apertada = 0;
int Entrada_Apertada_Anterior=0;
long long tecla_apertada_aux=0;
unsigned char conta=0;     //Contador de comandos das funções: SendETH, SendETHAu, TakeETHVi e TakeETHAu
unsigned char contup=0;    //Contador de comandos das funçãos: Update e UpdateAu

/*-----------------Definições utilizadas na comunicação Ethernet---------------*/

// Defines the server to be accessed for this application

#define USE_REMOTE_TCP_SERVER       "192.168.100.50"
#define USE_REMOTE_TCP_SERVER_AU    "192.168.100.51"


// Defines the port to be accessed for this application
#define ServerPort      3000
#define ServerPort_AU   4000

/*----------------------------------------------------------------------------*/


