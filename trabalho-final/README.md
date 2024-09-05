# Trabalho final (2024-1) - controle de sistemas de carro

Trabalho final da disciplina de Fundamentos de Sistemas Embarcados (2024/1)

## Membros

- Guilherme Soares Rocha - 211039789
- João Manoel Barreto Neto - 211039519
- Miguel Matos Costa de Frias Barbosa - 211039635
- Yan Luca Viana de Araújo Fontenele - 211031889

## Objetivos

Este projeto visa criar um sistema embarcado para controle e monitoramento de algumas funções de um carro. Os principais objetivos são:
- Controlar os faróis e a iluminação interna, coletar dados de temperatura, umidade e luminosidade, e otimizar o consumo de energia do veículo.
- Melhoria na eficiência energética do veículo, controle remoto eficaz dos sistemas do carro através do ThingsBoard, e uma interface de usuário intuitiva para monitoramento em tempo real.
- Coletar dados de sensores de luminosidade e outros sensores de controle.
- Utilizar um sistema distribuído com duas placas ESP32, cada uma controlando diferentes conjuntos de sensores e atuadores.
- Integrar o sistema com uma plataforma de monitoramento em tempo real (ThingsBoard), possibilitando a visualização dos dados e o controle remoto de componentes do carro.

## Descrição do Sistema
O sistema foi desenvolvido para funcionar com duas ESP32, que se comunicam via MQTT usando um broker. Ele utiliza sensores para monitorar condições do carro, como a luminosidade externa e interna, além de controlar os LEDs (indicando o estado dos faróis) e dispositivos PWM. A arquitetura do sistema foi organizada da seguinte forma:

- ESP32-Sensores: Responsável pela coleta de dados dos sensores de luminosidade e controle de dispositivos PWM. Ela envia os dados coletados ao ThingsBoard e interage com o dashboard.
- ESP32-GPIO: Controla os botões de entrada e LEDs de saída, responsável pelo controle das luzes do carro, como faróis e luz interna.

Ambas as ESP32 se comunicam com o ThingsBoard via MQTT, permitindo o monitoramento e controle centralizados no dashboard de um sistema chamado "Relâmpago Marquinhos", onde o usuário pode interagir com o sistema e visualizar os dados coletadas.

## Arquitetura do Projeto
A arquitetura do projeto está dividida entre hardware e software, como descrito abaixo:

#### Hardware:
- Duas ESP32. 
- Sensores: DHT11 (temperatura e umidade), LDR (luminosidade).
- LEDs: Indicando o estado dos faróis e iluminação interna.
- Botões de entrada: Controlam manualmente as funções do veículo, como ligar/desligar os faróis.

#### Software:

- Cada ESP32 possui um arquivo main diferente, sendo responsável por diferentes conjuntos de funcionalidades.
- ThingsBoard: Plataforma IoT que recebe e exibe os dados dos sensores e permite o controle remoto através de comandos MQTT.
- Comunicação MQTT: Utilizada para troca de informações entre os sensores e o ThingsBoard.
- Integração com o ThingsBoard: Dados de sensores e estado dos LEDs são monitorados e controlados via dashboard.

As ESP32 estão configuradas para operar em modo sleep. O objetivo é otimizar o consumo de energia, fazendo com que a ESP32 em modo sleep acorde apenas quando necessário, executando suas tarefas específicas e voltando ao estado de baixo consumo.
Configuração do Modo Sleep: A ESP32 é configurada para entrar em Light Sleep, um estado de baixo consumo de energia que permite a retomada rápida das operações. A GPIO conectada ao botão, é utilizada como o gatilho para acordar a placa.

- Configuração da GPIO:
    - A GPIO é configurada para entrada, com um resistor de pull-up habilitado.
    - A interrupção na GPIO é configurada para nível alto, o que significa que qualquer pressão no botão irá acordar a ESP32.

- Habilitação do Wakeup pela GPIO:
    - A função gpio_wakeup_enable é utilizada para habilitar a funcionalidade de wakeup pela GPIO.
    - A função esp_sleep_enable_gpio_wakeup ativa o wakeup através de uma interrupção na GPIO configurada.

A ESP32 acorda quando o botão é pressionado.

Comportamento Após Acordar:

- Verifica o nível da GPIO conectada ao botão.
- Se a GPIO estiver com o botão pressionado, espera até que o botão seja liberado.
- Uma vez liberado, a ESP32 entra novamente no modo Light Sleep após um curto período de delay.

