//************************************** BIBLIOTECAS ************************************************
#include "stdio.h"
#include "DS1302.h"
#include <Fuzzy.h>
//************************************* PARAMETROS DO PROJETO ***************************************
const int ClkPino = 5; // Pino Clk
const int DatPino = 6; // Pino Dat para relogio
const int RstPino = 7; // Pino Rst
DS1302 rtc(RstPino, DatPino, ClkPino); //objeto dispositivo hora 

#define pinSensor1 A0 // sensor da terra 
 
#define pinRele1 12  //Porta relé
#define nivelRele LOW

#define pinVermelho1 8 
#define pinVerde1 10
#define pinAmarelo 9

int leituraSensor;
int leituraSensor1;
int leituraAnterior1;

String sinais; //para enviar os sinais

//----------------------------------ASSINATURA FUNCOES
int getHour();
char* getTime();


int umidadeSensor(int pinSensor);
Fuzzy *fuzzy = new Fuzzy();//--------Instanciação dos objetos fuzzy

//umidadde - variaveis de entrada
FuzzySet *baixa = new FuzzySet(0, 20, 30, 40);
FuzzySet *media = new FuzzySet(30, 40, 40, 50);
FuzzySet *alta = new FuzzySet(40, 55, 60, 70);

//hora - variaveis de entrada
FuzzySet *manha = new FuzzySet(0, 7, 8, 10);
FuzzySet *tarde = new FuzzySet(9, 12, 15, 18);
FuzzySet *noite = new FuzzySet(17, 19, 20, 24);

//BOMBA - variaveis de saida
FuzzySet *acionar = new FuzzySet(0, 1, 1, 1);
FuzzySet *desligar = new FuzzySet(-1, 0, 0, 1);
FuzzySet *adiar = new FuzzySet(-1, 0, 0, 0);

//************************************** CODIGO DE CONFIGURACAO ***************************************
void setup() {
  Serial.begin(9600);
  delay(6000);            // Aguarda o sensor armar  
  //dht.begin();            // Inicializa o sensor

   //Definir a hora para seta-la no chip -> APENAS UMA VEZ
   Time dataT(2023, 04, 05, 07, 12, 05, Time::kMonday); 
   rtc.time(dataT);

   pinMode(pinSensor1, INPUT); //sensor
   pinMode(pinRele1, OUTPUT); //rele
   digitalWrite(pinRele1, !nivelRele);

   pinMode(pinVermelho1, OUTPUT); //led vermelho
   pinMode(pinVerde1, OUTPUT); //led verde
   pinMode(pinAmarelo, OUTPUT); //led amarelo
  
  FuzzyInput *umidade = new FuzzyInput(1);
  umidade->addFuzzySet(baixa);
  umidade->addFuzzySet(media);
  umidade->addFuzzySet(alta);
  fuzzy->addFuzzyInput(umidade);

  FuzzyInput *hora = new FuzzyInput(2);
  hora->addFuzzySet(manha);
  hora->addFuzzySet(tarde);
  hora->addFuzzySet(noite);
  fuzzy->addFuzzyInput(hora);

  FuzzyOutput *bomba = new FuzzyOutput(1);
  bomba->addFuzzySet(acionar);
  bomba->addFuzzySet(desligar);
  bomba->addFuzzySet(adiar);
  fuzzy->addFuzzyOutput(bomba);

//construindo a regra difusa BAIXA
  FuzzyRuleAntecedent *umidadeBaixaAndhoraManha = new FuzzyRuleAntecedent();
  umidadeBaixaAndhoraManha->joinWithAND(baixa, manha);

  FuzzyRuleAntecedent *umidadeBaixaAndhoraTarde = new FuzzyRuleAntecedent();
  umidadeBaixaAndhoraTarde->joinWithAND(baixa, tarde);

  FuzzyRuleAntecedent *umidadeBaixaAndhoranoite = new FuzzyRuleAntecedent();
  umidadeBaixaAndhoranoite->joinWithAND(baixa, noite);

//construindo a regra difusa MEDIA
  FuzzyRuleAntecedent *umidadeMediaAndhoraManha = new FuzzyRuleAntecedent();
  umidadeMediaAndhoraManha->joinWithAND(media, manha);

  FuzzyRuleAntecedent *umidadeMediaAndhoraTarde = new FuzzyRuleAntecedent();
  umidadeMediaAndhoraTarde->joinWithAND(media, tarde);

  FuzzyRuleAntecedent *umidadeMediaAndhoranoite = new FuzzyRuleAntecedent();
  umidadeMediaAndhoranoite->joinWithAND(media, noite);

//construindo a regra difusa ALTA
  FuzzyRuleAntecedent *umidadeAltaAndhoraManha = new FuzzyRuleAntecedent();
  umidadeAltaAndhoraManha->joinWithAND(alta, manha);

  FuzzyRuleAntecedent *umidadeAltaAndhoraTarde = new FuzzyRuleAntecedent();
  umidadeAltaAndhoraTarde->joinWithAND(alta, tarde);

  FuzzyRuleAntecedent *umidadeAltaAndhoranoite = new FuzzyRuleAntecedent();
  umidadeAltaAndhoranoite->joinWithAND(alta, noite);

//construindo a regra difusa BOMBA ATIVAR
  FuzzyRuleConsequent *thenBombaAcionar = new FuzzyRuleConsequent();
  thenBombaAcionar->addOutput(acionar);

//construindo a regra difusa BOMBA NAO ATIVAR
  FuzzyRuleConsequent *thenBombadesligar = new FuzzyRuleConsequent();
  thenBombadesligar->addOutput(desligar);

//construindo a regra difusa BOMBA ADIAR
  FuzzyRuleConsequent *thenBombaadiar = new FuzzyRuleConsequent();
  thenBombaadiar->addOutput(adiar);

// REGRAS BAIXA
  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, umidadeBaixaAndhoraManha, thenBombaAcionar);
  fuzzy->addFuzzyRule(fuzzyRule1);

  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, umidadeBaixaAndhoraTarde, thenBombaadiar);
  fuzzy->addFuzzyRule(fuzzyRule2);

  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, umidadeBaixaAndhoranoite, thenBombaAcionar);
  fuzzy->addFuzzyRule(fuzzyRule3);

