//############################################### ##########################
// Descrição:
//! \addtogroup f2806x_example_list
//! <h1> Início da conversão ADC (adc_soc)</h1>
//!
//! Este exemplo de ADC usa ePWM1 para gerar um ADC SOC - ADCINT1 periódico.
//! Dois canais são convertidos, ADCINA4 e ADCINA2.
//!
//! \b Observar \b Variáveis ​​\n
//! -
// Tensão1[10] - Últimos 10 valores ADCRESULT0
//! - Tensão2[10] - Últimos 10 valores ADCRESULT1
//! - ConversionCount - Número do resultado atual 0-9
//! - LoopCount - Contador de loop ocioso
//
//
//############################################### ##########################
// Versão $TI: Arquivos de cabeçalho C/C++ F2806x e exemplos de periféricos V151 $
// $Data de lançamento: 2 de fevereiro de 2016 $
// $Copyright: Copyright (C) 2011-2016 Texas Instruments Incorporated -
// http://www.ti.com/ TODOS OS DIREITOS RESERVADOS $
//############################################### ##########################

#include "DSP28x_Project.h"                 // Arquivo de cabeçalho do dispositivo e exemplos incluem arquivo
                                            //
                                            // Protótipo de instruções para funções encontradas neste arquivo.
__interrupt void adc_isr(void);             //
void Adc_Config(void);                      //
                                            //
                                            //
                                            // Variáveis ​​globais usadas neste exemplo:
Uint16 LoopCount;                           //
Uint16 ConversionCount;                     //
Uint16 Voltage1[10];                        //
Uint16 Voltage2[10];                        //
                                            //
                                            //
