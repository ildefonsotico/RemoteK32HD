/*******************************************************************************
'*  EMPRESA    : DATASINC                                                      *
'*  DESCRIÇÃO  : Função Principal                                              *
'*  PRODUTO    : DS-K32HD - Comutador da matriz HD (DS-M32HD e DS-M32As)       *
'*  PROJETISTAS: Ildefonso Ferreira e Vinicius Arruda                          *
'*  ARQUIVO    : K32HD_v00             ESQUEMA: DS-K32HD.PrjPCb                *
'*  COMPILADOR : Microchip C30                                                 *
'*  OBSERVAÇÃO : # Microcontrolador = PIC24HJ128GP206-I/PT                     *
'*             : Todos os direitos reservados                                  *
'==============================================================================*
'*                            Revision history                                 *
'*-----------------------------------------------------------------------------*
'*   rev    :   date    :     resp     :             description               *
'*-----------------------------------------------------------------------------*
'*   0.0    :jul/77/2012: Ildefonso F. : Initial issue                         *
'*-----------------------------------------------------------------------------*

'******************************************************************************/
 
/*******************************************************************************
*       ***Informações sobre o algoritmo e funcionamento do Remote***          *
*                                                                              *
*    .O fluxograma descrevendo os estados e comportamentos do equipamento      *
* estão no arquivo: "Protocolo DS-MCPU.xls" (K:\Projetos\DS-MCPU\Protocolo)             *
*                                                                              *
*                                                                              *
 ******************************************************************************/

/*******************************************************************************
*  ***Informações sobre a pilha TCPIP da Microchip - Microchip TCPIP Stack***  *
*                                                                              *
*   .Versão 4.55                                                               *
*   .Application Note: AN833 - Microchip TCP/IP Stack Application Note         *
*                      (de 21 de agosto de 2008)                               *
*                                                                              *
*   .Material extraído do site: www.microchip.com/enc28j60                     *
*                                                                              *
*                                                                              *
 ******************************************************************************/
 
/*
 * This macro uniquely defines this file as the main entry point.
 * There should only be one such definition in the entire project,
 * and this file must define the AppConfig variable as described below.
 */
#define THIS_IS_STACK_APPLICATION


// Include all headers for any enabled TCPIP Stack functions
#include "Include/TCPIP.h"

// Include functions specific to this stack application
#include "Include/MainDemo.h"

// Declare AppConfig structure and some other supporting stack variables
APP_CONFIG AppConfig;
BYTE AN0String[8];
BYTE myDHCPBindCount = 0xFF;
#if !defined(STACK_USE_DHCP_CLIENT)
   #define DHCPBindCount   (1)
#endif


#include "Include/Definicoes_v01.c"            	//Definições das variáveis globais 
//#include "Util_v00.c"                  		//Funções utilizadas por todo o projeto
#include "Include/Comm_v00.c"                   //Funções de comunicação
//#include "Interrupcoes_v01.c"          		//Funções de Interrupções


/*-------------------------- VARIÁVEIS GLOBAIS ------------------------------*/
long long contador_brilho=0;
unsigned long int contador_time_atualizacao=0; 
int entrada_update_aud = -1;
int entrada_update_vid = -1;
unsigned char Enable_Ativo = 0;	
/*----------------------------------------------------------------------------*/

/*------------------------------ Protótipos ----------------------------------*/
static void InitializeBoard(void);     	// Inicialização do hardware
static void InitAppConfig(void);    	// Inicialização TCP Stack
void AcendeLedSequencial(void);			// Acende ou pisca leds em sincronia exclusiva
void MaquinaTeclas(void);				// Máquina de estados das teclas
void DelayNop(); 						// Delay com 10 ciclos de máquina
void DelayMili(long mili); 				// Delay em milisegundos
void DelayNano(long nano); 				// Delay em nanosegundos
int TestaTeclas();						// Verifica qual tecla foi pressionada
void AcendeLed(int x,int COR);			// Acende determinado Led com Cor específica
void ApagaLed(int x,int COR);			// Apaga determinado Led com Cor específica
void ApagaTodosLeds(); 					// Apaga todos os Leds das Entradas (0 a 31)
void TestaLeds();						// Função que verifica os Leds na inicialização
BOOL TestaAudioVideo();					// Função que testa a conexão com Audio e\ou Video
void ProcessaEntradas(void);			// Função que processa a atualização das Entradas
/*----------------------------------------------------------------------------*/

// C30 and C32 Exception Handlers
// If your code gets here, you either tried to read or write
// a NULL pointer, or your application overflowed the stack
// by having too many local variables or parameters declared.
#if defined(__C30__)
   void _ISR __attribute__((__no_auto_psv__)) _AddressError(void)
   {
       Nop();
      Nop();
   }
   void _ISR __attribute__((__no_auto_psv__)) _StackError(void)
   {
       Nop();
      Nop();
   }
#endif


/*******************************************************************************
  NOME: main
  DESCRIÇÃO: Função Principal
  PARÂMETROS: nenhum
  RETORNO: nenhum
  NOTAS: nenhum
*******************************************************************************/
int main(void)
{   
   // Initialize application specific hardware
   InitializeBoard();
   TickInit();
   // Initialize Stack and application related NV variables into AppConfig.
   InitAppConfig();
   // Initialize core stack layers (MAC, ARP, TCP, UDP) and
   // application modules (HTTP, SNMP, etc.)
   StackInit();

	//************************************************************************************
	//									INICIALIZACOES
	//************************************************************************************	
	//Apaga LED_AV,LED_TAKE,LED_ENABLE,LED_CONFIG
	ApagaLed(AV,YELLOW);
	ApagaLed(TAKE,YELLOW);
	ApagaLed(ENABLE,YELLOW);
	ApagaLed(CONFIG,YELLOW);
	//Pisca todos os Leds Verdes e depois todos os Vermelhos nas teclas
	ApagaTodosLeds();
	TestaLeds(); 		
	//************************************************************************************
	//ESTADO INICIAL DAS VARIÁVEIS
	f=0;
	STATUS_ENABLE=STATUS_TAKE=STATUS_CONFIG=0;
	Tecla_Apertada_Anterior=-1;
	i=-1;
	BORDA_AV=BORDA_TAKE=BORDA_ENABLE=BORDA_CONFIG=BORDA_ENTRADAS=0;
	ESTADO_AV=ESTADO_TAKE=ESTADO_CONFIG=0;
	ESTADO_ENABLE = 1;
	contador_time_atualizacao = 0;
	CONTA_DELAY_ENABLE=0;
	flagEnable_Permanente =0;
	
	Entrada_Video = -1;
	Entrada_Audio = -1;
	Entrada_Audio_Take = -1;
	Entrada_Video_Take = -1;
	Entrada_Audio_Take_Temp = -1;
	Entrada_Video_Take_Temp = -1;
	
	//VARIÁVEIS NA EEPROM
	
	SAIDA = 0;
	
	IP_MATRIZ_AUDIO.v[0] = 192; 
	IP_MATRIZ_AUDIO.v[1] = 168; 
	IP_MATRIZ_AUDIO.v[2] = 100; 
	IP_MATRIZ_AUDIO.v[3] = 51; 
	
	
	IP_MATRIZ_VIDEO.v[0] = 192; 
	IP_MATRIZ_VIDEO.v[1] = 168; 
	IP_MATRIZ_VIDEO.v[2] = 100; 
	IP_MATRIZ_VIDEO.v[3] = 50;  

	//************************************************************************************
	DelayMili(3);
	STATUS_AV=-1;
	TestaAudioVideo();	
	AcendeLed(ENABLE,RED);
	
	//************************************************************************************
   while(1)             //Loop Infinito
   {

		MaquinaTeclas();
		
		if(STATUS_ENABLE == 1)
		{				
			if(flagEnable_Permanente == 0)
			{
				contador_time_atualizacao ++;
			}
			else
			{
				contador_time_atualizacao = 0;
			}
			if (contador_time_atualizacao<=40000)
			{
			
				if(STATUS_CONFIG != 1)
				{
					if((Entrada_Apertada != -1)||(STATUS_TAKE ==1))//Verifica se existe uma tecla pressionada
					{
						if((Entrada_Apertada != Entrada_Apertada_Anterior)||(STATUS_TAKE ==1))
						{							
							contador_time_atualizacao = 0;							
							ProcessaEntradas();
						}
					}		
				}		
				
			}	
			
			else
			{
				ApagaLed(ENABLE,YELLOW);
				AcendeLed(ENABLE,RED);	
				ApagaLed(TAKE,YELLOW);
				ApagaLed(CONFIG,YELLOW);
				contador_time_atualizacao = 0;//Zera variável de contagem do tempo Ativo do Enable
				contador_brilho = 0;//Zera variável de contagem de brilho
				STATUS_ENABLE = 0; //Desliga Flag da Tecla Enable-Ativa
				STATUS_CONFIG = 0; //Desliga Flag da Tecla Enable-Ativa	
				SAIDA_TEMP = -1;
				STATUS_TAKE = 0; //Desliga Flag da Tecla Enable-Ativa
				ESTADO_ENABLE = 1;
				Entrada_Apertada = -1;
				Entrada_Apertada_Anterior = -1;
				Entrada_Audio_Take = -1;
				Entrada_Video_Take = -1;
			}
		}
		AcendeLedSequencial();
   }
}

