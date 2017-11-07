/*******************************************************************************
'*  EMPRESA    : DATASINC                                                      *
'*  DESCRIÇÃO  : Funções de comunicação                                        *
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

//#include "Include/TCPIP Stack/ICMP.h"
/*------------------------------ Protótipos ----------------------------------*/
void SendETHVi(void);                 // Envia comando de escrita via ethernet a matriz de vídeo
void SendETHAu(void);               // Envia comando de escrita via ethernet para a matriz de áudio
BOOL UpdateVi(unsigned char saidaa);  // Envia comando de atualização via ethernet a matriz de vídeo
BOOL UpdateAu(unsigned char saidaa);// Envia comando de atualização via ethernet a matriz de áudio
BOOL PingDemo(IP_ADDR MatrizIP);



/*******************************************************************************
  NOME: SendETH
  DESCRIÇÃO: Envia comando de escrita via ethernet a matriz de vídeo
  PARÂMETROS: nenhum
  RETORNO: nenhum
  NOTAS:  nenhum
*******************************************************************************/ 
void SendETHVi()
{ 
/*
   Sequencia de comandos - contador: conta
   | > | @wvxxyy*;            envia um comando de escrita a matriz de vídeo
   | < | @d******;            recebe o comando de pronto da matriz
*/
   static TICK         Timer;
   static TCP_SOCKET   MySocket = INVALID_SOCKET;
   static enum _SendEthState
   {
      SM_HOME = 0,
      SM_SOCKET_OBTAINED,
      SM_PROCESS_RESPONSE,
      SM_DISCONNECT
   } SendEthState = SM_HOME;
   
   switch(SendEthState)
   {
      case SM_HOME:
         // Connect a socket to the remote TCP server
         MySocket = TCPOpen((DWORD)USE_REMOTE_TCP_SERVER, TCP_OPEN_ROM_HOST, ServerPort, TCP_PURPOSE_GENERIC_TCP_CLIENT);
         flagVi = 1;
         
         if(MySocket == INVALID_SOCKET)
         {   
	        flagVi = 0;     
            break;  
         }          

         SendEthState++;
         Timer = TickGet();
                 
         break;

      case SM_SOCKET_OBTAINED:
  
         if(!TCPIsConnected(MySocket))
         {
            if(IFS2bits.T6IF)
            {
               // Close the socket so it can be used by other modules
               TCPDisconnect(MySocket);
               MySocket = INVALID_SOCKET;
               SendEthState = SM_DISCONNECT;
               //Substituido por Entrada_Audio e Entrada_Video
               //for(i=0;i<9;i++)
               //{
               //   ColVi[contsai][i]=0b1111;
               //}
            }
            break;
         }
         
      
         SendEthState++;
         conta=0;
         break;
         
      case SM_PROCESS_RESPONSE:
                  
         if(conta==0)
         {
             //Envia o pedido de atualização
             
             
            t[0]='@';
            t[1]='w';         //Pedido de atualização para a saída comandada
            t[2]='v';            
         
            
			if(Entrada_Video >= 10)
			{
				t[3]= (SAIDA/10) + 48;            
				t[4]= (SAIDA%10) + 48;
				t[5]= (Entrada_Video/10) + 48;                    
				t[6]= (Entrada_Video%10) + 48;       
			}	
			else
			{
				t[3]=  48;            
				t[4]= SAIDA + 48;
				t[5]=  48;                    
				t[6]= Entrada_Video + 48;  
			}
            t[7]= '*';        //Futuramente esse bit irá sinalizar presença do vídeo na saída
            t[8]= ';';        //Sinaliza o final da string
            t[9]= '\r';       //Carriage Return
            
			/*
			t[0]='@';
            t[1]='w';         //Pedido de atualização para a saída comandada
            t[2]='v';            
            t[3]= '0';            
            t[4]= '0';            
            t[5]= '3';                    
            t[6]= '1';                     
            t[7]= '*';        //Futuramente esse bit irá sinalizar presença do vídeo na saída
            t[8]= ';';        //Sinaliza o final da string
            t[9]= '\r';       //Carriage Return
            */
            TCPPutArray(MySocket, t, 10);
            TCPFlush(MySocket);
            conta=1;
         }                                                      
		//Analisar este ponto pois a matriz responde @b**** e depois que envia @d****, analisar se isto não atrapalhará o funcionamento
         if(TCPIsGetReady(MySocket))   // Verifica se chegou algum comando novo
         {
            TCPGetArray(MySocket, s, 10);    // Leitura do novo comando
            if (s[0]=='@')
            {
               switch (s[1])
               {
                  case 'd':                  // Comando de leitura
                     SendEthState++;
                     conta=2;
                  break;
               }
            }
         }

            break;
 
      case SM_DISCONNECT:
         TCPDisconnect(MySocket);
         MySocket = INVALID_SOCKET;
         SendEthState = SM_HOME;
         if(conta==2)   flagVi=0;
         flagSend=0;
         break;
   }
}

