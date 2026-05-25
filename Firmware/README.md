# 💻 Firmware - Fonte Linear Ajustável (FLM System)

Este diretório contém o código-fonte integral do microcontrolador **ESP32**, responsável pelo sistema de instrumentação inteligente, processamento digital de sinais (DSP) e telemetria remota da Fonte Linear Ajustável (12V / 1A).

O desenvolvimento foi realizado em **C++**, com gerenciamento de dependências e compilação otimizada pelo **PlatformIO** no ambiente Visual Studio Code.

---

## 🗂️ Arquitetura do Software

A arquitetura foi projetada para otimizar o uso da memória SRAM do microcontrolador, isolando a lógica de controle pesada da interface gráfica:

* 📄 **`main.cpp`**: O núcleo de controle. Gerencia periféricos (I2C, ADC, Wi-Fi), executa o ciclo de DSP, calibra as leituras e responde aos *endpoints* da API REST.
* 📄 **`html.h`**: Armazena o *front-end* completo (HTML, CSS e JavaScript Vanilla) diretamente na memória de programa do chip (`PROGMEM`), liberando a RAM para o processamento de rede.
* 📄 **`images.h` e `images.cpp`**: Recursos visuais convertidos em matrizes de bytes binários (*hex arrays*), evitando a necessidade de um sistema de arquivos (SPIFFS/LittleFS).

---

## 🧠 Processamento Digital de Sinais (DSP) e Calibração

O grande diferencial deste firmware é atuar como um instrumento de precisão, não confiando cegamente nas leituras brutas do ADC. Foram implementadas quatro camadas de tratamento matemático:

### 1. Filtro Passa-Baixa (Rejeição de 60Hz)
Para mitigar o ruído eletromagnético (EMI) e a oscilação (*ripple*) da fonte AC, a função `lerMedia()` coleta 60 amostras do ADC com um atraso cirúrgico de 277µs entre cada leitura. 
O tempo total de amostragem ($60 \times 277\mu s$) resulta em aproximadamente **16,6 milissegundos** — o equivalente exato a um ciclo completo da rede elétrica de 60Hz. Isso promove o cancelamento natural das harmônicas da rede.

### 2. Auto-Tara de Ruído Basal (Offset)
Amplificadores Operacionais possuem tensões de deslocamento intrínsecas. Durante o *boot*, o algoritmo executa `medirOffset()`, aferindo 200 amostras do canal de corrente com a fonte em repouso. Esse viés de hardware é armazenado e subtraído de todas as leituras futuras, garantindo que o zero absoluto seja real.

### 3. Regressão Linear de 2 Pontos
A não-linearidade do ADC do ESP32 foi corrigida via software aplicando a equação da reta ($y = ax + b$). O código compara tensões e correntes reais medidas por um instrumento de referência com as leituras brutas do ADC, calculando os coeficientes dinamicamente para gerar uma leitura perfeitamente calibrada.

### 4. Compensação de Carga (Voltage Drop)
Como o resistor *shunt* (0,22 Ω) está em série com o retorno (GND), ele gera uma queda de tensão que mascararia o valor entregue à carga. O firmware calcula ativamente a tensão dissipada pelo próprio *shunt* e a subtrai da medição global da fonte (`tensaoAtual = tensao - Vshunt_real`), exibindo apenas a tensão útil e real.

### 5. Supressão de Zona Morta (Zero Blanking)
Ruídos térmicos residuais que representem medições fantasmas inferiores a **90mA** são logicamente forçados a `0.000 A`, garantindo total estabilidade do display em momentos de circuito aberto.

---

## 📡 Servidor Web e Interface Gráfica

O ESP32 atua de forma autônoma como um *Soft Access Point* (SoftAP), gerando sua própria rede Wi-Fi (`SSID: FLM_FTE12V`). O roteamento foi desenhado no padrão REST:

* **`GET /`** : Transmite o Dashboard (Dashboard responsivo e sem recarregamento de página).
* **`GET /dados`** : Endpoint de telemetria. Retorna um **JSON** com as grandezas filtradas em tempo real. Exemplo:
  ```json
  {"v": 12.05, "i": 0.850, "p": 10.24}