/******************************************************************************
  NOME: void MaquinaTeclas(void)
  DESCRIÇÃO: Máquina de estados das teclas.
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
void MaquinaTeclas(void)
{	
	if(CONTA_DELAY_TECLA>=65000)CONTA_DELAY_TECLA=0;
	CONTA_DELAY_TECLA++;
	Tecla_Apertada = TestaTeclas();
	if(Tecla_Apertada_Anterior!=Tecla_Apertada) 
	{
		CONTA_DELAY_TECLA=0;
		Tecla_Apertada_Anterior=Tecla_Apertada;
	}
	if(CONTA_DELAY_TECLA>DELAY_TECLA)
	{
		if(Tecla_Apertada==ENABLE)
		{
			if(BORDA_ENABLE==0)
			{
				BORDA_ENABLE=1;
			}
		}
		else if((Tecla_Apertada==AV)&&(STATUS_ENABLE == 1))
		{
			if(STATUS_CONFIG==0)
			{
				CONTA_DELAY_ENABLE=-1;
				if(BORDA_AV==0)
				{
					BORDA_AV=1;
				}
			}
		}
		else if((Tecla_Apertada==TAKE)&&(STATUS_ENABLE == 1))
		{
			if(STATUS_CONFIG==0)
			{
				CONTA_DELAY_ENABLE=-1;
				if(BORDA_TAKE==0)
				{
					BORDA_TAKE=1;
				}
			}
		}
		else if((Tecla_Apertada==CONFIG)&&(STATUS_ENABLE == 1))
		{
			CONTA_DELAY_ENABLE=-1;
			if(BORDA_CONFIG==0)
			{
				BORDA_CONFIG=1;
			}
		}
		else if((Tecla_Apertada>=0)&&(STATUS_ENABLE == 1))
		{
			CONTA_DELAY_ENABLE=-1;
			if(BORDA_ENTRADAS==0)
			{
				BORDA_ENTRADAS=1;				
				if(STATUS_CONFIG==1)
				{
					if(SAIDA_TEMP !=-1)ApagaLed(SAIDA_TEMP,YELLOW);
					if(SAIDA_TEMP == Tecla_Apertada) SAIDA_TEMP = -1;
					else SAIDA_TEMP = Tecla_Apertada;
					contador_time_atualizacao = 0;
				}
				else
				{
					ApagaLed(Entrada_Apertada,YELLOW);
					Entrada_Apertada = 	Tecla_Apertada;
					contador_time_atualizacao = 0;
				}				
			}
		}		
		else if(Tecla_Apertada=-1)
		{
			if(BORDA_ENABLE==1)
			{				
				if(CONTA_DELAY_ENABLE>DELAY_ENABLE)
				{					
					switch(ESTADO_ENABLE)
					{	
						case 0:
						{
							Entrada_Audio = Entrada_Audio_Temp;
							Entrada_Video = Entrada_Video_Temp;
							contador_time_atualizacao = 0;//Zera variável de contagem do tempo Ativo do Enable
							contador_brilho = 0;//Zera variável de contagem de brilho
							STATUS_ENABLE = 0; //Desliga Flag da Tecla Enable-Ativa
							STATUS_CONFIG = 0; //Desliga Flag da Tecla Enable-Ativa				
							STATUS_TAKE = 0; //Desliga Flag da Tecla Enable-Ativa
							ESTADO_ENABLE = 1;
							ESTADO_CONFIG=0;
							Entrada_Apertada = -1;
							Entrada_Apertada_Anterior = -1;
							Entrada_Audio_Take = -1;
							Entrada_Video_Take = -1;
							flagEnable_Permanente = 0;
							ApagaLed(TAKE,YELLOW);
							ApagaLed(CONFIG,YELLOW);
							ApagaLed(ENABLE,YELLOW);
							AcendeLed(ENABLE,RED);
							break;
						}
						case 1:
						{
							flagEnable_Permanente = 1;
							STATUS_ENABLE=1;
							Entrada_Apertada = -1;
							Entrada_Apertada_Anterior = -1;
							Entrada_Audio_Take = -1;
							Entrada_Video_Take = -1;
							ApagaLed(ENABLE,YELLOW);
							AcendeLed(ENABLE,GREEN);
							
							
							ESTADO_ENABLE=0;
							break;
						}
					}
					
				}
				else
				{
					flagEnable_Permanente = 0;
					switch(ESTADO_ENABLE)
						{	
						case 0:
						{
							Entrada_Audio = Entrada_Audio_Temp;
							Entrada_Video = Entrada_Video_Temp;
							contador_time_atualizacao = 0;//Zera variável de contagem do tempo Ativo do Enable
							contador_brilho = 0;//Zera variável de contagem de brilho
							STATUS_ENABLE = 0; //Desliga Flag da Tecla Enable-Ativa
							STATUS_CONFIG = 0; //Desliga Flag da Tecla Enable-Ativa				
							STATUS_TAKE = 0; //Desliga Flag da Tecla Enable-Ativa
							ESTADO_ENABLE = 1;
							ESTADO_CONFIG=0;
							Entrada_Apertada = -1;
							Entrada_Apertada_Anterior = -1;
							Entrada_Audio_Take = -1;
							Entrada_Video_Take = -1;
							ApagaLed(TAKE,YELLOW);
							ApagaLed(CONFIG,YELLOW);
							ApagaLed(ENABLE,YELLOW);
							AcendeLed(ENABLE,RED);
							break;
						}
						case 1:
						{
							Entrada_Apertada = -1;
							Entrada_Apertada_Anterior = -1;
							Entrada_Audio_Take = -1;
							Entrada_Video_Take = -1;
							STATUS_ENABLE=1;
							ApagaLed(ENABLE,YELLOW);
							AcendeLed(ENABLE,GREEN);
							ESTADO_ENABLE=0;
							break;
						}
					}	
				}
				CONTA_DELAY_ENABLE = 0;
				BORDA_ENABLE=0;
			}
			else if((BORDA_AV==1)&&(STATUS_ENABLE==1))
			{
				contador_time_atualizacao = 0;//Zera variável de contagem do tempo Ativo do Enable
				Entrada_Apertada_Anterior=-1;				
				TestaAudioVideo();
				BORDA_AV=0;
			}
			else if((BORDA_TAKE==1)&&(STATUS_ENABLE==1))
			{
				ApagaTodosLeds();
				contador_time_atualizacao = 0;//Zera variável de contagem do tempo Ativo do Enable
				if(STATUS_AV!=AUDIO_VIDEO)STATUS_TAKE=1;
				BORDA_TAKE=0;
			
			}
			else if((BORDA_CONFIG==1)&&(STATUS_ENABLE==1))
			{
				switch(ESTADO_CONFIG)
				{
					case 0:
					{	
						contador_time_atualizacao = 0;//Zera variável de contagem do tempo Ativo do Enable
						ApagaTodosLeds();
						AcendeLed(SAIDA,YELLOW);
						AcendeLed(CONFIG,YELLOW);
						STATUS_CONFIG=1;
						SAIDA_TEMP=-1;
						ESTADO_CONFIG=1;
						Entrada_Audio_Temp = Entrada_Audio;
						Entrada_Video_Temp = Entrada_Video;
						Entrada_Audio=-1;
						Entrada_Video=-1;
						Entrada_Audio_Take=-1;
						Entrada_Video_Take=-1;
						//Entrada_Apertada_Anterior=-1;
						break;
					}
					case 1:
					{
						contador_time_atualizacao = 0;//Zera variável de contagem do tempo Ativo do Enable
						if(SAIDA_TEMP==-1)//Abortar operação
						{
							//Acende LED_CONFIG vermelho (basta apagar o verde):
							ApagaLed(CONFIG,GREEN);
							DelayMili(1);
							//Apaga o LED_CONFIG:
							ApagaLed(CONFIG,RED);	
							ApagaTodosLeds();
						}	
						else //Gravar nova saida
						{
							SAIDA=SAIDA_TEMP;
							//Acende LED_CONFIG verde (basta apagar o vermelho):
							ApagaLed(CONFIG,RED);
							DelayMili(1);
							//Apaga o LED_CONFIG:
							ApagaLed(CONFIG,GREEN);
							ApagaLed(SAIDA,YELLOW);
							SAIDA_TEMP=-1;
						}
						ESTADO_CONFIG=0;
						STATUS_CONFIG=0;
						if(STATUS_AV==AUDIO||STATUS_AV<0)STATUS_AV=AUDIO_VIDEO;
						else STATUS_AV-=1;
						TestaAudioVideo();
						break;
					}
				}
				BORDA_CONFIG=0;
			}
			else if(BORDA_ENTRADAS==1)
			{	
				BORDA_ENTRADAS = 0;
				
			}
			else if(STATUS_CONFIG==1)
			{
				if(SAIDA_TEMP !=-1)
				{
					AcendeLed(SAIDA_TEMP,GREEN);
					DelayNano(12);
					ApagaLed(SAIDA_TEMP, YELLOW);
					AcendeLed(SAIDA_TEMP,RED);
					
					DelayNano(5);
					ApagaLed(SAIDA_TEMP, YELLOW);
				}
				else 
				{
					AcendeLed(SAIDA,GREEN);
					DelayNano(12);
					ApagaLed(SAIDA, YELLOW);
					AcendeLed(SAIDA,RED);
					
					DelayNano(5);
					ApagaLed(SAIDA, YELLOW);
				}
			}	
		}
		
		if(BORDA_ENABLE == 1)
		{
			//INSERIR PARTE DE CONTAGEM PARA O DELAY DO ENABLE
			CONTA_DELAY_ENABLE++;
			if(CONTA_DELAY_ENABLE>DELAY_ENABLE)
			{
				if(ESTADO_ENABLE == 1)
				{
					ApagaLed(ENABLE,YELLOW);
					AcendeLed(ENABLE,GREEN);
					DelayMili(3);
					ApagaLed(ENABLE,YELLOW);
					DelayMili(3);
					AcendeLed(ENABLE,GREEN);
					DelayMili(3);
					ApagaLed(ENABLE,YELLOW);
					DelayMili(3);
					AcendeLed(ENABLE,GREEN);
					DelayMili(3);
					ApagaLed(ENABLE,YELLOW);
					DelayMili(3);
				}				
			}
		}	
	}
}

/******************************************************************************
  NOME: void AcendeLedSequencial(void)
  DESCRIÇÃO: Acende ou pisca leds de forma exclusiva e em sincronia.
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
void AcendeLedSequencial(void)
{
	contador_brilho_aux++;
	contador_brilho ++;
	if(((Entrada_Audio_Take != -1)&&(Entrada_Video_Take !=-1))&&((Entrada_Audio != -1)&&(Entrada_Video !=-1)))//Verifica se existe alguma tecla em preview
	{ 
		if(contador_brilho<150)
		{				
			ApagaTodosLeds();
			if(contador_brilho_aux<6)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Audio,GREEN);
			}
			else if(contador_brilho_aux<12)
			{
				ApagaTodosLeds();
			}
			else if(contador_brilho_aux<15)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Video,RED);				
			
			}
			else if(contador_brilho_aux<18)
			{
				ApagaTodosLeds();
			}
			else contador_brilho_aux=0;
		}
		else if(contador_brilho<300)
		{						
			ApagaTodosLeds();
			if(contador_brilho_aux<6)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Audio,GREEN);
			}
			else if(contador_brilho_aux<12)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Audio_Take,GREEN);
			}
			else if(contador_brilho_aux<15)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Video,RED);
			}
			else if(contador_brilho_aux<18)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Video_Take,RED);
			}
			else contador_brilho_aux=0;			
		}
		else contador_brilho=0;		
	}
	else if(((Entrada_Video_Take !=-1))&&((Entrada_Audio != -1)&&(Entrada_Video !=-1)))//Verifica se existe alguma tecla em preview
	{ 
		if(contador_brilho<150)
		{				
			ApagaTodosLeds();
			if(contador_brilho_aux<6)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Audio,GREEN);
			}
			else if(contador_brilho_aux<12)
			{
				ApagaTodosLeds();
			}
			else if(contador_brilho_aux<15)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Video,RED);				
			}
			else if(contador_brilho_aux<18)
			{
				ApagaTodosLeds();
			}
			else contador_brilho_aux=0;
		}
		else if(contador_brilho<300)
		{						
			ApagaTodosLeds();
			if(contador_brilho_aux<6)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Audio,GREEN);
			}
			else if(contador_brilho_aux<12)
			{
				ApagaTodosLeds();
			}
			else if(contador_brilho_aux<15)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Video,RED);			
			}
			else if(contador_brilho_aux<18)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Video_Take,RED);			
			}
			else contador_brilho_aux=0;			
		}
		else contador_brilho=0;		
	}
	else if(((Entrada_Audio_Take != -1))&&((Entrada_Audio != -1)&&(Entrada_Video !=-1)))//Verifica se existe alguma tecla em preview
	{ 
		if(contador_brilho<150)
		{				
			ApagaTodosLeds();
			if(contador_brilho_aux<6)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Audio,GREEN);
			}
			else if(contador_brilho_aux<12)
			{
				ApagaTodosLeds();
			}
			else if(contador_brilho_aux<15)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Video,RED);			
			}
			else if(contador_brilho_aux<18)
			{
				ApagaTodosLeds();
			}
			else contador_brilho_aux=0;
		}
		else if(contador_brilho<300)
		{						
			ApagaTodosLeds();
			if(contador_brilho_aux<6)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Audio,GREEN);
			}
			else if(contador_brilho_aux<12)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Audio_Take,GREEN);
			}
			else if(contador_brilho_aux<15)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Video,RED);
			}
			else if(contador_brilho_aux<18)
			{
				ApagaTodosLeds();		
			}
			else contador_brilho_aux=0;
		}
		else contador_brilho=0;		
	}
	else if((Entrada_Audio_Take != -1)&&(Entrada_Audio != -1))//Verifica se existe alguma tecla em preview
	{ 
		if(contador_brilho<150)
		{				
			ApagaTodosLeds();
			if(contador_brilho_aux<6)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Audio,GREEN);
			}
			else if(contador_brilho_aux<12)
			{
				ApagaTodosLeds();
			}
			else if(contador_brilho_aux<15)
			{
				ApagaTodosLeds();		
			}
			else if(contador_brilho_aux<18)
			{
				ApagaTodosLeds();
			}
			else contador_brilho_aux=0;
		}
		else if(contador_brilho<300)
		{						
			ApagaTodosLeds();
			if(contador_brilho_aux<6)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Audio,GREEN);
			}
			else if(contador_brilho_aux<12)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Audio_Take,GREEN);
			}
			else if(contador_brilho_aux<15)
			{
				ApagaTodosLeds();
			}
			else if(contador_brilho_aux<18)
			{
				ApagaTodosLeds();
			}
			else contador_brilho_aux=0;
			
		}
		else contador_brilho=0;		
	}
	else if((Entrada_Video_Take != -1)&&(Entrada_Video != -1))//Verifica se existe alguma tecla em preview
	{ 
		if(contador_brilho<150)
		{				
			ApagaTodosLeds();
			if(contador_brilho_aux<6)
			{
				ApagaTodosLeds();
			}
			else if(contador_brilho_aux<12)
			{
				ApagaTodosLeds();
			}
			else if(contador_brilho_aux<15)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Video,RED);
			}
			else if(contador_brilho_aux<18)
			{
				ApagaTodosLeds();			
			}
			else contador_brilho_aux=0;
		}
		else if(contador_brilho<300)
		{						
			ApagaTodosLeds();
			if(contador_brilho_aux<6)
			{
				ApagaTodosLeds();
			}
			else if(contador_brilho_aux<12)
			{
				ApagaTodosLeds();
			}
			else if(contador_brilho_aux<15)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Video,RED);			
			}
			else if(contador_brilho_aux<18)
			{
				ApagaTodosLeds();
				AcendeLed(Entrada_Video_Take,RED);			
			}
			else contador_brilho_aux=0;
			
		}
		else contador_brilho=0;		
	}
	else if(((Entrada_Audio != -1)&&(Entrada_Video !=-1)))//Verifica se existe alguma tecla em preview
	{ 	
		ApagaTodosLeds();
		if(contador_brilho_aux<6)
		{
			ApagaTodosLeds();
			AcendeLed(Entrada_Audio,GREEN);
		}
		else if(contador_brilho_aux<12)
		{
			ApagaTodosLeds();
		}
		else if(contador_brilho_aux<15)
		{
			ApagaTodosLeds();
			AcendeLed(Entrada_Video,RED);		
		}
		else if(contador_brilho_aux<18)
		{
			ApagaTodosLeds();		
		}
		else contador_brilho_aux=0;
	}
	else if(((Entrada_Audio != -1)))//Verifica se existe alguma tecla em preview
	{ 
		ApagaTodosLeds();
		AcendeLed(Entrada_Audio,GREEN);
	}
	else if(((Entrada_Video != -1)))//Verifica se existe alguma tecla em preview
	{ 
		ApagaTodosLeds();
		AcendeLed(Entrada_Video,RED);
	}	
	else
	{		
		ApagaTodosLeds();
		contador_brilho = 0;
	}
}	

/******************************************************************************
  NOME: void DelayNop()
  DESCRIÇÃO: Função delay com 10 ciclos de máquina.
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
void DelayNop() 
{

		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();

}

/******************************************************************************
  NOME: void DelayMili(long mili) 
  DESCRIÇÃO: Função delay em milisegundos.
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
void DelayMili(long mili) 
{
	long long i;
	long long j;
	for(i=0;i<mili;i++)
	{
		for(j=0;j<50000;j++);
	}
}

/******************************************************************************
  NOME: void DelayNano(long nano)
  DESCRIÇÃO: Função delay em nanosegundos.
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
void DelayNano(long nano) 
{
	long long i;
	long long j;
	for(i=0;i<nano;i++)
	{
		for(j=0;j<10;j++);
	}
}

/******************************************************************************
  NOME: int TestaTeclas()
  DESCRIÇÃO: Verifica qual tecla foi pressionada.
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
int TestaTeclas()
{
	int ROW = -1;
	int COL;
	int ret = -1;
	//TESTA 32 TECLAS DE ENTRADAS:
	for(i=0;i<8;i++) //COLUNAS
	{
		TECLA_COL1=0;//COLUNA 1
		DelayNop();
		TECLA_COL2=0;//COLUNA 2
		DelayNop();
		TECLA_COL3=0;//COLUNA 3
		DelayNop();
		TECLA_COL4=0;//COLUNA 4
		DelayNop();
		TECLA_COL5=0;//COLUNA 5
		DelayNop();
		TECLA_COL6=0;//COLUNA 6
		DelayNop();
		TECLA_COL7=0;//COLUNA 7
		DelayNop();
		TECLA_COL8=0;//COLUNA 8
		DelayNop();
		switch(i)
		{
			case 0: //COLUNA1
				TECLA_COL1=1;
				break;
			case 1: //COLUNA2
				TECLA_COL2=1;
				break;
			case 2: //COLUNA3
				TECLA_COL3=1;
				break;
			case 3: //COLUNA4
				TECLA_COL4=1;
				break;
			case 4: //COLUNA5
				TECLA_COL5=1;
				break;
			case 5: //COLUNA6
				TECLA_COL6=1;
				break;
			case 6: //COLUNA7
				TECLA_COL7=1;
				break;
			case 7: //COLUNA8
				TECLA_COL8=1;
				break;

		}
		for(j=0;j<4;j++)
		{
			switch(j)
			{
				case 0: //ROW1
					if(TECLA_ROW1!=0)
					{
						ROW=j;
						COL=i;
					}
					break;
				case 1: //ROW2
					if(TECLA_ROW2!=0)
					{
						ROW=j;
						COL=i;
					}
					break;
				case 2: //ROW3
					if(TECLA_ROW3!=0)
					{
						ROW=j;	
						COL=i;
					}			
					break;
				case 3:  //ROW4
					if(TECLA_ROW4!=0)
					{
						ROW=j;
						COL=i;
					}
					break;		
			}
			
		}
		TECLA_COL1=0;//COLUNA 1
		DelayNop();
		TECLA_COL2=0;//COLUNA 2
		DelayNop();
		TECLA_COL3=0;//COLUNA 3
		DelayNop();
		TECLA_COL4=0;//COLUNA 4
		DelayNop();
		TECLA_COL5=0;//COLUNA 5
		DelayNop();
		TECLA_COL6=0;//COLUNA 6
		DelayNop();
		TECLA_COL7=0;//COLUNA 7
		DelayNop();
		TECLA_COL8=0;//COLUNA 8
		DelayNop();
	}
	if(ROW != -1)
	{
		ret=COL*4+ROW;
	}
	//TESTA TECLAS ESPECIAIS (AV,TAKE,ENABLE,CONFIG):
	else if(TECLA_AV==1)
	{
		ret=AV;
	}

	else if(TECLA_TAKE==1)
	{
		ret=TAKE;
	}
	else if(TECLA_ENABLE==1)
	{
		ret=ENABLE;
	}
	else if(TECLA_CONFIG==1)
	{
		ret=CONFIG;
	}
	return ret;	
}

/******************************************************************************
  NOME: void ApagaLed(int x,int COR)
  DESCRIÇÃO: apaga determinado Led com Cor específica.
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
void ApagaLed(int x,int COR)
{
	if(x==AV) 
	{
		if(COR==RED)LEDRED_AV=1;
		else if(COR==GREEN)LEDGREEN_AV=1;
		else if(COR==YELLOW)
		{
			LEDRED_AV=1;
			LEDGREEN_AV=1;
		}
	}
	else if(x==TAKE) 
	{
		if(COR==RED)LEDRED_TAKE=1;
		else if(COR==GREEN)LEDGREEN_TAKE=1;
		else if(COR==YELLOW)
		{
			LEDRED_TAKE=1;
			LEDGREEN_TAKE=1;
		}
	}
	else if(x==ENABLE) 
	{
		if(COR==RED)LEDRED_ENABLE=1;
		else if(COR==GREEN)LEDGREEN_ENABLE=1;
		else if(COR==YELLOW)
		{
			LEDRED_ENABLE=1;
			LEDGREEN_ENABLE=1;
		}
	}
	else if(x==CONFIG) 
	{
		if(COR==RED)LEDRED_CONFIG=1;
		else if(COR==GREEN)LEDGREEN_CONFIG=1;
		else if(COR==YELLOW)
		{
			LEDRED_CONFIG=1;
			LEDGREEN_CONFIG=1;
		}
	}
	else //ENTRADAS 0 a 31
	{
		int ROW = x%4;
		int COL =(int) x>>2;
		switch(ROW)
		{
			case 0: //ROW1
				if(COR==RED)LEDRED_ROW1=1;
				else if(COR==GREEN)LEDGREEN_ROW1=1;
				else if(COR==YELLOW)
				{
					LEDRED_ROW1=1;
					LEDGREEN_ROW1=1;
				}
				break;
			case 1: //ROW2
				if(COR==RED)LEDRED_ROW2=1;
				else if(COR==GREEN)LEDGREEN_ROW2=1;
				else if(COR==YELLOW)
				{
					LEDRED_ROW2=1;
					LEDGREEN_ROW2=1;
				} 
				break;
			case 2: //ROW3
				if(COR==RED)LEDRED_ROW3=1;
				else if(COR==GREEN)LEDGREEN_ROW3=1;
				else if(COR==YELLOW)
				{
					LEDRED_ROW3=1;
					LEDGREEN_ROW3=1;
				} 
				break;
			case 3:  //ROW4
				if(COR==RED)LEDRED_ROW4=1;
				else if(COR==GREEN)LEDGREEN_ROW4=1;
				else if(COR==YELLOW)
				{
					LEDRED_ROW4=1;
					LEDGREEN_ROW4=1;
				}
				break;		
		}
		switch(COL)
		{
			case 0: //COLUNA1
				LED_COL1=0;
				break;
			case 1: //COLUNA2
				LED_COL2=0;
				break;
			case 2: //COLUNA3
				LED_COL3=0;
				break;
			case 3: //COLUNA4
				LED_COL4=0;
				break;
			case 4: //COLUNA5
				LED_COL5=0;
				break;
			case 5: //COLUNA6
				LED_COL6=0;
				break;
			case 6: //COLUNA7
				LED_COL7=0;
				break;
			case 7: //COLUNA8
				LED_COL8=0;
				break;
	
		}
	}
}

/******************************************************************************
  NOME: void ApagaTodosLeds()
  DESCRIÇÃO: Apaga todos os Leds das Entradas (0 a 31).
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
void ApagaTodosLeds() 
{
	
		LEDRED_ROW1=1;
		LEDGREEN_ROW1=1;
		LEDRED_ROW2=1;
		LEDGREEN_ROW2=1;
		LEDRED_ROW3=1;
		LEDGREEN_ROW3=1;
		LEDRED_ROW4=1;
		LEDGREEN_ROW4=1;
	
		LED_COL1=0;
		LED_COL2=0;
		LED_COL3=0;
		LED_COL4=0;
		LED_COL5=0;
		LED_COL6=0;
		LED_COL7=0;
		LED_COL8=0;

	
}

/******************************************************************************
  NOME: void AcendeLed(int x,int COR)
  DESCRIÇÃO: Acende determinado Led com Cor específica.
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
void AcendeLed(int x,int COR)
{
	if(x==AV) 
	{
		if(COR==RED)LEDRED_AV=0;
		else if(COR==GREEN)LEDGREEN_AV=0;
		else if(COR==YELLOW)
		{
			LEDRED_AV=0;
			LEDGREEN_AV=0;
		}
	}
	else if(x==TAKE)
	{
		if(COR==RED)LEDRED_TAKE=0;
		else if(COR==GREEN)LEDGREEN_TAKE=0;
		else if(COR==YELLOW)
		{
			LEDRED_TAKE=0;
			LEDGREEN_TAKE=0;
		}
	}
	else if(x==ENABLE)
	{
		if(COR==RED)LEDRED_ENABLE=0;
		else if(COR==GREEN)LEDGREEN_ENABLE=0;
		else if(COR==YELLOW)
		{
			LEDRED_ENABLE=0;
			LEDGREEN_ENABLE=0;
		}
	}
	else if(x==CONFIG) 
	{
		if(COR==RED)LEDRED_CONFIG=0;
		else if(COR==GREEN)LEDGREEN_CONFIG=0;
		else if(COR==YELLOW)
		{
			LEDRED_CONFIG=0;
			LEDGREEN_CONFIG=0;
		}
	}
	else //ENTRADAS 0 a 31
	{
		int ROW = (int)(x%4);
		int COL =(int) x>>2;
		switch(ROW)
		{
			case 0: //ROW1
				if(COR==RED)LEDRED_ROW1=0;
				else if(COR==GREEN)LEDGREEN_ROW1=0;
				else if(COR==YELLOW)
				{
					LEDRED_ROW1=0;
					LEDGREEN_ROW1=0;
				}
				break;
			case 1: //ROW2
				if(COR==RED)LEDRED_ROW2=0;
				else if(COR==GREEN)LEDGREEN_ROW2=0;
				else if(COR==YELLOW)
				{
					LEDRED_ROW2=0;
					LEDGREEN_ROW2=0;
				} 
				break;
			case 2: //ROW3
				if(COR==RED)LEDRED_ROW3=0;
				else if(COR==GREEN)LEDGREEN_ROW3=0;
				else if(COR==YELLOW)
				{
					LEDRED_ROW3=0;
					LEDGREEN_ROW3=0;
				} 
				break;
			case 3:  //ROW4
				if(COR==RED)LEDRED_ROW4=0;
				else if(COR==GREEN)LEDGREEN_ROW4=0;
				else if(COR==YELLOW)
				{
					LEDRED_ROW4=0;
					LEDGREEN_ROW4=0;
				}
				break;		
		}
		switch(COL)
		{
			case 0: //COLUNA1
				LED_COL1=1;
				break;
			case 1: //COLUNA2
				LED_COL2=1;
				break;
			case 2: //COLUNA3
				LED_COL3=1;
				break;
			case 3: //COLUNA4
				LED_COL4=1;
				break;
			case 4: //COLUNA5
				LED_COL5=1;
				break;
			case 5: //COLUNA6
				LED_COL6=1;
				break;
			case 6: //COLUNA7
				LED_COL7=1;
				break;
			case 7: //COLUNA8
				LED_COL8=1;
				break;
	
		}
	}
}

/******************************************************************************
  NOME: void TestaLeds()
  DESCRIÇÃO: Testa os leds durante a inicialização.
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
void TestaLeds()
{
	long long j,i;
	//TESTA TODOS OS LEDS VERDES
    for(j=0;j<32;j++)
    {
		AcendeLed(j,GREEN);
		for(i=0;i<50000;i++);
		ApagaLed(j,GREEN);
    }	
	//TESTA TODOS OS LEDS VERMELHOS
    STATUS_AV=VIDEO;
    for(j=0;j<32;j++)
    {
		AcendeLed(j,RED);
		for(i=0;i<50000;i++);
		ApagaLed(j,RED);
    }
	ApagaTodosLeds();	

}

/******************************************************************************
  NOME: BOOL TestaAudioVideo()
  DESCRIÇÃO: Verifica conexão com Audio e\ou Video.
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
BOOL TestaAudioVideo()
{	
	if(Entrada_Audio!=-1)ApagaLed(Entrada_Audio, YELLOW);
	if(Entrada_Video!=-1)ApagaLed(Entrada_Video, YELLOW);
	CONTA_PING_TIME_OUT=0;
	ApagaLed(AV,YELLOW);
	//SE STATUS_AV ATUAL FOR -1 OU AUDIO_VIDEO: 
	if(STATUS_AV==AUDIO_VIDEO)
	{		
		Entrada_Audio_Take = Entrada_Audio_Take_Temp;
		Entrada_Video_Take = Entrada_Video_Take_Temp;
		STATUS_AV=-1;
		//TENTA MUDAR PARA AUDIO PRIMEIRO
		CONTA_PING_TIME_OUT=0;
		while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
		{
				StackTask();
				if((CONTA_PING_TIME_OUT%256)==0) ApagaLed(AV,YELLOW);
				else if((CONTA_PING_TIME_OUT%256)==128) AcendeLed(AV,GREEN);
				if(PingDemo(IP_MATRIZ_AUDIO) == TRUE)
				{
					STATUS_AV = AUDIO;
					ApagaLed(AV, YELLOW);
					AcendeLed(AV,GREEN);
					CONTA_PING_TIME_OUT=PING_TIME_OUT + 1;
					ICMPEndUsage();
					StackTask();
				}
				CONTA_PING_TIME_OUT++;
		}
		//TENTA MUDAR PARA VIDEO CASO NÃO FOI POSSÍVEL ÁUDIO
		if(STATUS_AV==-1)
		{
			Entrada_Audio=Entrada_Audio_Take=Entrada_Audio_Take_Temp=-1;
			CONTA_PING_TIME_OUT=0;
			while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
			{
					StackTask();
					if((CONTA_PING_TIME_OUT%256)==0) ApagaLed(AV,YELLOW);
					else if((CONTA_PING_TIME_OUT%256)==128) AcendeLed(AV,RED);
					if(PingDemo(IP_MATRIZ_VIDEO) == TRUE)
					{
						STATUS_AV = VIDEO;
						ApagaLed(AV, YELLOW);
						AcendeLed(AV,RED);
						CONTA_PING_TIME_OUT=PING_TIME_OUT + 1;
						ICMPEndUsage();
						StackTask();
					}
					CONTA_PING_TIME_OUT++;
			}
		}
		if(STATUS_AV==-1)Entrada_Video=Entrada_Video_Take=Entrada_Video_Take_Temp=-1;
	}
	//SE STATUS_AV ATUAL FOR AUDIO:
	else if(STATUS_AV==AUDIO)
	{
		Entrada_Audio_Take_Temp = Entrada_Audio_Take;
		Entrada_Video_Take_Temp = Entrada_Video_Take;
		STATUS_AV = -1;
		//TENTA MUDAR PARA VIDEO PRIMEIRO
	    CONTA_PING_TIME_OUT=0;
		while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
		{
			StackTask();
			if((CONTA_PING_TIME_OUT%256)==0) ApagaLed(AV,YELLOW);
			else if((CONTA_PING_TIME_OUT%256)==128) AcendeLed(AV,RED);
			if(PingDemo(IP_MATRIZ_VIDEO) == TRUE)
			{
				STATUS_AV = VIDEO;
				ApagaLed(AV, YELLOW);
				AcendeLed(AV,RED);
				CONTA_PING_TIME_OUT=PING_TIME_OUT + 1;
				ICMPEndUsage();
				StackTask();
			}
			CONTA_PING_TIME_OUT++;
		}
		//TENTA VOLTAR PARA AUDIO SE NÃO FOI POSSÍVEL VIDEO
		if(STATUS_AV ==-1)
		{
			Entrada_Video=Entrada_Video_Take=Entrada_Video_Take_Temp=-1;
			CONTA_PING_TIME_OUT=0;
			while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
			{
					StackTask();
					if((CONTA_PING_TIME_OUT%256)==0) ApagaLed(AV,YELLOW);
					else if((CONTA_PING_TIME_OUT%256)==128) AcendeLed(AV,GREEN);
					if(PingDemo(IP_MATRIZ_AUDIO))
					{
						STATUS_AV = AUDIO;
						ApagaLed(AV, YELLOW);
						AcendeLed(AV,GREEN);
						CONTA_PING_TIME_OUT=PING_TIME_OUT + 1;
						ICMPEndUsage();
						StackTask();
					}
					CONTA_PING_TIME_OUT++;
			}
		}
		if(STATUS_AV ==-1)Entrada_Audio=Entrada_Audio_Take=Entrada_Audio_Take_Temp=-1;
	}
	//SE STATUS_AV ATUAL FOR VIDEO:
	else if((STATUS_AV==-1)||(STATUS_AV==VIDEO))
	{
		STATUS_AV=-1;
		//TESTA AUDIO PRIMEIRO
		CONTA_PING_TIME_OUT=0;
		while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
		{
				StackTask();
				if((CONTA_PING_TIME_OUT%256)==0) ApagaLed(AV,YELLOW);
				else if((CONTA_PING_TIME_OUT%256)==128) AcendeLed(AV,GREEN);
				if(PingDemo(IP_MATRIZ_AUDIO)==TRUE)
				{
					STATUS_AV = AUDIO;
					ApagaLed(AV, YELLOW);
					AcendeLed(AV,GREEN);
					CONTA_PING_TIME_OUT=PING_TIME_OUT + 1;
					ICMPEndUsage();
					StackTask();
				}
				CONTA_PING_TIME_OUT++;
		}
		//TESTA VIDEO TAMBÉM PARA CONCLUIR SE É POSSIVEL AUDIO_VIDEO
		CONTA_PING_TIME_OUT=0;	
		while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
		{
				StackTask();
				if((CONTA_PING_TIME_OUT%256)==0) ApagaLed(AV,YELLOW);
				else if((CONTA_PING_TIME_OUT%256)==128) AcendeLed(AV,RED);
				if(PingDemo(IP_MATRIZ_VIDEO) == TRUE)
				{
					if(STATUS_AV == AUDIO)
					{
						STATUS_AV = AUDIO_VIDEO;
						AcendeLed(AV,YELLOW);
						Entrada_Audio_Take_Temp = Entrada_Audio_Take;
						Entrada_Video_Take_Temp = Entrada_Video_Take;
						Entrada_Audio_Take = -1;
						Entrada_Video_Take = -1;
					}
					else 
					{
						Entrada_Audio=Entrada_Audio_Take=Entrada_Audio_Take_Temp=-1;
						STATUS_AV = VIDEO;
						ApagaLed(AV,YELLOW);
						AcendeLed(AV,RED);
					}
					CONTA_PING_TIME_OUT=PING_TIME_OUT + 1;
					ICMPEndUsage();
					StackTask();
				}
				CONTA_PING_TIME_OUT++;
		}
		if(STATUS_AV==-1)Entrada_Audio=Entrada_Audio_Take=Entrada_Audio_Take_Temp=Entrada_Video=Entrada_Video_Take=Entrada_Video_Take_Temp=-1;
	}
	ApagaLed(AV,YELLOW);
	switch(STATUS_AV)
	{
		case AUDIO:
			AcendeLed(AV,GREEN);
			break;
		case VIDEO:
			AcendeLed(AV,RED);
			break;
		case AUDIO_VIDEO:
			AcendeLed(AV,YELLOW);
			break;
	}
	if((STATUS_AV==AUDIO)||(STATUS_AV==AUDIO_VIDEO)) 
	{
		while(!UpdateAu(SAIDA))StackTask();
		while(!UpdateAu(SAIDA))StackTask();
	}
	if((STATUS_AV==VIDEO)||(STATUS_AV==AUDIO_VIDEO))
	{
		while(!UpdateVi(SAIDA))StackTask();
		while(!UpdateVi(SAIDA))StackTask();
	}	
	Entrada_Video_Temp = Entrada_Video;
	Entrada_Audio_Temp = Entrada_Audio;
}

/******************************************************************************
  NOME: void ProcessaEntradas(void)
  DESCRIÇÃO: Atualiza as Entradas.
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
void ProcessaEntradas(void)
{
	switch(STATUS_AV)
	{
		case AUDIO:
		{
			if(STATUS_TAKE == 1)
			{
				//Avaliar se será preciso verificar se tem comunicação matriz de video conectada, para poder fazer o take dos das duas ou somente uma
				//AUDIO_VIDEO
				if((Entrada_Video_Take != -1)&&(Entrada_Audio_Take != -1))
				{											
					Entrada_Apertada_Anterior = Entrada_Apertada;//tecla anterior recebe a tecla pressionada
					Entrada_Audio_Temp=Entrada_Audio;
					Entrada_Video_Temp=Entrada_Video;
					Entrada_Video = Entrada_Video_Take;//entrada de video recebe a entrada pressionada, esta funcionando audio follow video
					Entrada_Audio = Entrada_Audio_Take;//entrada de audio recebe a entrada pressionada, esta funcionando audio follow video
					ApagaLed(Entrada_Video, YELLOW);
					ApagaLed(Entrada_Audio, YELLOW);
					//Inicializa a flag com zero 
					flagVi = 0;
					flagAu = 0;					
					CONTA_PING_TIME_OUT=0;
					PING_AUDIO=FALSE;
					while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
					{
						StackTask();
						if(PingDemo(IP_MATRIZ_AUDIO))
						{
							PING_AUDIO=TRUE;
							CONTA_PING_TIME_OUT=PING_TIME_OUT;
						}
						else
						{
							DelayNano(4500);
							CONTA_PING_TIME_OUT++;
						}
					}
					CONTA_PING_TIME_OUT=0;
					PING_VIDEO=FALSE;
					while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
					{
						StackTask();
						if(PingDemo(IP_MATRIZ_VIDEO))
						{
							PING_VIDEO=TRUE;
							CONTA_PING_TIME_OUT=PING_TIME_OUT;
						}
						else
						{
							DelayNano(4500);
							CONTA_PING_TIME_OUT++;
						}
					}	
					if(PING_AUDIO)
					{
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
										   //Envia o comando Update a Matriz de Vídeo
							StackTask();
							SendETHAu();  
						}while(flagAu == 1);
						
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
											//Envia o comando Update a Matriz de Vídeo
							StackTask();
							SendETHAu(); 
						}while(flagAu == 1);
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,GREEN);
						DelayNano(2000);
						ApagaLed(TAKE, YELLOW);
						DelayNano(2000);
					}
					else
					{
						
						ApagaLed(Entrada_Audio, YELLOW);
						Entrada_Audio = Entrada_Audio_Temp;
						Entrada_Audio_Take = -1;
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,RED);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					
					if(PING_VIDEO)
					{
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
										   //Envia o comando Update a Matriz de Vídeo
							StackTask();
							SendETHVi();  
						}while(flagVi == 1);
						
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
											//Envia o comando Update a Matriz de Vídeo
							StackTask();
							SendETHVi(); 
						}while(flagVi == 1);
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,GREEN);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					else
					{
						ApagaLed(Entrada_Video, YELLOW);
						Entrada_Video = Entrada_Video_Temp;
						Entrada_Video_Take = -1;
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,RED);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					//Volta aos valores default
					if(!(PING_AUDIO&&PING_VIDEO))
					{
						STATUS_AV=-1;
						ApagaLed(AV,YELLOW);
						TestaAudioVideo();
					}
					Entrada_Audio_Take = -1;
					Entrada_Video_Take = -1;
					Entrada_Apertada = -1;		
					STATUS_TAKE = 0;
				}
				//AUDIO
				else if((Entrada_Audio_Take != -1))
				{
					Entrada_Audio_Temp = Entrada_Audio;
					Entrada_Audio = Entrada_Audio_Take;
					flagAu = 0;
					
					CONTA_PING_TIME_OUT=0;
					PING_AUDIO=FALSE;
					while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
					{
						StackTask();
						if(PingDemo(IP_MATRIZ_AUDIO))
						{
							PING_AUDIO=TRUE;
							CONTA_PING_TIME_OUT=PING_TIME_OUT;
							ICMPEndUsage();
							StackTask();
						}
						else
						{
							CONTA_PING_TIME_OUT++;
						}
					}	
					if(PING_AUDIO)
					{
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
							//Envia o comando Update a Matriz de Audio
							StackTask();
							SendETHAu();  
						}while(flagAu == 1);
						
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
							//Envia o comando Update a Matriz de Audio
							StackTask();
							SendETHAu(); 
						}while(flagAu == 1);
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,GREEN);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					else
					{	
						ApagaLed(Entrada_Audio, YELLOW);
						Entrada_Audio = Entrada_Audio_Temp;
						Entrada_Audio_Take = -1;
						STATUS_AV=-1;
						ApagaLed(AV,YELLOW);
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,RED);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					STATUS_TAKE = 0;
					Entrada_Audio_Take = -1;
				}
				//VIDEO
				else if((Entrada_Video_Take != -1))
				{
					Entrada_Video_Temp=Entrada_Video;
					Entrada_Video = Entrada_Video_Take;
					flagVi = 0;
					CONTA_PING_TIME_OUT=0;
					PING_VIDEO=FALSE;
					while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
					{
						StackTask();
						if(PingDemo(IP_MATRIZ_VIDEO))
						{
							PING_VIDEO=TRUE;
							CONTA_PING_TIME_OUT=PING_TIME_OUT;
							ICMPEndUsage();
							StackTask();
						}
						else
						{
							CONTA_PING_TIME_OUT++;
						}
					}	
					if(PING_VIDEO)
					{
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
						   //Envia o comando Update a Matriz de Vídeo
							StackTask();
							SendETHVi();  
						}while(flagVi == 1);
						
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
							//Envia o comando Update a Matriz de Vídeo
							StackTask();
							SendETHVi(); 
						}while(flagVi == 1);
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,GREEN);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					else
					{
						ApagaLed(Entrada_Video, YELLOW);
						Entrada_Video = Entrada_Video_Temp;
						Entrada_Video_Take = -1;
						STATUS_AV=-1;
						ApagaLed(AV,YELLOW);
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,RED);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					STATUS_TAKE = 0;	
					Entrada_Video_Take = -1;
				}
				STATUS_TAKE = 0;
			}	
			else if((Entrada_Apertada >=0)&&(Entrada_Apertada <=31))//Verifica se a tecla pressionada não é nem AV, nem CONFIG nem ENABLE 
			{
				if(Entrada_Apertada != Entrada_Apertada_Anterior)//verifica se a tecla pressionada e diferente da anterior
				{
					Entrada_Apertada_Anterior = Entrada_Apertada;
					Entrada_Audio_Take = Entrada_Apertada;	
					Entrada_Apertada = -1;
				}
			}
			break;
		}
		case VIDEO:
		{		
			if(STATUS_TAKE == 1)
			{
				//Avaliar se será preciso verificar se tem comunicação matriz de video conectada, para poder fazer o take dos das duas ou somente uma
				if((Entrada_Video_Take != -1)&&(Entrada_Audio_Take != -1))
				{
					Entrada_Video_Temp = Entrada_Video;
					Entrada_Audio_Temp = Entrada_Audio;
					Entrada_Apertada_Anterior = Entrada_Apertada;//tecla anterior recebe a tecla pressionada
					Entrada_Video = Entrada_Video_Take;//entrada de video recebe a entrada pressionada, esta funcionando audio follow video
					Entrada_Audio = Entrada_Audio_Take;//entrada de audio recebe a entrada pressionada, esta funcionando audio follow video
					ApagaLed(Entrada_Video, YELLOW);
					ApagaLed(Entrada_Audio, YELLOW);
					//Inicializa a flag com zero 
					flagVi = 0;
					flagAu = 0;
					CONTA_PING_TIME_OUT=0;
					PING_AUDIO=FALSE;
					while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
					{
						StackTask();
						if(PingDemo(IP_MATRIZ_AUDIO))
						{
							PING_AUDIO=TRUE;
							CONTA_PING_TIME_OUT=PING_TIME_OUT;
							ICMPEndUsage();
							StackTask();
						}
						else
						{
							CONTA_PING_TIME_OUT++;
						}
					}	
					CONTA_PING_TIME_OUT=0;
					PING_VIDEO=FALSE;
					while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
					{
						StackTask();
						if(PingDemo(IP_MATRIZ_VIDEO))
						{
							PING_VIDEO=TRUE;
							CONTA_PING_TIME_OUT=PING_TIME_OUT;
							ICMPEndUsage();
							StackTask();
						}
						else
						{
							CONTA_PING_TIME_OUT++;
						}
					}	
					if(PING_AUDIO)
					{
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
						   //Envia o comando Update a Matriz de Audio
							StackTask();
							SendETHAu();  
						}while(flagAu == 1);
						
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
							//Envia o comando Update a Matriz de Audio
							StackTask();
							SendETHAu(); 
						}while(flagAu == 1);
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,GREEN);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					else
					{
						ApagaLed(Entrada_Audio, YELLOW);
						Entrada_Audio = Entrada_Audio_Temp;
						Entrada_Audio_Take = -1;
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,RED);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					
					if(PING_VIDEO)
					{
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
						   //Envia o comando Update a Matriz de Vídeo
							StackTask();
							SendETHVi();  
						}while(flagVi == 1);
						
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
							//Envia o comando Update a Matriz de Vídeo
							StackTask();
							SendETHVi(); 
						}while(flagVi == 1);
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,GREEN);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					else
					{
						ApagaLed(Entrada_Video, YELLOW);
						Entrada_Video = Entrada_Video_Temp;
						Entrada_Video_Take = -1;
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,RED);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					//Volta aos valores default
					if(!(PING_AUDIO&&PING_VIDEO))
					{
						STATUS_AV=-1;
						ApagaLed(AV,YELLOW);
						TestaAudioVideo();
					}
					Entrada_Audio_Take = -1;
					Entrada_Video_Take = -1;
					Entrada_Apertada = -1;	
					STATUS_TAKE = 0;
					
				}
				//AUDIO
				else if((Entrada_Audio_Take != -1))
				{
					Entrada_Audio_Temp = Entrada_Audio;
					Entrada_Audio = Entrada_Audio_Take;
					flagAu = 0;
					
					CONTA_PING_TIME_OUT=0;
					PING_AUDIO=FALSE;
					while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
					{
						StackTask();
						if(PingDemo(IP_MATRIZ_AUDIO))
						{
							PING_AUDIO=TRUE;
							CONTA_PING_TIME_OUT=PING_TIME_OUT;
							ICMPEndUsage();
							StackTask();
						}
						else
						{
							CONTA_PING_TIME_OUT++;
						}
					}	
					if(PING_AUDIO)
					{
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
						   //Envia o comando Update a Matriz de Vídeo
							StackTask();
							SendETHAu();  
						}while(flagAu == 1);
						
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
							//Envia o comando Update a Matriz de Vídeo
							StackTask();
							SendETHAu(); 
						}while(flagAu == 1);
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,GREEN);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					else
					{
						ApagaLed(Entrada_Audio, YELLOW);
						Entrada_Audio = Entrada_Audio_Temp;
						Entrada_Audio_Take = -1;
						STATUS_AV=-1;
						ApagaLed(AV,YELLOW);
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,RED);
						DelayNano(200);
						ApagaLed(TAKE, YELLOW);
						DelayNano(200);
					}
					STATUS_TAKE = 0;
					Entrada_Audio_Take = -1;
				}
				//VIDEO
				else if((Entrada_Video_Take != -1))
				{
					Entrada_Video_Temp = Entrada_Video;
					Entrada_Video = Entrada_Video_Take;
					flagVi = 0;
					CONTA_PING_TIME_OUT=0;
					PING_VIDEO=FALSE;
					while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
					{
						StackTask();
						if(PingDemo(IP_MATRIZ_VIDEO))
						{
							PING_VIDEO=TRUE;
							CONTA_PING_TIME_OUT=PING_TIME_OUT;
							ICMPEndUsage();
							StackTask();
						}
						else
						{
							CONTA_PING_TIME_OUT++;
						}
					}	
					if(PING_VIDEO)
					{
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
						   //Envia o comando Update a Matriz de Vídeo
							StackTask();
							SendETHVi();  
						}while(flagVi == 1);
						
						do//Inserir uma segurança para caso entre em loop infinito se possa sair
						{
							//Envia o comando Update a Matriz de Vídeo
							StackTask();
							SendETHVi(); 
						}while(flagVi == 1);
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,GREEN);
						DelayNano(2000);
						ApagaLed(TAKE, YELLOW);
						DelayNano(2000);
					}
					else
					{
						ApagaLed(Entrada_Video, YELLOW);
						Entrada_Video = Entrada_Video_Temp;
						Entrada_Video_Take = -1;
						STATUS_AV=-1;
						ApagaLed(AV,YELLOW);
						ApagaLed(TAKE, YELLOW);
						AcendeLed(TAKE,RED);
						DelayNano(2000);
						ApagaLed(TAKE, YELLOW);
						DelayNano(2000);
					}
					STATUS_TAKE = 0;	
					Entrada_Video_Take = -1;
				}
			
				STATUS_TAKE = 0;
			}					
			else if((Entrada_Apertada >=0)&&(Entrada_Apertada <=31))//Verifica se a tecla pressionada não é nem AV, nem CONFIG nem ENABLE 
			{
				if(Entrada_Apertada != Entrada_Apertada_Anterior)//verifica se a tecla pressionada e diferente da anterior
				{
					Entrada_Apertada_Anterior = Entrada_Apertada;
					Entrada_Video_Take = Entrada_Apertada;
					Entrada_Apertada = -1;
				}
			}
			break;
		}
		case AUDIO_VIDEO:
		{
				if((Entrada_Apertada >=0)&&(Entrada_Apertada <=31))//Verifica se a tecla pressionada não é nem AV, nem CONFIG nem ENABLE
				{
					if(Entrada_Apertada != Entrada_Apertada_Anterior)//verifica se a tecla pressionad ae diferente da anterior
					{						
						Entrada_Apertada_Anterior = Entrada_Apertada;//tecla anterior recebe a tecla pressionada
						Entrada_Video_Temp = Entrada_Video;
						Entrada_Audio_Temp = Entrada_Audio;
						Entrada_Video =  Entrada_Apertada;//entrada de video recebe a entrada pressionada, esta funcionando audio follow video
						Entrada_Audio =  Entrada_Apertada;//entrada de audio recebe a entrada pressionada, esta funcionando audio follow video
						ApagaLed(Entrada_Video, YELLOW);
						ApagaLed(Entrada_Audio, YELLOW);
						//Inicializa a flag com zero 
						flagVi = 0;
						flagAu = 0;
						CONTA_PING_TIME_OUT=0;
						PING_AUDIO=FALSE;
						PING_VIDEO=FALSE;
						while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
						{
								StackTask();
								if(PingDemo(IP_MATRIZ_AUDIO)==TRUE)
								{
									PING_AUDIO=TRUE;
									CONTA_PING_TIME_OUT=PING_TIME_OUT + 1;
									ICMPEndUsage();
									StackTask();
								}
								CONTA_PING_TIME_OUT++;
						}
						//TESTA VIDEO TAMBÉM PARA CONCLUIR SE É POSSIVEL AUDIO_VIDEO
						CONTA_PING_TIME_OUT=0;	
						while(CONTA_PING_TIME_OUT<PING_TIME_OUT)
						{
								StackTask();
								if(PingDemo(IP_MATRIZ_VIDEO) == TRUE)
								{
									PING_VIDEO=TRUE;
									CONTA_PING_TIME_OUT=PING_TIME_OUT + 1;
									ICMPEndUsage();
									StackTask();
								}
								CONTA_PING_TIME_OUT++;
						}
						Contador_Time_Out_Comutacao = 0;
						if(!(PING_AUDIO&&PING_VIDEO))
						{
							PING_AUDIO=FALSE;
							PING_VIDEO=FALSE;
							TestaAudioVideo();
						}
						if(PING_AUDIO)
						{
							do//Inserir uma segurança para caso entre em loop infinito se possa sair
							{
							   //Envia o comando Update a Matriz de Vídeo
								Contador_Time_Out_Comutacao = Contador_Time_Out_Comutacao + 1;
								StackTask();
								SendETHAu();  
							}while((flagAu == 1)&&(Contador_Time_Out_Comutacao < 5000));
							Contador_Time_Out_Comutacao = 0;
							do//Inserir uma segurança para caso entre em loop infinito se possa sair
							{
							   //Envia o comando Update a Matriz de Vídeo
								Contador_Time_Out_Comutacao = Contador_Time_Out_Comutacao + 1;
								StackTask();
								SendETHAu();  
							}while((flagAu == 1)&&(Contador_Time_Out_Comutacao < 5000));
							
							ApagaLed(TAKE, YELLOW);
							
							AcendeLed(TAKE,GREEN);
							DelayNano(200);
							ApagaLed(TAKE, YELLOW);
							DelayNano(200);
						}
						else
						{
							ApagaLed(Entrada_Audio, YELLOW);
							Entrada_Audio = Entrada_Audio_Temp;
							Entrada_Audio_Take = -1;
							ApagaLed(TAKE, YELLOW);
							AcendeLed(TAKE,RED);
							DelayNano(2000);
							ApagaLed(TAKE, YELLOW);
							DelayNano(2000);
						}
						Contador_Time_Out_Comutacao = 0;
						if(PING_VIDEO)
						{
							Contador_Time_Out_Comutacao = 0;
							do//Inserir uma segurança para caso entre em loop infinito se possa sair
							{
							   //Envia o comando Update a Matriz de Vídeo
								Contador_Time_Out_Comutacao = Contador_Time_Out_Comutacao + 1;
								StackTask();
								SendETHVi();  
							}while((flagVi == 1)&&(Contador_Time_Out_Comutacao < 5000));
							
							Contador_Time_Out_Comutacao = 0;
							
							do//Inserir uma segurança para caso entre em loop infinito se possa sair
							{
							    //Envia o comando Update a Matriz de Vídeo
								Contador_Time_Out_Comutacao = Contador_Time_Out_Comutacao + 1;
								StackTask();
								SendETHVi();  
							}while((flagVi == 1));
							
							ApagaLed(TAKE, YELLOW);
							AcendeLed(TAKE,GREEN);
							DelayNano(200);
							ApagaLed(TAKE, YELLOW);
							DelayNano(200);
						}
						else
						{
							ApagaLed(Entrada_Video, YELLOW);
							Entrada_Video = Entrada_Video_Temp;
							Entrada_Video_Take = -1;
							ApagaLed(TAKE, YELLOW);
							AcendeLed(TAKE,RED);
							DelayNano(2000);
							ApagaLed(TAKE, YELLOW);
							DelayNano(2000);
						}
						//Volta aos valores default
						Entrada_Audio_Take = -1;
						Entrada_Video_Take = -1;
						Entrada_Apertada = -1;						
						
					}
				}	
														
			break;
		}	
	}

}

/******************************************************************************
  NOME: void InitializeBoard(void)
  DESCRIÇÃO: Initialize board specific hardware.
  PARÂMETROS: nenhum
  ALGORITIMO:  nenhum
  NOTAS:  nenhum
*******************************************************************************/
static void InitializeBoard(void)
{

//_NVMOP=0b1111;
// Configurações dos Registradores ---------------------------------------------
   // Configuração das direções das portas
           //FEDCBA9876543210             Entradas = 1   Saidas = 0   
   TRISB = 0b0010000010100000;
   TRISC = 0b0000000000000000;
   TRISD = 0b0000111100000000;
   TRISF = 0b0000000001001000;
   TRISG = 0b1000000011000000;
   
   // Pull ups desabilitados
   ODCD  = 0;
   ODCF  = 0;
   ODCG  = 0;
      
   // Todos os pinos com função analógica como digital
   _ADON    = 0;
   AD1PCFGH = 0xFFFF;
   AD1PCFGL = 0xFFFF;
   
   // Crank up the core frequency
   PLLFBD = 38;      // Multiply by 40 for 160MHz VCO output (8MHz XT oscillator)
   CLKDIV = 0x0000;  // FRC: divide by 2, PLLPOST: divide by 2, PLLPRE: divide by 2
   
// Configurações das Interrupções ----------------------------------------------
   // Configurações Gerais
   //_NSTDIS = 0;   // Nested Interruptions Disabled   
  
   // External Interrupt 0 - Interrupção TAKE
   _INT0IP0 = 0;  // Bit 0       //Priority bits (highest = 7, lowest = 1, default = 4)
   _INT0IP1 = 0;  // Bit 1
   _INT0IP2 = 1;  // Bit 2
   
   _INT0EP = 1;   // External Interrupt 0 Edge Detect Polarity Select bit
                  // 0 = Positive Edge
                  // 1 = Negative Edge
   
   _INT0IF = 0;   // External Interrupt 0 Flag Status bit
                  // 0 = Interrupt request has not occurred
                  // 1 = Interrupt request has occurred
   
   _INT0IE = 0;   // External Interrupt 0 Enable bit
                  // 0 = Interrupt request not enabled
                  // 1 = Interrupt request enabled

   //External Interrupt 1 - Interrupção ÁUDIO/VÍDEO
   _INT1IP0 = 0; 
   _INT1IP1 = 1;   
   _INT1IP2 = 0;   
   
   _INT1EP = 1;   
   
   _INT1IF = 0;   
   
   _INT1IE = 0;

   //External Interrupt 2 - Interrupção ENABLE
   _INT2IP0 = 1;  // Maior Prioridade
   _INT2IP1 = 1;   
   _INT2IP2 = 1;   
   
   _INT2EP = 1;   
   
   _INT2IF = 0;   
   
   _INT2IE = 0;   
   
// Configuração do Timer  ------------------------------------------------------           
           
   // Timer 2 - Pisca LEDs na rotina de configuração de saída (Conf_Saida)        

   T2CONbits.TON = 0;               // Timer2 desligado
   TMR2 = 0;                        // Zera o timer para início de contagem
   PR2 = 3662;                      // Configura o registrador de período 
                                    // Tempo = 75ms
                                    // PR9 = (Fosc * Tempo)/(2 * PS)
                                    // PR9 = (25000000 * 0,075)/(2 * 256)
                                    // PR9 = 3662
   
   T2CONbits.TCS = 0;               // Modo timer (clock interno)
   T2CONbits.TGATE = 0;
   
   T2CONbits.TSIDL = 0;             // Timer ligado em modo Idle    
   T2CONbits.TCKPS = 3;             // Prescaler 1:256    
   
   IFS0bits.T2IF = 0;               // Limpa o flag

   // Timer 3 - Verifica se o botão Enable está apertado     

   T3CONbits.TON = 0;               // Timer3 desligado
   TMR3 = 0;                        // Zera o timer para início de contagem
   PR3 = 6104;                      // Configura o registrador de período 
                                    // Tempo = 250ms
                                    // PR3 = (Fosc * Tempo)/(2 * PS)
                                    // PR3 = (25000000 * 0,125)/(2 * 256)
                                    // PR3 = 3662
   
   T3CONbits.TCS = 0;               // Modo timer (clock interno)
   T3CONbits.TGATE = 0;
   
   T3CONbits.TSIDL = 0;             // Timer ligado em modo Idle    
   T3CONbits.TCKPS = 3;             // Prescaler 1:256    
   
   IFS0bits.T3IF = 0;               // Limpa o flag
   
   
   // Timer 4 - O remote busca na matriz o valor de entrada das suas saídas controladas       
        
   T4CONbits.TON = 0;               // Timer4 desligado
   TMR4 = 0;                        // Zera o timer para início de contagem
   PR4 = 24414;                     // Configura o registrador de período 
                                    // Tempo = 500ms
                                    // PR4 = (Fosc * Tempo)/(2 * PS)
                                    // PR4 = (25000000 * 0,500)/(2 * 256)
                                    // PR4 = 24414
   
   T4CONbits.TCS = 0;               // Modo timer (clock interno)
   T4CONbits.TGATE = 0;
   
   T4CONbits.TSIDL = 0;             // Timer ligado em modo Idle    
   T4CONbits.TCKPS = 3;             // Prescaler 1:256    

   IFS1bits.T4IF = 0;               // Limpa o flag
 

   // Timer 6 - Tempo limite de resposta da matriz       
        
   T6CONbits.TON = 0;               // Timer6 desligado
   TMR6 = 0;                        // Zera o timer para início de contagem
   PR6 = 48828;//977; //20ms                      // Configura o registrador de período 
                                    // Tempo = 100ms
                                    // PR6 = (Fosc * Tempo)/(2 * PS)
                                    // PR6 = (25000000 * 0,100)/(2 * 256)
                                    // PR6 = 4882,8125
   
   T6CONbits.TCS = 0;               // Modo timer (clock interno)
   T6CONbits.TGATE = 0;
   
   T6CONbits.TSIDL = 0;             // Timer ligado em modo Idle    
   T6CONbits.TCKPS = 3;             // Prescaler 1:256    

   IFS2bits.T6IF = 0;               // Limpa o flag 

 
   // Timer 9 - Desabilita o remote quando está ocioso
   
   T9CONbits.TON = 0;               // Timer9 desligado
   TMR9 = 0;                        // Zera o timer para início de contagem
   PR9 = 24414;                     // Configura o registrador de período 
                                    // Tempo = 500ms
                                    // PR9 = (Fosc * Tempo)/(2 * PS)
                                    // PR9 = (25000000 * 0,500)/(2 * 256)
                                    // PR9 = 24414
   
   T9CONbits.TCS = 0;               // Modo timer (clock interno)
   T9CONbits.TGATE = 0;
   
   T9CONbits.TSIDL = 0;             // Timer ligado em modo Idle    
   T9CONbits.TCKPS = 3;             // Prescaler 1:256    

   IFS3bits.T9IF = 0;               // Limpa o flag       
}

