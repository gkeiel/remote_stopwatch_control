#include <IRremote.hpp>
#define TR A1
#define EC A0
#define IR 3
#define RESET 2

float dist, dist_chao;
unsigned long tempo_inicio, tempo_volta;
bool flag_cronometro = false; // flag cronômetro contando
bool flag_travado    = false; // flag cronômetro travado
bool flag_disparo    = false; // flag disparo do sensor
bool flag_serial     = false; // flag comunicação serial

const float desvio = 2.0;
const float histerese = 1.0;
const int tempo_minimo = 3000;
const int n = 5;


void setup()
{
  // define pinos como entrada e saída
  pinMode(EC, INPUT);
  pinMode(RESET, INPUT_PULLUP);
  pinMode(TR, OUTPUT);

  // inicia emissor IR
  IrSender.begin(IR);
  delay(500);

  // leitura da distancia ao chao
  dist_chao = media_ultrassom(EC, TR);

  // inicia comunicação serial
  if(flag_serial)
  {
    Serial.begin(115200);
  }
}

void loop()
{
  // leitura da distancia
  dist = media_ultrassom(EC, TR);

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
  if (dist < (dist_chao -desvio) && !flag_disparo && ((millis() -tempo_inicio) > 2000UL))
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
  if (dist >= dist_chao -desvio +histerese)
  {
    flag_disparo = false;
  }
}

// função sensor ultrassom
float ultrassom(float pin_echo, float pin_trigger)
{
  long t_start, t_stop;
  float dist;

  if (digitalRead(pin_echo) == 1) return 0;
  digitalWrite(pin_trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pin_trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin_trigger, LOW);

  t_start = micros();
  while (digitalRead(pin_echo) == 0)
  {
    if ((micros() -t_start) > 3000) return 100;
  }

  t_start = micros();
  while (digitalRead(pin_echo) == 1)
  {
    if ((micros() -t_start) > 4000) return 100;
  }

  t_stop = micros();
  dist = 0.0170145*(t_stop -t_start);
  return dist;
}

// função filtragem
float media_ultrassom(int pin_echo, int pin_trigger)
{
  float soma = 0;

  for (int i = 0; i < n; i++)
  {
    soma += ultrassom(pin_echo, pin_trigger);
    delay(5);
  }

  return soma/n;
}

// função dispara cronometro
void dispara_cronometro()
{
  IrSender.sendNEC(0x00, 0x5, 0);
  tempo_inicio = millis();
  flag_cronometro = true;
}