#  Jantar dos Filósofos

Implementação em **C** (pthreads) do clássico problema de sincronização proposto por Dijkstra, resolvido com a estratégia de **hierarquia de recursos**: os garfos são numerados e todo filósofo pega primeiro o de menor número, o que elimina a possibilidade de deadlock.

Além da solução, o programa inclui um **watchdog** no `main`, que detecta a falta de progresso e avisaria caso algum dia houvesse um travamento, e um **log com timestamp** que permite acompanhar exatamente quem esperou por quem.

Desenvolvido para a disciplina de **T303-26 Proj de sistema operacional**
**Prof Felipe Jucá - Universidade de Fortaleza (UNIFOR)**

![Linguagem](https://img.shields.io/badge/linguagem-C11-blue)
![Threads](https://img.shields.io/badge/threads-pthreads-green)
![Plataforma](https://img.shields.io/badge/plataforma-Linux-lightgrey)

## Sumário

- [O problema](#-o-problema)
- [Por que isso é difícil](#-por-que-isso-é-difícil)
- [Estratégia implementada](#-estratégia-implementada)
- [Modelo escolhido e justificativa](#-modelo-escolhido-e-justificativa)
- [Por que C](#-por-que-c)
- [Estrutura do projeto](#-estrutura-do-projeto)
- [Como o programa funciona](#-como-o-programa-funciona)
- [Como compilar e executar](#-como-compilar-e-executar)
- [Exemplo de saída](#-exemplo-de-saída)
- [Decisões de implementação](#-decisões-de-implementação)
- [Validação](#-validação)
- [Limitações](#-limitações)
- [Autor](#-autor)

## O problema

Proposto por Dijkstra em 1965, o Jantar dos Filósofos ilustra os desafios de sincronização entre processos que disputam recursos. A cena é a seguinte:

- 5 filósofos sentam-se em uma mesa circular;
- um garfo é colocado entre cada par de filósofos (5 garfos);
- cada filósofo precisa de **2 garfos** (esquerda e direita) para comer;
- os filósofos alternam entre **pensar** e **comer**.

Cada filósofo executa o ciclo: **pensar → tentar pegar os garfos → comer → devolver os garfos**.

A dificuldade vem de um detalhe da mesa: como há 5 garfos para 5 filósofos e cada um precisa de dois, **nunca podem comer mais de 2 ao mesmo tempo**, e cada garfo é disputado pelos dois vizinhos. Cada filósofo representa uma thread, cada garfo representa um recurso compartilhado, e o desafio é fazer todos comerem sem que ninguém fique travado e sem que dois usem o mesmo garfo ao mesmo tempo.

## Por que isso é difícil

| Problema | O que acontece |
|---|---|
| **Deadlock** | Todos pegam o garfo da esquerda ao mesmo tempo e esperam o da direita para sempre (espera circular). |
| **Starvation (inanição)** | O sistema continua andando, mas um filósofo específico nunca consegue os dois garfos. |
| **Livelock** | Todos ficam ocupados (pegam e soltam garfos), mas ninguém avança. |
| **Race condition** | Sem exclusão mútua, dois filósofos poderiam usar o mesmo garfo. |

Os quatro problemas são diferentes entre si e uma solução pode resolver um e piorar outro. O **deadlock** é o mais conhecido: o programa para de andar, mas nenhuma thread termina nem dá erro, simplesmente ficam todas esperando. A **starvation** é mais sutil, porque o sistema como um todo continua progredindo, só que alguém fica de fora. O **livelock** parece progresso (as threads estão ativas), mas é trabalho inútil. Já a **race condition** é o que os mutexes evitam: sem eles, dois filósofos poderiam "pegar" o mesmo garfo.

O deadlock exige quatro condições simultâneas (Coffman). Basta quebrar uma delas para ele ser impossível:

| Condição de Coffman | Significado no jantar | Neste projeto |
|---|---|---|
| Exclusão mútua | Um garfo só pode estar com um filósofo | Necessária, mantida (mutex por garfo) |
| Posse e espera | Quem segura um garfo espera pelo outro | Mantida |
| Não preempção | Ninguém tira o garfo de quem o segura | Mantida |
| **Espera circular** | A fila de espera fecha um ciclo | **Eliminada pela hierarquia** |

## Estratégia implementada

| Estratégia | Compilação | Ideia | Resultado |
|---|---|---|---|
| **Hierarquia de recursos** | `make` ou `make hierarquia` | Garfos numerados; todos pegam primeiro o de **menor índice** |  Sem deadlock |

### O que seria a abordagem ingênua

A forma intuitiva de resolver seria cada filósofo pegar o garfo da esquerda e depois o da direita. Parece razoável, mas basta que todos fiquem com fome ao mesmo tempo para dar problema: cada um segura o garfo da esquerda e espera o da direita, que está nas mãos do vizinho. Forma-se um ciclo (o filósofo 1 espera o 2, que espera o 3, e assim por diante até o 5 esperar o 1) e ninguém avança. Essa abordagem **não está implementada** neste projeto; ela serve apenas para entender o que a hierarquia resolve.

### Hierarquia de recursos

Os garfos têm uma ordem global (o índice). Como todos pegam primeiro o garfo de menor número, **não existe ciclo de espera**. Exemplo: o filósofo 5 usa os garfos 4 e 0 e pega o 0 antes do 4, o que quebra o ciclo.

| Filósofo (como aparece no log) | Garfos | Pega primeiro | Pega depois |
|---|---|---|---|
| 1 | 0 e 1 | 0 | 1 |
| 2 | 1 e 2 | 1 | 2 |
| 3 | 2 e 3 | 2 | 3 |
| 4 | 3 e 4 | 3 | 4 |
| 5 | 4 e 0 | **0** | **4** |

> **Numeração:** na saída, os filósofos aparecem numerados de **1 a 5** (o código soma 1 ao `id` apenas ao imprimir), enquanto os garfos mantêm o índice interno, de **0 a 4**. O `id` interno (0 a 4) não muda, pois também é usado para calcular os garfos de cada filósofo.

Só o filósofo 5 se comporta diferente da abordagem intuitiva: em vez de pegar primeiro o garfo da esquerda (4), ele pega o 0. Os demais já pegavam o menor primeiro.

Isso não elimina a espera: um filósofo pode ficar bloqueado enquanto o vizinho come. O que a hierarquia garante é que **a espera sempre termina**. O raciocínio é o seguinte: o garfo de maior número (o 4) só é pedido como **segundo** garfo, então quem o segura já tem o primeiro e está comendo; quando terminar, devolve os dois. A cadeia de espera nunca volta ao ponto de partida, e o último da fila sempre consegue avançar.

## Modelo escolhido e justificativa

**Solução principal: hierarquia de recursos.**

- **Simplicidade:** a solução é só a ordem de aquisição dos garfos. Não há estado extra, semáforos nem lógica de coordenação entre filósofos.
- **Corretude demonstrável:** elimina a espera circular, e a prova cabe em uma frase: se os recursos têm ordem e todos os pedem em ordem crescente, nenhum ciclo de espera consegue se fechar.
- **Boa concorrência:** até 2 filósofos comem ao mesmo tempo (o máximo possível com 5 garfos), diferente de um mutex global, que serializaria tudo e deixaria garfos livres sem uso.
- **Isolamento:** a estratégia vive em `garfos.c`; o filósofo só chama `pegar_garfos()`/`devolver_garfos()`. Trocar de estratégia no futuro não exigiria mexer em `filosofo.c`.

**Descartadas** (não implementadas): mutex global (ineficiente, pois só um filósofo comeria por vez), limitar a N-1 filósofos à mesa (funciona, mas exige um semáforo contador extra) e trylock com backoff (risco de livelock e espera ocupada, já que os filósofos podem ficar pegando e soltando garfos sem nunca comer).

## Por que C

- **É a linguagem dos sistemas operacionais:** `pthread_mutex_t` é a primitiva que o sistema oferece, sem camada de abstração escondendo a sincronização. O que está escrito no código é o que o SO executa.
- **Paralelismo real:** em linguagens com GIL (como CPython), as threads não executam em paralelo de verdade; em C a concorrência é real e os problemas aparecem como no SO, inclusive o deadlock.
- **Controle explícito:** cabe ao programador evitar race conditions (`atomic_int`, `rand_r` em vez de `rand`), o que torna a análise mais rica e obriga a pensar em cada acesso compartilhado.
- **Ferramentas de verificação:** ThreadSanitizer e Helgrind funcionam diretamente sobre o código C, sem instrumentação adicional além de uma flag de compilação.

## Estrutura do projeto

```
jantar-filosofos/
├── include/
│   ├── filosofo.h        # struct Filosofo, constantes e ciclo de vida
│   ├── garfos.h          # interface da estratégia (pegar/devolver garfos)
│   └── log.h             # log com timestamp
├── src/
│   ├── main.c            # cria as threads, watchdog de deadlock, resumo
│   ├── filosofo.c        # pensar, pegar, comer, devolver
│   ├── garfos.c          # hierarquia de recursos (mutex por garfo)
│   └── log.c             # log thread-safe
├── Makefile
└── README.md
```

O código segue uma separação simples: `filosofo.c` cuida **do que** o filósofo faz (o ciclo), `garfos.c` cuida **de como** os garfos são disputados (a estratégia) e `log.c` cuida de **mostrar** o que acontece. Os arquivos `.h` em `include/` são as interfaces entre esses módulos.

## Como o programa funciona

1. O `main` inicializa o log e os mutexes dos garfos, cria uma thread por filósofo e começa a observar o progresso.
2. Cada thread repete o ciclo até atingir `REFEICOES_ALVO`: **pensa** (espera um tempo entre 50 e 200 ms), fica com **fome** e chama `pegar_garfos()`, **come** (outra espera de 50 a 200 ms), incrementa seu contador de refeições e chama `devolver_garfos()`.
3. Em `pegar_garfos()`, a estratégia decide a ordem (menor índice primeiro, maior depois), trava o primeiro mutex, espera `ATRASO_ENTRE_GARFOS_US` e trava o segundo.
4. O `main` verifica o total de refeições a cada 1 segundo. Se o total ficar **5 segundos seguidos** sem aumentar, declara deadlock, mostra as refeições de cada filósofo e encerra com `exit(1)`.
5. Se todos terminarem, o `main` espera as threads (`pthread_join`), imprime o resumo e libera os recursos.

Cada ação gera uma linha de log com o tempo desde o início do programa, o que permite reconstruir exatamente quem esperou por quem.

## Como compilar e executar

Requisitos: `gcc` e `make` em Linux (ou WSL). Em distribuições baseadas em Debian/Ubuntu, `sudo apt install build-essential` instala os dois.

```bash
make && ./filosofos_hierarquia   # `make` e `make hierarquia` fazem a mesma coisa
make clean
```

Com o valor padrão (1 refeição por filósofo), o programa termina em cerca de 1 segundo, com o resumo das refeições e código de saída 0.

> Compile com `-std=gnu11`: o `-std=c11` esconde `usleep` e `rand_r`.

**Parâmetros ajustáveis** (constantes no código; é preciso recompilar depois de alterar):

| Constante | Arquivo | Função |
|---|---|---|
| `NUM_FILOSOFOS` | `include/filosofo.h` | Número de filósofos/garfos (use 2 ou mais) |
| `REFEICOES_ALVO` | `include/filosofo.h` | Refeições por filósofo (padrão: 1) |
| `TEMPO_MIN_MS` / `TEMPO_MAX_MS` | `src/filosofo.c` | Faixa (50 a 200 ms) do tempo de pensar e de comer |
| `ATRASO_ENTRE_GARFOS_US` | `src/garfos.c` | Pausa entre o 1º e o 2º garfo (100 000 µs). Aumenta a chance de intercalamento entre os filósofos |
| `SEGUNDOS_SEM_PROGRESSO` | `src/main.c` | Tempo sem ninguém comer para declarar deadlock (5 s) |

O valor padrão (1) mantém a saída curta e fácil de acompanhar, com cada filósofo comendo uma única vez. Para uma execução mais longa, aumente `REFEICOES_ALVO` (com 5 refeições por filósofo, o programa leva cerca de 3 segundos).

## Exemplo de saída

Cada ação gera uma linha `[tempo em ms] Filosofo N: ação`. Esta é a saída completa de uma execução com `make && ./filosofos_hierarquia` (1 refeição por filósofo):

```
Estrategia: HIERARQUIA DE RECURSOS
[    0 ms] Filosofo 1: comecou a pensar
[    0 ms] Filosofo 2: comecou a pensar
[    1 ms] Filosofo 3: comecou a pensar
[    1 ms] Filosofo 4: comecou a pensar
[    1 ms] Filosofo 5: comecou a pensar
[   57 ms] Filosofo 3: esta com fome e tenta pegar os garfos
[   57 ms] Filosofo 3: pegou o garfo 2
[  131 ms] Filosofo 4: esta com fome e tenta pegar os garfos
[  131 ms] Filosofo 4: pegou o garfo 3
[  140 ms] Filosofo 5: esta com fome e tenta pegar os garfos
[  140 ms] Filosofo 5: pegou o garfo 0
[  155 ms] Filosofo 1: esta com fome e tenta pegar os garfos
[  164 ms] Filosofo 2: esta com fome e tenta pegar os garfos
[  164 ms] Filosofo 2: pegou o garfo 1
[  231 ms] Filosofo 4: pegou o garfo 4
[  231 ms] Filosofo 4: conseguiu os garfos e esta COMENDO (refeicao 1)
[  356 ms] Filosofo 4: devolveu os garfos
[  356 ms] Filosofo 5: pegou o garfo 4
[  356 ms] Filosofo 3: pegou o garfo 3
[  356 ms] Filosofo 3: conseguiu os garfos e esta COMENDO (refeicao 1)
[  356 ms] Filosofo 5: conseguiu os garfos e esta COMENDO (refeicao 1)
[  356 ms] Filosofo 4: terminou todas as refeicoes
[  470 ms] Filosofo 3: devolveu os garfos
[  470 ms] Filosofo 3: terminou todas as refeicoes
[  470 ms] Filosofo 2: pegou o garfo 2
[  471 ms] Filosofo 2: conseguiu os garfos e esta COMENDO (refeicao 1)
[  491 ms] Filosofo 5: devolveu os garfos
[  491 ms] Filosofo 5: terminou todas as refeicoes
[  492 ms] Filosofo 1: pegou o garfo 0
[  605 ms] Filosofo 2: devolveu os garfos
[  605 ms] Filosofo 2: terminou todas as refeicoes
[  605 ms] Filosofo 1: pegou o garfo 1
[  605 ms] Filosofo 1: conseguiu os garfos e esta COMENDO (refeicao 1)
[  694 ms] Filosofo 1: devolveu os garfos
[  694 ms] Filosofo 1: terminou todas as refeicoes

Resumo (refeicoes por filosofo)
Filosofo 1: 1 refeicoes
Filosofo 2: 1 refeicoes
Filosofo 3: 1 refeicoes
Filosofo 4: 1 refeicoes
Filosofo 5: 1 refeicoes
```

A saída é **praticamente constante** entre execuções, porque a semente dos tempos aleatórios é fixa (veja [Decisões de implementação](#-decisões-de-implementação)): a ordem em que os filósofos ficam com fome e a sequência geral de eventos se repetem. O que muda é o milissegundo exato de cada linha (1 a 2 ms de diferença, por causa do escalonador) e, quando duas ações acontecem no mesmo instante, a ordem entre elas pode trocar de lugar.

### Como ler esta saída

- **Início (0 a 1 ms):** os 5 filósofos começam a pensar ao mesmo tempo.
- **57 a 164 ms:** eles ficam com fome na ordem 3, 4, 5, 1, 2 (a de menor tempo de pensar). Cada um pega seu primeiro garfo, **exceto o Filosofo 1**: ele fica com fome aos 155 ms, tenta o garfo 0 e não consegue, porque o Filosofo 5 já o pegou aos 140 ms. É a hierarquia em ação: o 5 pega o 0 antes do 4, em vez de pegar o 4 primeiro.
- **231 ms:** o Filosofo 4 consegue o garfo 4 (que estava livre, pois o 5 não o pediu antes do 0) e come.
- **356 ms:** o 4 devolve os garfos e libera o garfo 3 e o 4 ao mesmo tempo. O Filosofo 3 (que esperava o garfo 3) e o Filosofo 5 (que esperava o garfo 4) comem **simultaneamente**, porque não dividem nenhum garfo.
- **470 a 605 ms:** o 3 termina e libera o garfo 2 para o Filosofo 2. O Filosofo 1 só consegue o garfo 0 aos 492 ms, quando o 5 termina, e o garfo 1 aos 605 ms, quando o 2 termina. É o último a comer.
- **Final:** o resumo mostra 1 refeição para cada filósofo, e nenhum ficou para trás.

Essas esperas (como a do Filosofo 1, de 155 ms a 605 ms) são temporárias e esperadas: o que a hierarquia impede é a espera infinita.

## Decisões de implementação

- **Um mutex por garfo** (mutex travado = garfo em uso). É a forma mais direta de modelar um recurso exclusivo e garante a exclusão mútua sem estado adicional.
- **`usleep` entre os dois garfos:** aumenta a chance de intercalamento entre os filósofos e deixa a disputa pelos garfos visível no log. Sem a pausa, a janela entre pegar o primeiro e o segundo garfo seria tão curta que as esperas quase não apareceriam.
- **Watchdog no `main`:** detecta deadlock por falta de progresso (nenhuma refeição nova por `SEGUNDOS_SEM_PROGRESSO` segundos) e encerra com mensagem clara. Com a hierarquia ele nunca dispara, mas serve de proteção: sem ele, um travamento deixaria o programa parado para sempre.
- **`atomic_int` no contador de refeições:** o `main` lê enquanto as threads escrevem, então um `int` comum seria uma data race.
- **`rand_r` com semente por filósofo:** `rand()` usa estado global e não é thread-safe. A semente é fixa (`(id + 1) * 7919`), então os tempos de pensar e comer de cada filósofo se repetem a cada execução. Por isso a ordem inicial em que ficam com fome é sempre a mesma (3, 4, 5, 1, 2 na numeração do log), o que torna o comportamento reproduzível. Para variar a cada execução, basta misturar `time(NULL)` na semente.
- **Log protegido por mutex:** evita linhas misturadas entre threads, já que `printf` de várias threads ao mesmo tempo poderia intercalar caracteres. O tempo vem de `CLOCK_MONOTONIC`, que não é afetado por ajustes no relógio do sistema.
- **Numeração de 1 a 5 na saída:** o código soma 1 ao `id` apenas na hora de imprimir (em `log.c` e no resumo do `main.c`). O `id` interno continua de 0 a 4, pois é usado como índice dos garfos; os garfos aparecem no log com o índice interno (0 a 4).
- **Contador de refeições por filósofo:** além de ser a base do watchdog, serve de evidência prática sobre starvation, pois mostra se algum filósofo ficou para trás.

## Validação

```bash
# ThreadSanitizer (detecta data races)
gcc -std=gnu11 -fsanitize=thread -Iinclude -pthread src/main.c src/filosofo.c src/log.c src/garfos.c -o tsan && ./tsan

# Helgrind (detecta problemas de lock; requer valgrind instalado)
valgrind --tool=helgrind ./filosofos_hierarquia
```

O ThreadSanitizer instrumenta os acessos à memória e avisa quando duas threads acessam o mesmo dado sem sincronização. O Helgrind, por sua vez, observa a ordem de aquisição dos locks e aponta inconsistências. Os dois complementam os testes funcionais.

- O programa terminou com todas as refeições concluídas em todas as execuções testadas (5 de 5).
- Nas execuções realizadas, o ThreadSanitizer não apontou data races.

Esses testes mostram que o comportamento esperado se repete, mas **não são uma prova formal**: a ausência de avisos em algumas execuções não garante que nenhuma intercalação problemática exista. A garantia de ausência de deadlock vem do argumento da ordem dos recursos, não dos testes.

## Limitações

- **Justiça não garantida:** a hierarquia não impede formalmente a starvation. Um filósofo azarado pode ser preterido pelos vizinhos. O contador de refeições mostra o equilíbrio na prática, mas não é prova. Na prática, alguns filósofos esperam mais que outros (no exemplo acima, o Filosofo 1 é o último a comer, pois depende do 5).
- **Tempos curtos:** os tempos de pensar/comer (50 a 200 ms) foram escolhidos para facilitar demonstração e vídeo, não para simular carga real.
- **Semente fixa:** a ordem inicial se repete a cada execução; a variação entre execuções vem só do escalonador do sistema.
- **Detecção por heurística:** o watchdog considera deadlock qualquer período de 5 segundos sem refeições novas. Com os tempos usados (no máximo 200 ms), isso só acontece de fato em caso de travamento, mas com tempos muito maiores poderia dar um falso positivo. Ao detectar, ele encerra o processo com `exit(1)` sem esperar as threads.
- **Número de filósofos:** com `NUM_FILOSOFOS` igual a 1, o garfo esquerdo e o direito seriam o mesmo mutex; use 2 ou mais.
- **Plataforma:** desenvolvido e testado em Linux (gcc, `-std=gnu11`).


## Autor

**Kauam**, Ciência da Computação, UNIFOR
GitHub: [@ksobreira](https://github.com/ksobreira)