/******************************************************************************
  NOME: void InitAppConfig(void)
  DESCRIÇÃO: Initialize board specific hardware.
  PARÂMETROS: 
  RETORNO: Write/Read non-volatile config variables.
  NOTAS:  nenhum
*******************************************************************************/
// Uncomment these two pragmas for production MAC address 
// serialization if using C18. The MACROM=0x1FFF0 statement causes 
// the MAC address to be located at aboslute program memory address 
// 0x1FFF0 for easy auto-increment without recompiling the stack for 
// each device made.  Note, other compilers/linkers use a different 
// means of allocating variables at an absolute address.  Check your 
// compiler documentation for the right method.
//#pragma romdata MACROM=0x1FFF0

static ROM BYTE SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};
//#pragma romdata

static void InitAppConfig(void)
{

#if (defined(MPFS_USE_EEPROM) || defined(MPFS_USE_SPI_FLASH)) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
    BYTE c;
    BYTE *p;
#endif

   AppConfig.Flags.bIsDHCPEnabled = TRUE;
   AppConfig.Flags.bInConfigMode = TRUE;
   memcpypgm2ram((void*)&AppConfig.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));
//   {
//      _prog_addressT MACAddressAddress;
//      MACAddressAddress.next = 0x157F8;
//      _memcpy_p2d24((char*)&AppConfig.MyMACAddr, MACAddressAddress, sizeof(AppConfig.MyMACAddr));
//   }
   AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
   AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
   AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
   AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
   AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
   AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
   AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;

   // Load the default NetBIOS Host Name
   memcpypgm2ram(AppConfig.NetBIOSName, (ROM void*)MY_DEFAULT_HOST_NAME, 16);
   FormatNetBIOSName(AppConfig.NetBIOSName);

