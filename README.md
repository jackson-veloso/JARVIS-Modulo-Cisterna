
# JARVIS MODULO CAIXA DAGUA

Utilizado 03 sensores de nível de água para leitura do nível de água.

## CONFIGURAÇÃO DO MÓDULO
- ID - 5  
- Sensor Nivel de agua: id 10

### CODIGOS
- 100 - msg hello
- 200 - msg de leitura de sensores
- 300 - acionamento de sensores

{  
    "code":100,  
    "id":2,  
    "description":"Modulo Cisterna"  
}  
{  
    "code": 200,  
    "idSensor": 1005,  
    "description": "nivel de agua",  
    "cheio": false,  
    "meio": false,  
    "vazio": false  
}  
{  
    "code":200,  
    "idSensor":1102,  
    "description":"chave rele",  
    "status":false  
}  
{  
    "code":300,  
    “idModule”: 2,  
    "idSensor":1102,  
    "status":true  
} 


## FUNCOES DOS SENSORES
readSensorCheio()  //leitura do sensor de nível de água - cheio  
readSensorMeio()  //leitura do sensor de nível de água - meio  
readSensorVazio()  //leitura do sensor de nível de água - vazio    
readButton()  //leitura do botão de ligar a bomba - pressionar por 02 segundos  
setModuleRele(bool status)  //aciona o módulo relé - bomba dágua  

## CONFIGURAÇÕES REDE MESH
- ID: ID do módulo no projeto JARVIS  
- STATION_SSID: nome da rede wifi  
- STATION_PASSWORD: senha da rede wifi  
- STATION_CHANNEL: canal do AP (necessário configurar o AP local para canal fixo)  

## FUNÇÕES REDE MESH
getMessageHello(JsonDocument myObject)  //imprime as msg hello recebidas  
sendMessageHello()  //envia msg hello a cada 11s  
checkConnection()  //reinicia o ESP se perder conexao a rede mesh  

sendSensorReandingNivelCaixaDagua()  //envia a leitura dos sensores de nível de água a cada 5s  
void sendStatusRele()  //envia o status do Modulo rele a cada 5s (true-desligado / false-ligado)
void setStatusSensors(JsonDocument doc)  //liga/desliga o Modulo Rele - chama a funcao setModuleRele(status)

## Observações:
- Somente um dispositivo pode ser o nó raiz na malha (root)  
- Configurar o SSID e Senha da Rede WIFI Local
- Configurar Roteador WIFI para canal Manual, o mesmo canal do AP deve ser configurado em todos os dispositivos pertencentes a rede mesh

## Referências
https://github.com/jackson-veloso/ESP32_WATCHDOG  
https://github.com/jackson-veloso/ESP32_REDE_MESH_PAINLESSMESH  
https://github.com/jackson-veloso/JARVIS-Modulo-ESP32-Client  
https://github.com/jackson-veloso/ESP32_EFEITO_BOUCING  

https://blog.eletrogate.com/criando-uma-rede-mesh-com-os-modulos-esp-8266-esp-32-esp-01/  
https://randomnerdtutorials.com/esp-mesh-esp32-esp8266-painlessmesh/  
https://gitlab.com/painlessMesh/painlessMesh  
https://www.fernandok.com/2018/06/travou-e-agora.html  
https://labdegaragem.com/forum/topics/esp32-travou-e-agora  
https://www.blogdarobotica.com/2021/05/24/entenda-o-efeito-bounce-e-como-fazer-debounce-no-arduino-ao-apertar-um-botao/  
https://blog.eletrogate.com/o-que-e-o-efeito-bouncing-e-como-evita-lo/  
https://www.usinainfo.com.br/blog/sensor-de-nivel-de-agua-com-arduino-automacao-residencial-de-controle/  