/*******************************************************************************
  NOME: SendETHAu
  DESCRIÇÃO: Envia comando de escrita via ethernet para a matriz de áudio
  PARÂMETROS: nenhum
  RETORNO: nenhum
  NOTAS: nenhum
*******************************************************************************/ 
void SendETHAu(void)
{
/*
   Sequencia de comandos - contador: conta
   | > | @waxxyy*;            envia um comando de escrita a matriz de áudio
   | < | @d******;            recebe o comando de pronto da matriz
*/   
   static TICK         Timer;
   static TCP_SOCKET   MySocket = INVALID_SOCKET;
   static enum _SendEthStateAu
   {
      SM_HOME = 0,
      SM_SOCKET_OBTAINED,
      SM_PROCESS_RESPONSE,
      SM_DISCONNECT
   } SendEthStateAu = SM_HOME;
   
   switch(SendEthStateAu)
   {
      case SM_HOME:
         // Connect a socket to the remote TCP server
         MySocket = TCPOpen((DWORD)USE_REMOTE_TCP_SERVER_AU, TCP_OPEN_ROM_HOST, ServerPort_AU, TCP_PURPOSE_GENERIC_TCP_CLIENT);
         flagAu = 1;
         if(MySocket == INVALID_SOCKET)
         {
          	flagAu = 0;
            break;
         }   

         SendEthStateAu++;
         Timer = TickGet();

         break;

      case SM_SOCKET_OBTAINED:        
         if(!TCPIsConnected(MySocket))
         {
            if(IFS2bits.T6IF)
            {
               // Close the socket so it can be used by other modules
               TCPDisconnect(MySocket);
               MySocket = INVALID_SOCKET;
               SendEthStateAu = SM_DISCONNECT;
               //for(i=0;i<9;i++)
               //{
               //   ColAu[contsai][i]=0b1111;
               //}
            }
            break;
         }
         
         SendEthStateAu++;
         conta=0;
         break;
         
      case SM_PROCESS_RESPONSE:
                  
         if(conta==0)
         {
             //Envia o pedido de atualização
            entAu[contsai]=ent[contsai]; 
            t[0]='@';
            t[1]='w';         //Pedido de atualização para a saída comandada
            t[2]='a';            
   
			
             
			if(Entrada_Audio >= 10)
			{
				t[3]= (SAIDA/10) + 48;            
				t[4]= (SAIDA%10) + 48;   
				t[5]= (Entrada_Audio/10) + 48;                    
				t[6]= (Entrada_Audio%10) + 48;       
			}	
			else
			{
				t[3]= 48;            
				t[4]= SAIDA + 48;   
				t[5]=  48;                    
				t[6]= Entrada_Audio + 48;  
			}          
            t[7]= '*';        //Futuramente esse bit irá sinalizar presença do vídeo na saída
            t[8]= ';';        //Sinaliza o final da string
            t[9]= '\r';       //Carriage Return
            TCPPutArray(MySocket, t, 10);
            TCPFlush(MySocket);
            conta=1;
         }                                                      

         if(TCPIsGetReady(MySocket))   // Verifica se chegou algum comando novo
         {
            TCPGetArray(MySocket, s, 10);    // Leitura do novo comando
            if (s[0]=='@')
            {
               switch (s[1])
               {
                  case 'd':                  // Comando de leitura
                     SendEthStateAu++;
                     conta=2;
                  break;
               }
            }
         }

            break;
 
      case SM_DISCONNECT:
         TCPDisconnect(MySocket);
         MySocket = INVALID_SOCKET;
         SendEthStateAu = SM_HOME;                
         if(conta==2)   flagAu=0;
         flagSendAu=0;
         break;
   }
}
/*******************************************************************************
  NOME: UpdateVi
  DESCRIÇÃO: Envia comando de atualização via ethernet a matriz de vídeo
  PARÂMETROS: saidaa (número da saida que se deseja atualizar)
  RETORNO: nenhum
  NOTAS:  nenhum
*******************************************************************************/ 
BOOL UpdateVi(unsigned char saidaa)
{
/*
   Sequencia de comandos - contador: contup
   | > | @uvxx***;            envia um pedido de atualização a matriz de vídeo
   | < | @wvxxyy*;            recebe um comando de escrita de vídeo da matriz
*/
   BOOL success = FALSE;
   unsigned char j;
   
   static TICK         Timer;
   static TCP_SOCKET   MySocket = INVALID_SOCKET;
   unsigned long long conta_led,conta_dly;
   static enum _UpdateState
   {
      SM_HOME = 0,
      SM_SOCKET_OBTAINED,
      SM_PROCESS_RESPONSE,
      SM_DISCONNECT
   } UpdateState = SM_HOME;
      
   switch(UpdateState)
   {
      case SM_HOME:
         // Connect a socket to the remote TCP server
         MySocket = TCPOpen((DWORD)USE_REMOTE_TCP_SERVER, TCP_OPEN_ROM_HOST, ServerPort, TCP_PURPOSE_GENERIC_TCP_CLIENT);
                  
         // Abort operation if no TCP socket of type TCP_PURPOSE_GENERIC_TCP_CLIENT is available
         // If this ever happens, you need to go add one to TCPIPConfig.h
         
         if(MySocket == INVALID_SOCKET)
		 {
            break;
		 }

         UpdateState++;
         Timer = TickGet();

         break;

      case SM_SOCKET_OBTAINED:
         // Wait for the remote server to accept our connection request
         
         if(!TCPIsConnected(MySocket))
         {
            if(IFS2bits.T6IF)
            {
               // Close the socket so it can be used by other modules
               TCPDisconnect(MySocket);
               MySocket = INVALID_SOCKET;
               UpdateState = SM_DISCONNECT;
               /*for(i=0;i<9;i++)
               {
                  ColVi[contsai][i]=0b1111;
               }*/               
            }
            break;
         }
         
         UpdateState++;
         contup=0;
         break;

      case SM_PROCESS_RESPONSE:
                  
         if(contup==0)
         {
             //Envia o pedido de atualização
            t[0]='@';
            t[1]='u';         //Pedido de atualização para a saída comandada
            t[2]='v';      
            t[3] = (saidaa/10) + 48;   // Codifica a DEZENA correspondente à saída
                              // divindo o valor por 10 e somando 48 ao resultado 
                              // INTEIRO de modo a corresponder ao caracter ASCII
                              // correto
            t[4] = (saidaa%10) + 48;   // Codifica a UNIDADE correspondente à saída
                              // divindo o valor por 10 e somando 48 ao RESTO do
                              // resultado de modo a corresponder ao caracter ASCII
                              // correto
            t[5]='*';
            t[6]='*';
            t[7]='*';
            t[8]=';';
            t[9]='\r';
			
		
            TCPPutArray(MySocket, t, 10);
            TCPFlush(MySocket);
            contup=1;
         }                                                      

         if(TCPIsGetReady(MySocket))   // Verifica se chegou algum comando novo
         {
            TCPGetArray(MySocket, s, 10);    // Leitura do novo comando
            if (s[0]=='@')
            {
               switch (s[1])
               {
                  case 'w':                  // Comando de leitura
                     j=0;
                     j=(s[5]-48)*10;         // Decodifica qual é a entrada a ser controlada
                     j=(s[6]-48)+j;
                     switch (s[2])
                     {
                        case 'v':
						   Entrada_Video = j;                           
                           UpdateState++; 
                           break;
                     }
                  break;
               }
            }
         }
         
         break;


      case SM_DISCONNECT:
         TCPDisconnect(MySocket);
         MySocket = INVALID_SOCKET;
         UpdateState = SM_HOME;
         flagVi=1;
         contup=0;
         flagupdate=0;
		 success=TRUE;
         break;
   }
   return success;
}
/*******************************************************************************
  NOME: UpdateAu
  DESCRIÇÃO: Envia comando de atualização via ethernet
  PARÂMETROS: saidaa (número da saida que se deseja atualizar)
  RETORNO: nenhum
  NOTAS:  nenhum
*******************************************************************************/ 
BOOL UpdateAu(unsigned char saidaa)
{
/*
   Sequencia de comandos - contador: contup
   | > | @uaxx***;            envia um pedido de atualização a matriz de áudio
   | < | @waxxyy*;            recebe um comando de escrita de áudio da matriz
*/   
   BOOL success=FALSE;
   unsigned char j;
   
   static TICK         Timer;
   static TCP_SOCKET   MySocket = INVALID_SOCKET;
   static enum _UpdateStateAu
   {
      SM_HOME = 0,
      SM_SOCKET_OBTAINED,
      SM_PROCESS_RESPONSE,
      SM_DISCONNECT
   } UpdateStateAu = SM_HOME;
      
   switch(UpdateStateAu)
   {
      case SM_HOME:
         // Connect a socket to the remote TCP server
         MySocket = TCPOpen((DWORD)USE_REMOTE_TCP_SERVER_AU, TCP_OPEN_ROM_HOST, ServerPort_AU, TCP_PURPOSE_GENERIC_TCP_CLIENT);
         
         // Abort operation if no TCP socket of type TCP_PURPOSE_GENERIC_TCP_CLIENT is available
         // If this ever happens, you need to go add one to TCPIPConfig.h
         if(MySocket == INVALID_SOCKET)
		 {
            break;
		 }
         UpdateStateAu++;
         Timer = TickGet();

         break;

      case SM_SOCKET_OBTAINED:
         // Wait for the remote server to accept our connection request
         
         if(!TCPIsConnected(MySocket))
         {
            if(IFS2bits.T6IF)
            {
               TCPDisconnect(MySocket);
               MySocket = INVALID_SOCKET;
               UpdateStateAu = SM_DISCONNECT;
               /*for(i=0;i<9;i++)
               {
                  ColAu[contsai][i]=0b1111;
               } */                       
            }
            break;
         }
         
         UpdateStateAu++;
         contup=0;
         break;

      case SM_PROCESS_RESPONSE:
                  
         if(contup==0)
         {
            //Envia o pedido de atualização
            t[0]='@';
            t[1]='u';         //Pedido de atualização para a saída comandada
            t[2]='a';      
            t[3] = (saidaa/10) + 48;   // Codifica a DEZENA correspondente à saída
                              // divindo o valor por 10 e somando 48 ao resultado
                              // INTEIRO de modo a corresponder ao caracter ASCII
                              // correto
            t[4] = (saidaa%10) + 48;   // Codifica a UNIDADE correspondente à saída
                              // divindo o valor por 10 e somando 48 ao RESTO do
                              // resultado de modo a corresponder ao caracter ASCII
                              // correto
            t[5]='*';
            t[6]='*';
            t[7]='*';
            t[8]=';';
            t[9]='\r';
            TCPPutArray(MySocket, t, 10);
            TCPFlush(MySocket);
            contup=1;
         }

         if(TCPIsGetReady(MySocket))   // Verifica se chegou algum comando novo
         {
            TCPGetArray(MySocket, s, 10);    // Leitura do novo comando
            if (s[0]=='@')
            {
               switch (s[1])
               {
                  case 'w':                  // Comando de leitura
                     j=0;
                     j=(s[5]-48)*10;         // Decodifica qual é a entrada a ser controlada
                     j=(s[6]-48)+j;
                     switch (s[2])
                     {
                        case 'a':
                           Entrada_Audio = j;
                           UpdateStateAu++;   
                           break;
                     }
                  break;
               } 
            }
         }
         break;
  
         
      case SM_DISCONNECT:
         TCPDisconnect(MySocket);
         MySocket = INVALID_SOCKET;
         UpdateStateAu = SM_HOME;
         flagAu=1;
         contup=0;
         flagupdateAu=0;
		 success=TRUE;
         break;         
   }
   return success;
}