main()                                      //
{                                           //
                                            //
                                            // Etapa 1. Inicializar o controle do sistema:
                                            // PLL, WatchDog, habilita relógios periféricos
                                            // Este exemplo de função é encontrado no arquivo F2806x_SysCtrl.c.
   InitSysCtrl();                           //
                                            //
                                            //
                                            // Etapa 2. Inicializar GPIO:
                                            // Este exemplo de função é encontrado no arquivo F2806x_Gpio.c e
                                            // ilustra como definir o GPIO para seu estado padrão.
                                            // InitGpio(); // Ignorado neste exemplo
                                            //
                                            // Etapa 3. Limpe todas as interrupções e inicialize a tabela de vetores PIE:
                                            // Desabilita interrupções de CPU
   DINT;                                    //
                                            //
                                            // Inicializa os registradores de controle PIE em seu estado padrão.
                                            // O estado padrão é todas as interrupções PIE desabilitadas e flags
                                            // são limpos.
                                            // Esta função é encontrada no arquivo F2806x_PieCtrl.c.
   InitPieCtrl();                           // organiza prioridade de interrupcao
                                           //
                                            // Desativa as interrupções da CPU e limpa todos os flags de interrupção da CPU:
   IER = 0x0000;                            //
   IFR = 0x0000;                            //
                                            //
                                            // Inicializa a tabela de vetores PIE com ponteiros para o shell Interrupt
                                         // Rotinas de Atendimento (ISR).
                                            // Isso preencherá toda a tabela, mesmo que a interrupção
                                            // não é usado neste exemplo. Isso é útil para fins de depuração.
                                            // As rotinas ISR do shell são encontradas em F2806x_DefaultIsr.c.
                                            // Esta função é encontrada em F2806x_PieVect.c.
InitPieVectTable();                         //
                                            //
                                            // As interrupções usadas neste exemplo são mapeadas novamente para
                                            // Funções ISR encontradas neste arquivo.
   EALLOW;                                  // Isto é necessário para escrever no registro protegido EALLOW
   PieVectTable.ADCINT1 = &adc_isr;            // interrupcao associada ao endere�o adc_isr
   EDIS;                                    // Isso é necessário para desabilitar a gravação em registros protegidos por EALLOW
                                            //
                                            // Etapa 4. Inicialize todos os periféricos do dispositivo:
                                            // Esta função é encontrada em F2806x_InitPeripherals.c
                                            // InitPeripherals(); //Não é necessário para este exemplo
   InitAdc();                               // Para este exemplo, inicialize o ADC
   AdcOffsetSelfCal();                      //
                                            //
                                            // Etapa 5. Código específico do usuário, habilite interrupções:
                                            //
                                            // Habilita ADCINT1 no PIE
   PieCtrlRegs.PIEIER1.bit.INTx1 = 1;       // Habilita INT 1.1 no PIE
   IER |= M_INT1;                           // Habilita interrupção de CPU 1
   EINT;                                    // Habilita interrupção global INTM
   ERTM;                                    // Habilita DBGM de interrupção global em tempo real
                                            //
   LoopCount = 0;                           //
   ConversionCount = 0;                     //
                                            //
                                            // Configura o ADC
    EALLOW;                                 //
    AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1;  // Ativar modo sem sobreposição
    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;    // ADCINT1 desarma após a trava do AdcResults
    AdcRegs.INTSEL1N2.bit.INT1E     = 1;    // ADCINT1 habilitado
    AdcRegs.INTSEL1N2.bit.INT1CONT  = 0;    // Desativar modo contínuo ADCINT1
    AdcRegs.INTSEL1N2.bit.INT1SEL   = 1;    // configure o EOC1 para acionar o ADCINT1 para disparar
    AdcRegs.ADCSOC0CTL.bit.CHSEL    = 4;    // defina a seleção do canal SOC0 para ADCINA4
    AdcRegs.ADCSOC1CTL.bit.CHSEL    = 2;    // defina a seleção do canal SOC1 para ADCINA2
    AdcRegs.ADCSOC0CTL.bit.TRIGSEL  = 5;    // definir o gatilho de início do SOC0 no EPWM1A, devido ao round-robin, o SOC0 converte primeiro e depois o SOC1
    AdcRegs.ADCSOC1CTL.bit.TRIGSEL  = 5;    // definir o gatilho de início do SOC1 no EPWM1A, devido ao round-robin SOC0 converte primeiro e depois SOC1
    AdcRegs.ADCSOC0CTL.bit.ACQPS    = 6;    // defina a janela SOC0 S/H para 7 ciclos de relógio ADC, (6 ACQPS mais 1)
    AdcRegs.ADCSOC1CTL.bit.ACQPS    = 6;    // defina a janela SOC1 S/H para 7 ciclos de relógio ADC, (6 ACQPS mais 1)
    EDIS;                                   //
                                            //
                                            // Assume que o relógio ePWM1 já está habilitado em InitSysCtrl();
   EPwm1Regs.ETSEL.bit.SOCAEN   = 1;        // Habilitar SOC em um grupo
   EPwm1Regs.ETSEL.bit.SOCASEL  = 4;        // Selecione SOC do CMPA na contagem crescente
   EPwm1Regs.ETPS.bit.SOCAPRD   = 1;        // Gerar pulso no 1º evento
   EPwm1Regs.CMPA.half.CMPA     = 0x0080;   // Definir valor de comparação A
   EPwm1Regs.TBPRD              = 0xFFFF;   // Definir período para ePWM1
   EPwm1Regs.TBCTL.bit.CTRMODE  = 0;        // conte e comece
                                            //
                                            // Aguarde a interrupção do ADC
for(;;)                                                 //
   {                                                    //
  LoopCount++;                                          //
   }                                                    //
                                                        //
}                                                       //
                                                        //
                                                        //
__interrupt void  adc_isr(void)                         //
{                                                       //
                                                        //
  Voltage1[ConversionCount] = AdcResult.ADCRESULT0;     //
  Voltage2[ConversionCount] = AdcResult.ADCRESULT1;     //
                                                        //
                                                        // If 20 conversions have been logged, start over
  if(ConversionCount == 9)                              //
  {                                                     //
     ConversionCount = 0;                               //
  }                                                     //
  else ConversionCount++;                               //
                                                        //
  AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;                 // Clear ADCINT1 flag reinitialize for next SOC
  PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;               // Acknowledge interrupt to PIE
                                                        //
  return ;                                               //
}                                                       //