Estado de Sleep Contínuo:
A ESP32 não permanece dormindo o tempo inteiro; ela entra em modo Light Sleep apenas quando as condições são atendidas, permitindo uma resposta rápida ao botão, mas retornando ao estado de baixo consumo rapidamente para economizar energia.


1. Pasta esp32_sensores
Esta parte do código é responsável por:
- Configuração do MQTT: Conexão com broker, publicando e assinando tópicos para comunicação de dados.
- Sensores e atuadores:
    - Leitura de temperatura e umidade.
    - Leitura de luminosidade através de um ADC.
    - Controle de um LED via PWM, associado à "luz interna".
    - Controle de faróis, com comunicação via MQTT para refletir o estado dos faróis e luz interna.
- Processamento de métodos: Interpretação de comandos recebidos via MQTT e execução de ações, como ligar/desligar faróis e ajustar a intensidade da luz interna.
- Envio de dados via MQTT: Telemetria sobre temperatura, umidade, e luminosidade, e envio de atributos como estado dos faróis e da luz interna.
- Modo Sleep: Implementação de um botão para acordar o sistema e colocá-lo em modo light sleep quando não há atividade, visando economia de energia.

2. Pasta esp32_gpios
- Controle de farol, travas, vidros.
- Configuração de MQTT: Similar à primeira ESP, configurando um cliente MQTT para comunicação.
- Leitura de sensores e controle de atuadores: Configuração de um ADC para leitura de luminosidade e, provavelmente, controle de GPIOs ou LEDs.

#### Diagrama
![Diagrama](/assets/diagrama.png)

## Passos para Execução
#### 1. Clone o repositório do GitHub:

```bash
git clone https://github.com/FGA-FSE/trabalho-final-relampago-marquinhos.git
```

#### 2. É recomendado que você instale a extensão ESP-IDF no Visual Studio Code. Ela auxilia no desenvolvimento e execução de projetos com ESP32.

#### 3. Configuração do ambiente de execução:

- Abra o projeto no VSCode.
- Abra dois terminais, pois você precisará rodar duas ESP32 separadamente com arquivos 'main' diferentes.

#### 4. Execução do projeto:

- No primeiro terminal, navegue até a pasta ESP32-Sensores:
```bash
cd esp32-sensores
```
- No segundo terminal, navegue até a pasta ESP32-GPIOS:
```bash
cd esp32-gpios
```
- Em ambos os terminais, execute o comando para fazer o build do projeto:
```bash
idf.py build
```
- Para dar o flash na placa, utilize o seguinte comando com a placa em modo boot:
```bash
idf.py flash monitor
```

#### 5. Configuração da Conexão Wi-Fi da ESP32

A ESP32 pode ser configurada para se conectar a uma rede Wi-Fi usando um processo de conexão automática e uma configuração manual via ponto de acesso (AP) se a conexão inicial falhar. Siga os passos abaixo para configurar o Wi-Fi da ESP32:

##### Passo a Passo para Configuração do Wi-Fi

1. Inicialização da Conexão Wi-Fi

- A ESP32 tentará se conectar à rede Wi-Fi configurada três vezes.
- Se a conexão falhar após três tentativas, a ESP32 ativará um ponto de acesso (AP) próprio para configuração manual.

2. Ativação do Ponto de Acesso

- Quando o ponto de acesso é ativado, a ESP32 criará uma rede Wi-Fi com um nome específico.
- No seu dispositivo, procure pela rede Wi-Fi com o nome fornecido e conecte-se a ela.

3. Acesso à Página de Configuração

- Após conectar-se ao ponto de acesso da ESP32, abra um navegador da web.
- A ESP32 deve redirecionar automaticamente para uma página de configuração.

4. Configuração da Rede Wi-Fi

- Na página de configuração, você verá campos para inserir o 'SSID' (nome da rede) e 'password' (senha) da rede Wi-Fi à qual deseja conectar a ESP32.
- Preencha os campos com as informações da sua rede Wi-Fi.
- Clique no botão para salvar as configurações.

5. Verificação da Conexão

- Após enviar as configurações, a ESP32 tentará se conectar à rede Wi-Fi configurada.
- A página de configuração exibirá uma mensagem indicando se a conexão foi bem-sucedida ou se houve algum erro.

#### 6. Acessando o Dashboard do ThingsBoard:

- Acesse o ThingsBoard, faça login na sua conta, e acesse os dashboards disponíveis.
- Vá até o dashboard 'RelampagoMarquinhos', onde você poderá visualizar os dados dos sensores e utilizar as entradas e sensores disponíveis no sistema.
![ThingsBoard](/assets/thingsBoard.png)