/*
//TESTE DO PING:
	ACENDE_LED(TAKE,YELLOW);
	delay(2);
	APAGA_LED(TAKE,YELLOW);
	while(!PingDemo(IP_MATRIZ_VIDEO))
	{
		StackTask();
		ACENDE_LED(TAKE,RED);
	}
	APAGA_LED(TAKE,YELLOW);
	ACENDE_LED(TAKE,GREEN);		
*/
						
BOOL PingDemo(IP_ADDR MatrizIP) 
{ 
	BOOL success=FALSE;
	static enum 
	{ 
		SM_HOME = 0, 
		SM_GET_RESPONSE 
	} PingState = SM_HOME; 
	static TICK Timer; 
	SHORT ret; 
	switch(PingState) 
	{ 
		case SM_HOME: 
		{
			// Obtain ownership of the ICMP module 
			if(ICMPBeginUsage()) 
			{ 
				Timer = TickGet(); 
				PingState = SM_GET_RESPONSE; 

				/* Send the ping request to 4.78.194.159 (ww1.microchip.com) 
				MatrizIP.v[0] = 192; 
				MatrizIP.v[1] = 168; 
				MatrizIP.v[2] = 100; 
				MatrizIP.v[3] = 50; */
				ICMPSendPing(MatrizIP.Val); 
			} 
			break; 
		}
		case SM_GET_RESPONSE:
		{
			// Get the status of the ICMP module 
			ret = ICMPGetReply();      
			if(ret == -2) 
			{ 
				// Do nothing: still waiting for echo 
				break; 
			} 
			else if(ret == -1) 
			{ 
				// Request timed out 
				#if defined(USE_LCD) 
				memcpypgm2ram((void*)&LCDText[16], (ROM void *)"Ping timed out", 15); 
				LCDUpdate(); 
				#endif 
				PingState = SM_HOME; 
			} 
			else 
			{ 
				// Echo received.  Time elapsed is stored in ret (units of TICK). 
				#if defined(USE_LCD) 
				memcpypgm2ram((void*)&LCDText[16], (ROM void *)"Reply: ", 7); 
				uitoa(ret, &LCDText[16+7]); 
				strcatpgm2ram((char*)&LCDText[16+7], (ROM char*)"ms"); 
				LCDUpdate(); 
				#endif 
				PingState = SM_HOME; 
				success=TRUE;
			} 

			// Finished with the ICMP module, release it so other apps can begin using it 
			ICMPEndUsage(); 
			break; 
		}
	}
	return success;
} 



