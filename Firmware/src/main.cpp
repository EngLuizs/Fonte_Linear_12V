//=======================================================================================
//	PROJETO     : FONTE LINEAR AJUSTAVEL 12V 1A COM MONITORAMENTO
//	EQUIPE      : FLM SYSTEMS
//	COMPILADOR  : VISUAL STUDIO CODE (1.116.0) + PLATFORMIO
//	PROCESSADOR : ESP32 WROOM 32E
//	DATA        : 15/04/2026
//	REVISAO		  : 22/05/2026
//	VERSÃO      : 0.02.01
//  OBS: Implementação do web server com o esp32, possibilitando a criação do site.
//***************************************************************************************/

// ================ INCLUSÃO DE BIBLIOTECAS ================
  #include <Arduino.h>
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  #include <WiFi.h>
  #include <WebServer.h>

// ================ INCLUSÃO DE FUNÇÕES ================
  #include "html.h"
  #include "images.h"

// ================ CONFIGURAÇÕES DE REDE ================
  const char* ssid = "FLM_FTE12V";
  const char* password = "11110000";

// ================ CONFIGURAÇÕES DO LCD E SERVER ================
  LiquidCrystal_I2C lcd(0x27, 20, 4);
  WebServer server(80);

// ================ DEFINIÇÃO DE PINOS ================
  #define SDA_PIN 16
  #define SCL_PIN 19
  #define PIN_V 34
  #define PIN_I 33
  #define PIN_LDRED 23
  #define PIN_LDGREEN 5

// ================ PARAMETROS ================
  float Vref = 1.1;          // Referência pura do ESP32 (ADC_0db)
  float fator_tensao = 15.0; // Divisor de tensão (14K + 1K) / 1K
  float Rshunt = 0.22;       // Resistor shunt para medição da corrente 
  float ganho_I = 4.0;       // Ganho do AmpOp da corrente 

// ================ CALIBRAÇÃO ================

  // ===== PONTOS DE CALIBRAÇÃO TENSÃO =====
    float V1_real = 3.93;
    float V1_adc  = 3.00;

    float V2_real = 14.26;
    float V2_adc  = 14.39;

    // ===== CALCULO AUTOMATICO =====
      float Va = (V2_real - V1_real) / (V2_adc - V1_adc);
      float Vb = V1_real - Va * V1_adc;

  // ===== PONTOS DE CALIBRAÇÃO CORRENTE =====
    float I1_real = 0.089;
    float I1_adc  = 0.007;

    float I2_real = 1.1;
    float I2_adc  = 1.166;

    // ===== CALCULO =====
      float Ia = (I2_real - I1_real) / (I2_adc - I1_adc);
      float Ib = I1_real - Ia * I1_adc;

// ================ VARIÁVEIS ================
  float tensaoAtual = 0;
  float correnteAtual = 0;
  float potenciaAtual = 0;
  unsigned long tempoAnterior = 0;

// ================ OFFSET CORRENTE ================
  float offsetShunt = 0;

// ================ FILTRO ================
  const int N_AMOSTRAS = 60;

  float lerMedia(int pin) {
    long soma = 0;
    for (int i = 0; i < N_AMOSTRAS; i++) {
      soma += analogRead(pin);
      delayMicroseconds(277); // 277us * 60 =~ 16,6 milissegundos (exatamente 1 ciclo de 60Hz)
    }
    return soma / (float)N_AMOSTRAS;
  }

// ================ MEDIÇÃO DE OFFSET ================
  void medirOffset() {
    float soma = 0;

    for (int i = 0; i < 200; i++) {
      float vadc_I = lerMedia(PIN_I);
      float Vshunt = (vadc_I / 4095.0) * Vref;
      soma += Vshunt;
      delay(5);
    }

    offsetShunt = soma / 200.0;
  }
