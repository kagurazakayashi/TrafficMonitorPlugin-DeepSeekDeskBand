# Assistente DeepSeek (DeepSeekDeskBand)

![screenshot](screenshot.png)

Plugin para [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor). Exibe o saldo do DeepSeek em tempo real na janela da barra de tarefas, com suporte a histórico de variações do saldo.

[English](README.md) | [简体中文](README.zh-Hans.md) | [繁體中文](README.zh-Hant.md) | [日本語](README.ja.md) | [Deutsch](README.de.md) | [עברית](README.he.md) | [Magyar](README.hu.md) | [Italiano](README.it.md) | [Polski](README.pl.md) | Português (Brasil) | [Русский](README.ru.md) | [Türkçe](README.tr.md)

## Funcionalidades

- Consulta periódica do saldo via API oficial do DeepSeek
- Exibe o valor do saldo e a moeda na barra de tarefas e na janela principal do TrafficMonitor
- Histórico de variações do saldo (armazenamento incremental automático, com limite ajustável)
- Suporte multilíngue (detecção automática do idioma do sistema ou seleção manual)
- Arquivo de configuração (incluindo a chave de API) e histórico criptografados com DPAPI do Windows, acessíveis apenas pelo usuário atual do Windows

## Instalação

1. Faça o download da DLL correspondente em [Release](https://github.com/KagurazakaYashi/TrafficMonitorPlugin-DeepSeekDeskBand/releases):
   - TrafficMonitor x86 → `DeepSeekDeskBand.dll`
   - TrafficMonitor x64 → `DeepSeekDeskBand64.dll`
2. Copie a DLL para a pasta `plugins` dentro do diretório do TrafficMonitor
3. Reinicie o TrafficMonitor
4. Clique com o botão direito na área vazia da barra de tarefas, selecione "Configurações de exibição" e marque "Assistente DeepSeek" para exibir

> **Nota**: A DLL deve estar na subpasta `plugins`. Colocá-la em outro diretório impedirá o carregamento.

## Descrição do conteúdo exibido

O conteúdo exibido pelo plugin é composto por duas partes:

- **Texto do rótulo à esquerda**: pode ser alterado em "Opções" → "Configurações da janela da barra de tarefas" → "Configurações de exibição de texto" no TrafficMonitor.
- **Texto do valor à direita**: exibe `valor do saldo moeda`, por exemplo `100.00 CNY`. Os dados do saldo são atualizados automaticamente em intervalos regulares.

No submenu "Itens de exibição" do menu de contexto do TrafficMonitor, o nome de exibição do plugin muda conforme o idioma selecionado (ex.: "Assistente DeepSeek" no ambiente em português).

## Abrir a janela de configuração do plugin

Clique com o botão direito na janela da barra de tarefas ou na janela principal do TrafficMonitor, selecione "Gerenciar plugins" no menu de contexto, clique com o botão direito neste plugin e selecione "Opções" para abrir a caixa de diálogo de configuração. Nela, você pode definir a chave de API e outras opções, além de visualizar o histórico.

## Descrição das opções de configuração

### Chave de API do DeepSeek

- Insira sua [Chave de API do DeepSeek](https://platform.deepseek.com/api_keys) no campo de texto
- A chave é exibida em modo oculto (não é mostrada em texto claro)
- O formato é validado em tempo real abaixo do campo: deve começar com `sk-`, seguido de 32 caracteres alfanuméricos
- Após a validação do formato, clique no botão "Testar API" para verificar a validade da chave
- Se o teste for bem-sucedido, uma caixa de diálogo com os detalhes do saldo será exibida e o botão OK ficará habilitado

> **É necessário passar no teste da API** para que o botão OK possa salvar. Se a chave estiver vazia (não deseja exibir), é possível salvar diretamente.

### Intervalo de atualização (segundos)

- Valor padrão: **60** segundos
- Faixa: 1 ~ 31536000
- Consulta o saldo da API do DeepSeek automaticamente a cada intervalo definido
- O intervalo de atualização deve ser maior que o tempo limite da requisição; caso contrário, não será possível salvar

### Tempo limite da requisição (segundos)

- Valor padrão: **10** segundos
- Faixa: 3 ~ 60 segundos
- Tempo máximo de espera por uma requisição à API; se excedido, a requisição é considerada falha
- Recomenda-se ajustar conforme as condições da rede; aumente o valor se a rede estiver ruim

### Quantidade de registros no histórico

- Valor padrão: **1000** registros
- Faixa: 0 ~ 10000 (definir como 0 desativa o histórico)
- Um novo registro é adicionado somente quando há uma variação real do saldo (diferença > 0,001), evitando redundância
- Registros antigos que excederem o limite são removidos automaticamente

### Atualização automática

- Marcada por padrão
- Quando marcada, a lista do histórico é atualizada automaticamente a cada segundo
- Clique em qualquer lugar da lista para desativar a atualização automática

### Idioma de exibição

- Valor padrão: "Automático" (detectado com base no idioma do sistema)
- Suporta 12 idiomas: 简体中文, 繁體中文, 日本語, English, Deutsch, עברית, Magyar, Italiano, Polski, Português (Brasil), Русский, Türkçe
- A troca de idioma tem efeito imediato na caixa de diálogo, mas só é salva permanentemente ao clicar no botão "OK"

### Limpar histórico

- Clique no botão "Limpar histórico" e confirme para apagar todos os registros do histórico
- Esta operação é irreversível

## Compilação

### Pré-requisitos

- Visual Studio 2026 (conjunto de ferramentas v145)
- Windows 10 SDK ou superior
- A opção de compilação `/utf-8` deve estar habilitada (necessário para código-fonte em chinês)

### Compilação rápida (recomendado)

```batch
build.bat
```

Compila automaticamente a configuração Release de acordo com a arquitetura do sistema e copia a DLL gerada para `C:\TrafficMonitor\plugins\`.

> **Nota**: Ajuste a variável `PLUGINDIR` no `build.bat` de acordo com o caminho real de instalação do TrafficMonitor.

### Compilação manual

```batch
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

### Arquivos de saída

| Plataforma | Configuração | Arquivo de saída                     |
|------------|--------------|--------------------------------------|
| x86        | Debug        | `Debug\DeepSeekDeskBand.dll`         |
| x86        | Release      | `Release\DeepSeekDeskBand.dll`       |
| x64        | Debug        | `x64\Debug\DeepSeekDeskBand64.dll`   |
| x64        | Release      | `x64\Release\DeepSeekDeskBand64.dll` |

## Privacidade e segurança de dados

Este plugin leva muito a sério a privacidade do usuário e a segurança dos dados:

- **Chave de API**: armazenada localmente e criptografada com DPAPI do Windows no arquivo de configuração `DeepSeekDeskBand.dat`, acessível apenas pelo usuário atual do Windows. A chave nunca é enviada a terceiros.
- **Dados de saldo**: obtidos exclusivamente via HTTPS da API oficial do DeepSeek (`api.deepseek.com`), sem passar por nenhum servidor intermediário.
- **Histórico**: os registros de variação do saldo são criptografados com DPAPI e armazenados localmente no arquivo `DeepSeekDeskBand_History.dat`, acessível apenas pelo usuário atual do Windows.
- **Requisições de rede**: apenas requisições HTTPS são enviadas para `api.deepseek.com` (Authorization: Bearer Token), sem solicitar outros domínios.
- **Sem coleta de dados**: este plugin não coleta nem envia quaisquer dados do usuário ou estatísticas de uso.
- **Cache de ícone**: o ícone (favicon.ico) é baixado de `www.deepseek.com` apenas uma vez, na primeira abertura da caixa de diálogo de configuração, e armazenado em cache no diretório de configuração do plugin.

## Notas técnicas

- Interface do plugin: TrafficMonitor API v7
- Sem dependência do MFC, implementado em C++ Win32 puro
- Gerenciamento da instância do plugin via padrão Singleton
- Cliente HTTPS baseado em WinHTTP
- Limiar de detecção de variação do saldo > 0,001, reduzindo registros redundantes

## Licença

Copyright (c) 2026 KagurazakaYashi (KagurazakaMiyabi)

Este projeto é distribuído sob a licença Mulan PSL v2. Você pode usar este software desde que cumpra os termos da Mulan PSL v2. Cópia da licença disponível em: http://license.coscl.org.cn/MulanPSL2

Este software é fornecido "no estado em que se encontra", sem qualquer garantia expressa ou implícita, incluindo, entre outras, garantias de comercialização, adequação a um propósito específico e não violação.
