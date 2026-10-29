#include <IRremote.hpp>
#define IR1 A6
#define IR2 A7
#define IR 3
#define RESET 2

uint32_t tempo_inicio, tempo_volta;
uint16_t dist, dist_ref, cont;
bool flag_cronometro = false; // flag cronômetro contando
bool flag_travado    = false; // flag cronômetro travado
bool flag_disparo    = false; // flag disparo do sensor
bool flag_serial     = false; // flag comunicação serial

const int desvio = 0;
const int tempo_minimo = 3000;
const int n = 2;


void setup()
{
  // define pinos como entrada e saída
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(RESET, INPUT_PULLUP);

  // inicia emissor IR
  IrSender.begin(IR);
  delay(500);

  // distancia de referência
  dist_ref = 1;

  // inicia comunicação serial
  if(flag_serial)
  {
    Serial.begin(115200);
  }
}

void loop()
{
  // leitura da distancia
  dist = media_ir(IR1, IR2);

  // envia valor pela serial
  if(flag_serial)
  {
    Serial.print("Distancia: ");
    Serial.println(dist);
  }

  // ZERA manual
  if(digitalRead(RESET) == LOW)
  {
    IrSender.sendNEC(0x00, 0xD, 0);
    flag_cronometro = false;
    flag_travado = false;
    flag_disparo = false;
    tempo_inicio = millis();
    delay(100);
  }

  // detecta passagem
  if (dist < (dist_ref -desvio) && !flag_disparo && ((millis() -tempo_inicio) > 3000UL))
  {
    if(flag_serial)
    {
      Serial.println("PASSAGEM!");
      Serial.println(dist);
    }

    // atualiza tempo da volta
    tempo_volta = millis() -tempo_inicio;

    // largada
    if(!flag_cronometro)
    {
      dispara_cronometro();
      flag_travado = false;
    }
    // chegada
    else
    {
      IrSender.sendNEC(0x00, 0x5, 0);
      flag_cronometro = false;
      flag_travado = true;
      tempo_inicio = millis();
    }
    flag_disparo = true;
  }

  // não detecta passagem
  if (dist >= dist_ref -desvio)
  {
    flag_disparo = false;
  }
}

// função sensor IR
float ir(byte pin1, byte pin2)
{
  int s1 = analogRead(pin1);
  int s2 = analogRead(pin2);
  return (s1 < 512 && s2 < 512) ? 1 : 0;
}

// função filtragem
float media_ir(byte pin1, byte pin2)
{
  float soma = 0;

  for (int i = 0; i < n; i++)
  {
    soma += ir(pin1, pin2);
    delay(5);
  }

  return soma/n;
}

// função dispara cronometro
void dispara_cronometro()
{
  IrSender.sendNEC(0x00, 0x5, 0);
  tempo_inicio    = millis();
  flag_cronometro = true;
}