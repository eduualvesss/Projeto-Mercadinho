/* ============================================================
 *  comum.h — Tipos, constantes e utilitários compartilhados
 *  Sistema Mercadinho — Módulos: Clientes | Produtos | Pedidos
 * ============================================================ */
#ifndef COMUM_H
#define COMUM_H

#include <stdio.h>

/* ==================== CORES ANSI ========================== */
#define RED    "\x1b[31m"
#define YELLOW "\x1b[33m"
#define GREEN  "\x1b[32m"
#define CYAN   "\x1b[36m"
#define BOLD   "\x1b[1m"
#define RESET  "\x1b[0m"

/* ==================== VERSÃO ============================== */
#define VERSAO "2.0.0"

/* ==================== ARQUIVO DE CONFIGURAÇÃO ============== */
#define ARQ_CONFIG "mercadinho.cfg"

/* ==================== CONSTANTES — CLIENTE ================ */
#define MAX_NOME_CLI   80
#define MAX_CPF        15
#define MAX_EMAIL      60
#define MAX_TELEFONE   20
#define MAX_ENDERECO  100
#define MAX_CIDADE     50
#define MAX_ESTADO      3
#define MAX_CEP        10
#define MAX_OBS_CLI   200
#define ARQ_CLIENTES  "clientes.dat"
#define ARQ_CLI_TXT   "clientes_export.txt"

/* ==================== CONSTANTES — PRODUTO ================ */
#define MAX_NOME_PROD  60
#define MAX_CATEGORIA  35
#define MAX_UNIDADE    10
#define MAX_OBS_PROD  200
#define ARQ_PRODUTOS  "produtos.dat"
#define ARQ_HIST      "historico.dat"
#define ARQ_PROD_TXT  "produtos_export.txt"
#define LIMITE_CRITICO 3
#define LIMITE_ATENCAO 7

/* ==================== CONSTANTES — PEDIDO ================= */
#define MAX_ITENS_PEDIDO 20
#define MAX_PEDIDOS      200
#define ARQ_PEDIDOS      "pedidos.dat"

/* ==================== CONSTANTES — LOG / SNAPSHOT ========== */
#define ARQ_LOG_DIARIO   "log_diario.txt"

/* ==================== ESTRUTURA — CLIENTE ================= */
typedef struct {
    int   codigo;
    char  nome[MAX_NOME_CLI];
    char  cpf[MAX_CPF];
    char  rg[MAX_CPF];
    char  data_nascimento[12];
    char  email[MAX_EMAIL];
    char  telefone[MAX_TELEFONE];
    char  celular[MAX_TELEFONE];
    char  endereco[MAX_ENDERECO];
    char  numero[10];
    char  complemento[40];
    char  bairro[50];
    char  cidade[MAX_CIDADE];
    char  estado[MAX_ESTADO];
    char  cep[MAX_CEP];
    float limite_credito;
    int   tipo;   /* 1=PF  2=PJ  */
    int   ativo;  /* 1=ativo 0=inativo */
    char  data_cadastro[20];
    char  observacoes[MAX_OBS_CLI];
} Cliente;

/* ==================== ESTRUTURA — PRODUTO ================= */
typedef struct {
    int   codigo;
    char  nome[MAX_NOME_PROD];
    char  categoria[MAX_CATEGORIA];
    char  unidade[MAX_UNIDADE];
    float peso_volume;
    int   quantidade;
    float preco_custo;
    float preco_venda;
    int   dia_val, mes_val, ano_val;
    int   dias_restantes;
    int   prioridade; /* 0=Vencido 1=Crítico 2=Atenção 3=OK */
    int   ativo;
    char  data_cadastro[20];
    char  observacoes[MAX_OBS_PROD];
} Produto;

/* ==================== ESTRUTURA — MOVIMENTAÇÃO ============ */
typedef struct {
    int  codigo_produto;
    char tipo_acao[15]; /* CADASTRO | ENTRADA | SAIDA | EXCLUSAO | EDICAO */
    int  quantidade;
    char data_hora[25];
} Movimentacao;

/* ==================== ESTRUTURA — ITEM DE PEDIDO ========== */
typedef struct {
    int   codigo_produto;
    char  nome_produto[MAX_NOME_PROD];
    int   quantidade;
    float preco_unitario;
    float subtotal;
} ItemPedido;

/* ==================== ESTRUTURA — PEDIDO ================== */
typedef struct {
    int        numero;
    int        codigo_cliente;
    char       nome_cliente[MAX_NOME_CLI];
    ItemPedido itens[MAX_ITENS_PEDIDO];
    int        total_itens;
    float      total;
    float      desconto;
    float      total_final;
    char       forma_pagamento[20];
    char       data_hora[25];
    int        status; /* 0=Cancelado 1=Aberto 2=Finalizado */
} Pedido;

/* ============================================================
 *  CONFIGURAÇÃO DE DIRETÓRIO DE DADOS
 * ============================================================ */
void        config_inicializar(void);
const char *config_dir_dados(void);
void        caminho_arquivo(char *dest, const char *nome_arquivo);

/* ============================================================
 *  UTILITÁRIOS DE TELA / ENTRADA
 * ============================================================ */
void  limpar_tela(void);
void  pausar(void);
void  limpar_buffer(void);
void  ler_string(const char *prompt, char *dest, int max);
void  data_hoje(char *buf);
void  data_hora_agora(char *buf);
void  data_hoje_arquivo(char *buf);
void  sep_txt(FILE *f, char c, int n);
void  ler_numeros_exatos(const char *prompt, char *dest, int max, int digitos_exatos);
int   ler_inteiro_positivo(const char *prompt);
float ler_float_positivo(const char *prompt);
void  ler_data_validade(int *dia, int *mes, int *ano);
void  formatar_cpf(const char *entrada, char *saida);
void  para_minusculas(const char *origem, char *destino);

/* ============================================================
 *  LOG DIÁRIO E SNAPSHOT CONSOLIDADO
 * ============================================================ */
void log_evento(const char *modulo, const char *acao, const char *detalhe);
void gerar_snapshot_do_dia(void);

#endif /* COMUM_H */