#if defined(MPFS_USE_EEPROM) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
    p = (BYTE*)&AppConfig;

    XEEBeginRead(0x0000);
    c = XEERead();
    XEEEndRead();

    // When a record is saved, first byte is written as 0x60 to indicate
    // that a valid record was saved.  Note that older stack versions 
   // used 0x57.  This change has been made to so old EEPROM contents 
   // will get overwritten.  The AppConfig() structure has been changed,
   // resulting in parameter misalignment if still using old EEPROM 
   // contents.
    if(c == 0x60u)
    {
        XEEBeginRead(0x0001);
        for ( c = 0; c < sizeof(AppConfig); c++ )
            *p++ = XEERead();
        XEEEndRead();
    }
    else
        SaveAppConfig();

#elif defined(MPFS_USE_SPI_FLASH) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
   SPIFlashReadArray(0x00, &c, 1);
   if(c == 0x60u)
      SPIFlashReadArray(0x01, (BYTE*)&AppConfig, sizeof(AppConfig));
   else
      SaveAppConfig();
#endif

}

#if (defined(MPFS_USE_EEPROM) || defined(MPFS_USE_SPI_FLASH)) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
void SaveAppConfig(void)
{
   #if defined(MPFS_USE_EEPROM)

    BYTE c;
    BYTE *p;

    p = (BYTE*)&AppConfig;
    XEEBeginWrite(0x0000);
    XEEWrite(0x60);
    for ( c = 0; c < sizeof(AppConfig); c++ )
    {
        XEEWrite(*p++);
    }

    XEEEndWrite();
    
    #else

    SPIFlashBeginWrite(0x0000);
    SPIFlashWrite(0x60);
    SPIFlashWriteArray((BYTE*)&AppConfig, sizeof(AppConfig));
    
    #endif
}
#endif