// REGRAS MEDIA
  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, umidadeMediaAndhoraManha, thenBombaAcionar);
  fuzzy->addFuzzyRule(fuzzyRule4);

  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, umidadeMediaAndhoraTarde, thenBombaadiar);
  fuzzy->addFuzzyRule(fuzzyRule5);

  FuzzyRule *fuzzyRule6 = new FuzzyRule(6, umidadeMediaAndhoranoite, thenBombaAcionar);
  fuzzy->addFuzzyRule(fuzzyRule6);

// REGRAS ALTA
  FuzzyRule *fuzzyRule7 = new FuzzyRule(7, umidadeAltaAndhoraManha, thenBombadesligar);
  fuzzy->addFuzzyRule(fuzzyRule7);

  FuzzyRule *fuzzyRule8 = new FuzzyRule(8, umidadeAltaAndhoraTarde, thenBombadesligar);
  fuzzy->addFuzzyRule(fuzzyRule8);

  FuzzyRule *fuzzyRule9 = new FuzzyRule(9, umidadeAltaAndhoranoite, thenBombadesligar);
  fuzzy->addFuzzyRule(fuzzyRule9);
}

//************************************** CODIGO PRINCIPAL ********************************************
void loop() {
  Time dataT = dataT;
  int input1 = umidadeSensor(pinSensor1);
  int input2 = getHour();
  float t = input1;  // Leitura da umidade 
  float h = input2; // Leitura da hora
  int a, i; // resultado e acao
  
  if (isnan(h) || isnan(t)) { // Verificar por erros
    Serial.println("ERRO VARIAVEIS");   
    return;
  }
  
  fuzzy->setInput(1, input1);
  fuzzy->setInput(2, input2);
  fuzzy->fuzzify();

  int saida = fuzzy->defuzzify(1);
  //("\n Result: ");
  if(saida == 1){
    a = 1; 
    i = 1; 
    //("\t UMIDADE BAIXA OU MEDIA - HORA IDEAL -> IRRIGAR ");
    //("\t ACIONAR \n\n");
    digitalWrite(pinVermelho1, HIGH); //vermelho liga
    digitalWrite(pinVerde1, LOW); //verde desliga
    digitalWrite(pinRele1, nivelRele); //relé ativa
    delay(20000);
    digitalWrite(pinRele1, !nivelRele); //relé desativa
  }
  else if(saida == -1){
    a = 2;  
    i = 2; 
    //("\t UMIDADE BAIXA OU MEDIA - HORA NAO IDEAL -> ADIAR ");
    //("\t ADIAR \n\n");
    digitalWrite(pinVermelho1, LOW); //vermelho desliga
    digitalWrite(pinVerde1, LOW); //verde desliga
    for(int i=0; i<=4; i++){
      digitalWrite(pinAmarelo, HIGH); //amarelo pisca
      delay(100);
      digitalWrite(pinAmarelo, LOW); //amarelo pisca
    } 
  }
  else{
    a = 0;
    i = 0;
    //(" UMIDADE ALTA OU HORA NAO IDEAL -> NAO IRRIGAR ");
    //("\t NAO ACIONAR \n\n");
    digitalWrite(pinVermelho1, LOW); //vermelho desliga
    digitalWrite(pinVerde1, HIGH); //verde liga
  }
  //envia os sinais do sensor para o BD
  Time dataTime = rtc.time(); //hora
  sinais = String(t) + "|" +
           String(h) + "|" +
           String(a) + "|" +
           String(i) + "|" +
           String(dataTime.yr) + "|" +
           String(dataTime.mon) + "|" +
           String(dataTime.date) + "|";
  Serial.println(sinais);
  
  delay(30000); 
}

//************************************** FUNCOES *********************************************************

int getHour() {  //hora
  Time dataT = rtc.time();
  
  return (dataT.hr);
}

char* getTime() {  //data
  Time dataT = rtc.time(); 
  char buf[70];

    //snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", 
    //dataT.yr, dataT.mon, dataT.date, dataT.hr, dataT.min, dataT.sec);
    snprintf(buf, sizeof(buf), "%04d|%02d|%02d",  
    dataT.yr, dataT.mon, dataT.day);

    //Serial.print("\t LEITURA DATA/HORA: ");
    //Serial.println(buf);
    return (dataT.hr);
    
}//--------------------------------FIM

int umidadeSensor(int pinSensor){ //LEITURA SENSOR

  digitalWrite(pinAmarelo, HIGH); 
  int leituraSensor = analogRead(A0);
  int porcento = map(leituraSensor, 600, 1023, 100, 0);  //da faixa original de 0 a 1023 para a faixa desejada de 0 a 100.

  //Serial.print("\t LEITURA SENSOR: ");
  //Serial.print(porcento);
  //Serial.println("%");
  
  delay(3000);
  digitalWrite(pinAmarelo, LOW); //amarelo desliga
  
  return porcento;
  
} //--------------------------------  FIM
