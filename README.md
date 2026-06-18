# 🏪 Sistema Mercadinho

Sistema de gestão para um pequeno mercado, feito em **C puro**, com módulos separados de **Clientes**, **Produtos/Estoque** e **Pedidos (Ponto de Venda)**.

## ✨ Funcionalidades

- **Clientes**: cadastro (PF/PJ), consulta por código/nome/CPF, edição, inativação/exclusão, relatório e exportação em `.txt`.
- **Produtos/Estoque**: cadastro com controle de validade (classificação automática em OK / Atenção / Crítico / Vencido), movimentação de entrada/saída, histórico de auditoria, relatório gerencial e exportação em `.txt`.
- **Pedidos (PDV)**: ponto de venda com múltiplos itens, baixa automática de estoque, desconto, múltiplas formas de pagamento, emissão de cupom, histórico e relatório de vendas.
- **Diretório de dados configurável**: na primeira execução, o sistema pergunta onde salvar os arquivos `.dat`. A escolha é lembrada nas próximas execuções (arquivo `mercadinho.cfg`).
- **Log diário automático**: todo evento relevante (cadastros, vendas, edições, exclusões) é registrado em `log_diario.txt`.
- **Snapshot consolidado do dia**: gera, sob demanda, um resumo de todo o sistema (`snapshot_AAAA-MM-DD.txt` e `.dat`) com clientes, produtos e os pedidos feitos no dia.

## 📁 Estrutura do projeto

```
SISTEMA-MERCADINHO/
│
├── README.md          # Este arquivo
├── main.c              # Menu principal que liga todo o sistema
│
├── comum.h / comum.c    # Tipos, constantes e utilitários compartilhados
│                        # (config de diretório, log diário, snapshot)
│
├── clientes.h / clientes.c   # Cadastro de clientes
├── produtos.h  / produtos.c  # Controle de estoque
├── pedidos.h   / pedidos.c   # Ponto de venda (PDV)
│
└── data/                # Pasta padrão sugerida para os dados
    ├── clientes.dat
    ├── produtos.dat
    ├── pedidos.dat
    ├── historico.dat
    └── log_diario.txt
```

> Os arquivos `.dat`, `.txt` gerados e o `mercadinho.cfg` **não** ficam versionados no Git (veja `.gitignore`) — eles são criados automaticamente quando o programa roda.

## 🔧 Como compilar

```bash
gcc -Wall -Wextra -o mercadinho main.c clientes.c produtos.c pedidos.c comum.c
```

## ▶️ Como executar

```bash
./mercadinho
```

Na primeira execução, o sistema vai perguntar em qual pasta deseja salvar os dados. Pressione **ENTER** para usar a pasta padrão `data/`, ou digite o caminho desejado.

## 🗂️ Arquivos gerados automaticamente

| Arquivo                       | Conteúdo                                              |
|--------------------------------|--------------------------------------------------------|
| `clientes.dat`                 | Registros binários de clientes                         |
| `produtos.dat`                 | Registros binários de produtos                         |
| `pedidos.dat`                  | Registros binários de pedidos (vendas)                 |
| `historico.dat`                | Movimentações de estoque (auditoria)                   |
| `log_diario.txt`                | Log de todos os eventos do sistema, acumulado por dia   |
| `snapshot_AAAA-MM-DD.txt/.dat`  | Resumo consolidado do dia (gerado pela opção [5] do menu) |
| `clientes_export.txt` / `produtos_export.txt` / `pedidos_export.txt` | Exportações manuais em texto |

## 📋 Menu principal

```
[1] PONTO DE VENDA
[2] CONTROLE DE ESTOQUE
[3] VENDAS E RELATÓRIOS
[4] CADASTRO DE CLIENTES
[5] GERAR SNAPSHOT DO DIA
[0] SAIR DO SISTEMA
```

## 🛠️ Tecnologias

- Linguagem **C** (padrão C99)
- Persistência em arquivos binários (`fread`/`fwrite`)
- Sem dependências externas — compila com `gcc` em Windows, Linux ou macOS

## 📝 Licença

Projeto livre para fins de estudo.