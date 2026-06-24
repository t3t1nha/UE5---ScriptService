# Scripted Service
 
## Descricao
 
Scripted Service e um jogo desenvolvido em Unreal Engine 5 onde o jogador programa um robot de servico para gerir pedidos num restaurante. O objetivo e criar sequencias de instrucoes que permitam ao robot navegar autonomamente entre as mesas e a cozinha, recolher e entregar pratos corretamente, e maximizar a pontuacao antes que os pedidos expirem.
 
## Mecanicas principais
 
O jogador interage com o mundo na primeira pessoa, podendo pegar em ingredientes fisicamente, interagir com aparelhos de cozinha e aceder ao sistema operativo do robot.
 
O robot executa programas compostos por blocos de instrucoes que o jogador define visualmente atraves de uma interface de arrastar e largar. O programa e carregado e executado em tempo real, com o robot a navegar pelo nivel usando o sistema de navegacao da Unreal Engine.
 
O sistema de pontuacao atribui pontos por cada entrega correta e penaliza o jogador por pedidos que expirem sem serem atendidos. As mesas geram pedidos aleatorios a intervalos configuraveis.
 
## Arquitetura do codigo
 
### Robot e execucao de programas
 
O robot e representado pela classe `ARobotCharacter`, que implementa a interface `IProgrammable`. O sistema de execucao funciona como um interpretador de bytecode simples com suporte a estruturas de controlo de fluxo.
 
As instrucoes sao representadas pelo struct `FRobotInstruction` e o robot mantem um apontador de instrucao (`InstructionPointer`) que avanca conforme os comandos sao completados ou as condicoes sao avaliadas.
 
Cada acao concreta (mover, recolher, entregar) e encapsulada numa classe que herda de `URobotCommand`. Os comandos asincronos, como o movimento, comunicam a sua conclusao atraves de delegates.
 
### Comandos disponiveis
 
- `UMoveCommand` - move o robot para uma mesa, para a cozinha ou para uma localizacao especifica
- `UTakeOrderCommand` - recolhe o pedido de uma mesa
- `UPickupCommand` - recolhe o prato do balcao da cozinha
- `UDeliverCommand` - entrega o prato na mesa correta
- `UWaitCommand` - pausa a execucao durante um numero de segundos definido
### Instrucoes de controlo de fluxo
 
O interpretador suporta blocos condicionais e ciclos que sao compilados para um formato plano de instrucoes com marcadores `EndBlock`:
 
- `IfTableHasOrder` - executa o bloco se a mesa tiver um pedido pendente
- `IfCarryingDish` / `IfNotCarryingDish` - condiciona com base no inventario do robot
- `IfKitchenHasOrder` - verifica se ha pratos disponiveis no balcao
- `IfTableWaitingTooLong` - verifica se uma mesa espera ha demasiado tempo
- `RepeatLoop` - repete o bloco um numero fixo de vezes
- `LoopForever` - repete o bloco indefinidamente
### Slots de memoria
 
O robot dispoe de quatro slots de memoria (A, B, C, D) que permitem guardar e reutilizar valores inteiros durante a execucao do programa. Instrucoes como `MoveToTable` e `TakeOrder` podem guardar o numero da mesa num slot, e outras instrucoes podem ler esse slot em vez de usar um valor literal.
 
### Interface de programacao
 
A interface de utilizador e composta por varios widgets:
 
- `UProgramSequenceWidget` - area onde o jogador constroi a sequencia de instrucoes por drag and drop
- `UBlockWidget` - representa um bloco de instrucao individual, com campos editaveis para parametros como numero de mesa ou duracao de espera
- `UContainerBlockWidget` - variante do bloco para instrucoes com corpo interno (condicionais e ciclos)
- `UProgrammingMenu` - janela principal que agrega a paleta de blocos e a sequencia
- `URobotOSWidget` - sistema operativo do robot acessivel ao aproximar-se do robot
### Gestao do restaurante
 
- `ATableManager` - regista todas as mesas do nivel e expoe a localizacao da cozinha
- `ATableActor` - representa uma mesa, gera pedidos aleatorios com temporizadores e implementa a interface `IOrderable`
- `AKitchenCounter` - balcao onde os pratos ficam disponiveis para o robot recolher, implementa `IPickupPoint`
- `ACustomGameMode` - gere a pontuacao, subscreve os eventos das mesas e controla o numero maximo de pedidos ativos em simultaneo
### Cozinha
 
`AApparatusActor` representa um aparelho de cozinha (grelha, fogao, forno, etc.). O jogador pode largar ingredientes no aparelho, e se a combinacao corresponder a uma receita na tabela de dados, o processo de cozinha e iniciado automaticamente com um temporizador.
