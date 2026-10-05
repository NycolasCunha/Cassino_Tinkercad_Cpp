#include <Adafruit_LiquidCrystal.h>

Adafruit_LiquidCrystal lcd(0);

// Definição dos pinos 
const int PIN_POTENCIOMETRO = A0;
const int PIN_BOTAO_CONTINUA = 10; // Botão para apostar / ligar
const int PIN_BOTAO_PARA = 9;      // Botão para desligar
const int PIN_BUZZER = 12;
const int PIN_LED_R = 3;
const int PIN_LED_G = 4;
const int PIN_LED_B = 2;

// Variáveis do sistema
int valor = 0;             // Leitura do potenciômetro
int aposta = 0;            // Valor mapeado da aposta
int apostaAnterior = -1;
bool sistema = true;       // Liga / Desliga o sistema

// Variáveis para debounce/leitura dos botões
bool estadoBtnContinuaAnt = HIGH;
bool estadoBtnParaAnt = HIGH;

void setup()
{
  // Configuração do LCD
  lcd.begin(16, 2);
  lcd.setBacklight(1);

  // Configuração dos Pinos
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  // Botões com PULLUP 
  pinMode(PIN_BOTAO_CONTINUA, INPUT_PULLUP);
  pinMode(PIN_BOTAO_PARA, INPUT_PULLUP);
  pinMode(PIN_POTENCIOMETRO, INPUT);

  Serial.begin(9600); // Conexão com o monitor
  
  // Inicializa semente para sorteio aleatório
  randomSeed(analogRead(A1));

  // Mensagem inicial
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" MINI CASSINO ");
  lcd.setCursor(0, 1);
  lcd.print("   BEM-VINDO!   ");
  delay(2000);
  lcd.clear();
}

void loop()
{
  // Leitura dos botões 
  bool btnContinua = digitalRead(PIN_BOTAO_CONTINUA);
  bool btnPara = digitalRead(PIN_BOTAO_PARA);

  //  AÇÃO DO BOTÃO PARAR (Desliga o sistema)
  if (btnPara == LOW && estadoBtnParaAnt == HIGH) {
    sistema = false;
    desligarLED();
    noTone(PIN_BUZZER);
    
    lcd.clear();
    lcd.setBacklight(0); // Desliga a luz do LCD
    Serial.println("Sistema Desligado.");
    delay(200);
  }
  estadoBtnParaAnt = btnPara;

  // AÇÃO DO BOTÃO CONTINUAR (Liga ou Realiza a Aposta)
  if (btnContinua == LOW && estadoBtnContinuaAnt == HIGH) {
    if (!sistema) {
      // Se estava desligado, liga o sistema
      sistema = true;
      lcd.setBacklight(1);
      lcd.clear();
      apostaAnterior = -1; // Força atualização da tela
      Serial.println("Sistema Ligado.");
    } else {
      // Se já estava ligado, realiza a jogada
      jogarCassino();
    }
    delay(200);
  }
  estadoBtnContinuaAnt = btnContinua;

  // FUNCIONAMENTO QUANDO O SISTEMA ESTÁ LIGADO
  if (sistema) {
    // Leitura do Potenciômetro
    valor = analogRead(PIN_POTENCIOMETRO);
    
    // Mapeia a leitura (0-1023) para valores de aposta (ex: R$ 5 a R$ 100)
    aposta = map(valor, 0, 1023, 5, 100);

    // O LED permanece aceso quando o sistema está ligado aguardando aposta
    definirCor(255, 255, 0); // Amarelo/Branco (Pronto)

    // Atualiza a tela LCD apenas quando a aposta mudar
    if (abs(aposta - apostaAnterior) >= 2) {
      lcd.setCursor(0, 0);
      lcd.print("SISTEMA LIGADO  ");
      lcd.setCursor(0, 1);
      lcd.print("Aposta: R$ ");
      lcd.print(aposta);
      lcd.print("   "); // Limpa caracteres sobressalentes
      
      Serial.print("Leitura Pot: ");
      Serial.print(valor);
      Serial.print(" | Aposta: R$ ");
      Serial.println(aposta);

      apostaAnterior = aposta;
    }
  }
}

// --- FUNÇÃO DO JOGO (ROLETA / SORTEIO) ---
void jogarCassino() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aposta: R$ ");
  lcd.print(aposta);
  lcd.setCursor(0, 1);
  lcd.print("Girando...      ");

  // Animação das cores e som da roleta girando
  for (int i = 0; i < 20; i++) {
    // Alterna cores no LED RGB
    int r = (i % 3 == 0) ? 255 : 0;
    int g = (i % 3 == 1) ? 255 : 0;
    int b = (i % 3 == 2) ? 255 : 0;
    definirCor(r, g, b);

    // Som de giro no Buzzer
    tone(PIN_BUZZER, 600 + (i * 40), 50);
    delay(60 + (i * 10)); // O giro desacelera gradativamente
  }

  // Sorteio aleatório: 1 = Ganhou, 0 = Perdeu
  int resultado = random(0, 2);

  if (resultado == 1) {
    // --- SE GANHAR ---
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  VOCE GANHOU!  ");
    lcd.setCursor(0, 1);
    lcd.print("  PARABENS! :)  ");

    // LED pisca em cores variadas + Som alto de vitória
    for (int j = 0; j < 6; j++) {
      definirCor(255, 0, 0); // Vermelho
      tone(PIN_BUZZER, 1500, 100);
      delay(120);

      definirCor(0, 255, 0); // Verde
      tone(PIN_BUZZER, 2000, 100);
      delay(120);

      definirCor(0, 0, 255); // Azul
      tone(PIN_BUZZER, 2500, 100);
      delay(120);
    }
    noTone(PIN_BUZZER);
  } 
  else {
    // --- SE PERDER ---
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  VOCE PERDEU!  ");
    lcd.setCursor(0, 1);
    lcd.print(" Tente de novo! ");

    // O LED DESLIGA completamente
    desligarLED();

    // Som triste/grave de derrota no buzzer
    tone(PIN_BUZZER, 300, 300);
    delay(350);
    tone(PIN_BUZZER, 150, 600);
    delay(650);
    noTone(PIN_BUZZER);
  }

  delay(2000); // Pausa para ver o resultado antes da próxima aposta
  lcd.clear();
  apostaAnterior = -1; // Força reexibição do menu
}

// --- FUNÇÕES AUXILIARES DO LED RGB ---
void definirCor(int r, int g, int b) {
  // Converte valores para ligar/desligar nos pinos digitais/PWM
  digitalWrite(PIN_LED_R, r > 0 ? HIGH : LOW);
  digitalWrite(PIN_LED_G, g > 0 ? HIGH : LOW);
  digitalWrite(PIN_LED_B, b > 0 ? HIGH : LOW);
}

void desligarLED() {
  definirCor(0, 0, 0);
}