// ==========================================
//             SETUP INICIAL
// ==========================================
  void setup() {
    Serial.begin(115200);

    // ================ INICIALIZAÇÃO DO LCD ================
      Wire.begin(SDA_PIN, SCL_PIN);
      lcd.init();
      lcd.backlight();

    // ================ CONFIGURAÇÃO DE PINOS ================
      pinMode(PIN_LDRED, OUTPUT);
      pinMode(PIN_LDGREEN, OUTPUT);

    // ================ CONFIGURAÇÃO DO AP ================
      WiFi.mode(WIFI_AP);
      WiFi.softAP(ssid, password);

      IPAddress IP = WiFi.softAPIP();

      Serial.println("AP iniciado");
      Serial.println(IP);

      lcd.clear();
      lcd.setCursor(0,0); lcd.print("WiFi:");
      lcd.setCursor(0,1); lcd.print(ssid);
      lcd.setCursor(0,2); lcd.print("IP:");
      lcd.setCursor(0,3); lcd.print(IP);
      delay(2000);
      lcd.clear();

    // ==========================================
    //         CONFIGURAÇÃO DO SERVIDOR            
    // ==========================================

      // ================ PAGINA PRINCIPAL ================
        server.on("/", []() {
          server.send(200, "text/html", INDEX_HTML);
        });

      // ================ DADOS EM JSON ================
        server.on("/dados", []() {
          String json = "{\"v\":" + String(tensaoAtual,2) +
                        ",\"i\":" + String(correnteAtual,3) +
                        ",\"p\":" + String(potenciaAtual,2) + "}";
          server.send(200, "application/json", json);
        });

      // =============== IMAGENS ================
        server.on("/logo.png", []() {
        server.send_P(200, "image/png", (const char*)FLM_Systems_Logo, FLM_Systems_Logo_len);
        });

        server.on("/fonte.png", []() {
        server.send_P(200, "image/png", (const char*)Fonte_de_Bancada, Fonte_de_Bancada_len);
        });
      
      // ================ INICIALIZAÇÃO DO SERVIDOR ================
        server.begin();

    // ================ CONFIGURAÇÃO DO ADC ================
      analogReadResolution(12);
      analogSetAttenuation(ADC_0db);

    // MEDIR OFFSET COM A FONTE SEM CARGA
      lcd.setCursor(0,0); lcd.print("Calculando OFFSET...");
      medirOffset();
      lcd.clear();
  }


// ==========================================
//             LOOP PRINCIPAL
// ==========================================
  void loop() {

    // ================ MANUTENÇÃO DO SERVIDOR ================
      server.handleClient();

    // ================ LEITURA DE DADOS E ATUALIZAÇÃO DO LCD ================
      if (millis() - tempoAnterior >= 300) {
        tempoAnterior = millis();
      
        // Leitura dos ADCs e cálculo de tensão, corrente e potência
          float vadc_V = lerMedia(PIN_V);
          float vadc_I = lerMedia(PIN_I);

          float Vadc = (vadc_V / 4095.0) * Vref;
          float Vshunt_amplificado = (vadc_I / 4095.0) * Vref;
          
          
          // ===== TRATAMENTO DA CORRENTE =====
          // Subtrair o offset do shunt para obter a corrente real, e evitar que o ruído seja interpretado como corrente
          float Vshunt_sem_offset = Vshunt_amplificado - offsetShunt;
          if (Vshunt_sem_offset < 0) Vshunt_sem_offset = 0; // Evitar valores negativos devido a ruído
          
          // Remover o ganho do amplificador para obter a tensão real no shunt
          float Vshunt_real = Vshunt_sem_offset / ganho_I;
          
          //===== TENSÃO E CORRENTE BRUTA =====
            // Descomente as linhas abaixo para usar os valores sem calibradção, apenas com o fator de escala e offset do shunt
            // tensaoAtual = (Vadc * fator_tensao);
            // correnteAtual = (Vshunt_real / Rshunt);
                
          
          // ===== LEITURAS CALIBRADAS =====
            float tensao = (Va * (Vadc * fator_tensao) + Vb);
            correnteAtual = (Ia * (Vshunt_real / Rshunt) + Ib);

          // ===== REMOÇÃO DA QUEDA DE TENSÃO NO RSHUNT =====
            tensaoAtual = tensao - Vshunt_real;

          // ===== ZERAR RUÍDO DA CORRENTE =====
            if (correnteAtual < 0.090) correnteAtual = 0;

          // ===== POTENCIA =====
            potenciaAtual = tensaoAtual * correnteAtual;

          digitalWrite(PIN_LDGREEN, correnteAtual < 1.1);
          digitalWrite(PIN_LDRED, correnteAtual >= 1.1);

        // ================ ATUALIZAÇÃO DO LCD ================
          lcd.setCursor(0, 0);
          lcd.print("FONTE LINEAR 12V   ");
          lcd.setCursor(0, 1);
          lcd.print("Tensao: ");
          lcd.setCursor(10, 1);
          lcd.print("        ");
          lcd.setCursor(10, 1);
          lcd.print(tensaoAtual, 2);
          lcd.print(" V");
          lcd.setCursor(0, 2);
          lcd.print("Corrente:");
          lcd.setCursor(10, 2);
          lcd.print("          ");
          lcd.setCursor(10, 2);
          lcd.print(correnteAtual, 3);
          lcd.print(" A");
          lcd.setCursor(0, 3);
          lcd.print("Potencia:");
          lcd.setCursor(10, 3);
          lcd.print("        ");
          lcd.setCursor(10, 3);
          lcd.print(potenciaAtual, 2);
          lcd.print("  W");
        }
  }