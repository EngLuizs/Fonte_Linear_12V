# FONTE LINEAR AJUSTÁVEL 12V 1A COM MONITORAMENTO

Projeto desenvolvido pela equipe **FLM System** utilizando ESP32, KiCad e PlatformIO para controle, monitoramento e interface web de uma fonte linear regulada ajustável.

---

# Descrição do Projeto

O projeto consiste em uma fonte linear ajustável de até 12V e 1A com:

- Monitoramento de tensão, corrente e potência
- Interface Web via ESP32
- Display LCD I2C 20x4
- Medição analógica calibrada
- Indicação visual por LED
- PCB dedicada desenvolvida no KiCad
- Controle e aquisição utilizando ESP32-WROOM-32E

O sistema disponibiliza os dados em tempo real tanto no display LCD quanto em uma página web hospedada diretamente pelo ESP32 em modo Access Point.

---

# Funcionalidades

- Leitura de tensão de saída
- Leitura de corrente utilizando resistor shunt
- Cálculo automático de potência
- Web Server integrado
- Interface Web responsiva
- Calibração de tensão e corrente
- LEDs indicadores de status
- Comunicação I2C com display LCD
- Sistema de proteção visual por sobrecorrente

---

# Hardware Utilizado

## Processador
- ESP32-WROOM-32E

## Reguladores
- LM7812
- LM2576
- LD1117

## Componentes principais
- LM358
- TIP120
- BC547
- LCD I2C 20x4
- Shunt 0.22Ω / 5W 
- Shunt 0,44Ω / 5W

---

# Estrutura do Projeto

```text
Firmware/       -> Código fonte do ESP32
PCB/            -> Esquemáticos, PCB e Gerbers
Documentos/     -> Relatórios e documentação
Modelagem_3D/   -> Estruturas e peças mecânicas
```

---

# Firmware

O firmware foi desenvolvido utilizando:

- Visual Studio Code
- PlatformIO
- Framework Arduino para ESP32

## Principais bibliotecas

- Arduino.h
- Wire.h
- LiquidCrystal_I2C
- WiFi.h
- WebServer.h

---

# Interface Web

O ESP32 opera em modo Access Point criando a rede:

```text
SSID: FLM_FTE12V
Senha: 12345678
```

Após conexão, o sistema disponibiliza:
- Página web embarcada
- Dados em JSON
- Monitoramento em tempo real

---

# Monitoramento

O sistema realiza:

- Média de amostras ADC
- Filtragem sincronizada em 60Hz
- Correção de offset
- Calibração linear de tensão e corrente

---

# PCB e Projeto Eletrônico

O projeto eletrônico foi desenvolvido no:

- KiCad

Incluindo:
- Esquemático completo
- PCB dedicada
- Gerbers de fabricação
- Instrumentação analógica
- Fonte linear regulada
- Conversores DC-DC auxiliares

---

# Organização de Versionamento

Este projeto utiliza:

- Git
- GitHub

Para:
- Controle de revisões
- Histórico de alterações
- Backup
- Versionamento de firmware e PCB

---

# Equipe

FLM System

---

# Versão Atual

```text
Versão Firmware: 0.02.00
Última Revisão: 01/05/2026
```