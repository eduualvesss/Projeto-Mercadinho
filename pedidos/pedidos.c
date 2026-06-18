/* ============================================================
 *  pedidos.c — Módulo de vendas (Ponto de Venda)
 * ============================================================ */
#include "pedidos.h"
#include "../clientes/clientes.h"
#include "../produtos/produtos.h"
#include <stdlib.h>
#include <string.h>

static FILE *ped_abrir(const char *modo) {
    char caminho[300];
    caminho_arquivo(caminho, ARQ_PEDIDOS);
    return fopen(caminho, modo);
}

int ped_proximo_numero(void) {
    FILE *f = ped_abrir("rb");
    if (!f) return 1;
    int max = 0; Pedido p;
    while (fread(&p, sizeof(Pedido), 1, f) == 1)
        if (p.numero > max) max = p.numero;
    fclose(f);
    return max + 1;
}

void imprimir_cupom(const Pedido *ped) {
    printf("\n ========================================\n");
    printf("       MERCADINHO DO PORTO\n");
    printf(" ========================================\n");
    printf(" Pedido Nº : %d\n", ped->numero);
    printf(" Data      : %s\n", ped->data_hora);
    printf(" Cliente   : %s\n", ped->nome_cliente);
    printf(" ----------------------------------------\n");
    printf(" %-20s %4s %10s\n", "PRODUTO", "QTD", "SUBTOTAL");
    printf(" ----------------------------------------\n");
    for (int i = 0; i < ped->total_itens; i++) {
        const ItemPedido *it = &ped->itens[i];
        printf(" %-20s %4d R$ %8.2f\n", it->nome_produto, it->quantidade, it->subtotal);
    }
    printf(" ----------------------------------------\n");
    printf(" Total          : R$ %.2f\n", ped->total);
    if (ped->desconto > 0)
        printf(" Desconto       : R$ %.2f\n", ped->desconto);
    printf(" Total Final    : R$ %.2f\n", ped->total_final);
    printf(" Pagamento      : %s\n",      ped->forma_pagamento);
    printf(" ========================================\n");
    printf("        Obrigado pela preferência!\n");
    printf(" ========================================\n\n");
}

void realizar_venda(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║         PONTO DE VENDA                   ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");

    Pedido ped;
    memset(&ped, 0, sizeof(Pedido));
    ped.numero      = ped_proximo_numero();
    ped.status      = 1;
    data_hora_agora(ped.data_hora);

    printf(" Código do cliente (0 para venda sem cadastro): ");
    int cod_cli; scanf("%d", &cod_cli); limpar_buffer();
    if (cod_cli > 0) {
        Cliente c;
        if (cli_buscar_por_codigo(cod_cli, &c)) {
            if (!c.ativo) {
                printf("\n " RED "[BLOQUEADO] Cliente INATIVO nao pode realizar compras!" RESET "\n");
                pausar(); return;
            }
            ped.codigo_cliente = cod_cli;
            strncpy(ped.nome_cliente, c.nome, MAX_NOME_CLI - 1);
        } else {
            printf(" Cliente nao encontrado. Continuando sem cadastro.\n");
            strcpy(ped.nome_cliente, "CONSUMIDOR FINAL");
        }
    } else {
        strcpy(ped.nome_cliente, "CONSUMIDOR FINAL");
    }

    int continuar = 1;
    while (continuar && ped.total_itens < MAX_ITENS_PEDIDO) {
        printf("\n --- Item %d ---\n", ped.total_itens + 1);
        printf(" Código do produto (0 para finalizar): ");
        int cod_prod; scanf("%d", &cod_prod); limpar_buffer();
        if (cod_prod == 0) break;

        Produto prod;
        if (!prod_buscar_por_codigo(cod_prod, &prod)) { printf(" Produto não encontrado!\n"); continue; }
        if (!prod.ativo) { printf(" Produto inativo!\n"); continue; }
        if (prod.quantidade == 0) { printf(" Sem estoque!\n"); continue; }

        printf(" Produto: %s | Estoque: %d | Preço: R$ %.2f\n",
               prod.nome, prod.quantidade, prod.preco_venda);
        int qtd = ler_inteiro_positivo(" Quantidade (> 0): ");
        if (qtd <= 0 || qtd > prod.quantidade) { printf(" Quantidade invalida!\n"); continue; }

        ItemPedido *item = &ped.itens[ped.total_itens];
        item->codigo_produto = cod_prod;
        strncpy(item->nome_produto, prod.nome, MAX_NOME_PROD - 1);
        item->quantidade     = qtd;
        item->preco_unitario = prod.preco_venda;
        item->subtotal       = qtd * prod.preco_venda;

        ped.total += item->subtotal;
        ped.total_itens++;

        prod.quantidade -= qtd;
        prod_atualizar(cod_prod, &prod);
        registrar_movimentacao(cod_prod, "SAIDA", qtd);

        printf(" " GREEN "✔ Item adicionado. Subtotal: R$ %.2f" RESET "\n", item->subtotal);
        printf(" Total parcial: R$ %.2f\n", ped.total);
    }

    if (ped.total_itens == 0) { printf("\n Nenhum item. Venda cancelada.\n"); pausar(); return; }

    printf("\n Total: R$ %.2f\n", ped.total);
    ped.desconto = ler_float_positivo(" Desconto (R$, 0 para nenhum): ");
    if (ped.desconto > ped.total) {
        printf(" Desconto nao pode ser maior que o total! Desconto zerado.\n");
        ped.desconto = 0.0f;
    }
    ped.total_final = ped.total - ped.desconto;

    printf(" Forma de pagamento:\n [1] Dinheiro  [2] Cartão Débito  [3] Cartão Crédito  [4] PIX\n Opção: ");
    int pag; scanf("%d", &pag); limpar_buffer();
    switch (pag) {
        case 1: strcpy(ped.forma_pagamento, "Dinheiro");       break;
        case 2: strcpy(ped.forma_pagamento, "Cartão Débito");  break;
        case 3: strcpy(ped.forma_pagamento, "Cartão Crédito"); break;
        case 4: strcpy(ped.forma_pagamento, "PIX");            break;
        default:strcpy(ped.forma_pagamento, "Não informado");  break;
    }
    ped.status = 2;

    FILE *f = ped_abrir("ab");
    if (f) { fwrite(&ped, sizeof(Pedido), 1, f); fclose(f); }

    char det[160];
    sprintf(det, "Pedido #%d finalizado | Cliente: %s | Total: R$ %.2f",
            ped.numero, ped.nome_cliente, ped.total_final);
    log_evento("PEDIDO", "VENDA", det);

    imprimir_cupom(&ped);
    pausar();
}

void listar_pedidos(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════════════════════════╗\n");
    printf(" ║              HISTÓRICO DE PEDIDOS                            ║\n");
    printf(" ╠════════╦══════════════════════╦══════════╦═══════════════════╣\n");
    printf(" ║ Nº PED ║ DATA/HORA            ║ TOTAL(R$)║ CLIENTE           ║\n");
    printf(" ╠════════╬══════════════════════╬══════════╬═══════════════════╣\n");

    FILE *f = ped_abrir("rb");
    if (!f) {
        printf(" ║ Nenhum pedido registrado.                                    ║\n");
        printf(" ╚══════════════════════════════════════════════════════════════╝\n");
        pausar(); return;
    }
    Pedido p; int count = 0;
    while (fread(&p, sizeof(Pedido), 1, f) == 1) {
        printf(" ║ %-6d ║ %-20s ║ %-8.2f ║ %-17.17s ║\n",
               p.numero, p.data_hora, p.total_final, p.nome_cliente);
        count++;
    }
    fclose(f);
    printf(" ╚════════╩══════════════════════╩══════════╩═══════════════════╝\n");
    printf(" Total: %d pedido(s).\n", count);
    pausar();
}

void relatorio_vendas(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║       RELATÓRIO DE VENDAS                ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");

    FILE *f = ped_abrir("rb");
    if (!f) { printf(" Nenhum pedido encontrado.\n"); pausar(); return; }

    int total_pedidos = 0, total_itens = 0;
    float total_bruto = 0.0f, total_desconto = 0.0f, total_liquido = 0.0f;
    Pedido p;
    while (fread(&p, sizeof(Pedido), 1, f) == 1) {
        if (p.status != 2) continue;
        total_pedidos++;
        total_itens    += p.total_itens;
        total_bruto    += p.total;
        total_desconto += p.desconto;
        total_liquido  += p.total_final;
    }
    fclose(f);

    printf(" Pedidos finalizados     : %d\n", total_pedidos);
    printf(" Total de itens vendidos : %d\n", total_itens);
    printf(" Receita bruta (R$)      : %.2f\n", total_bruto);
    printf(" Total descontos (R$)    : %.2f\n", total_desconto);
    printf(" Receita líquida (R$)    : %.2f\n", total_liquido);
    if (total_pedidos > 0)
        printf(" Ticket médio (R$)       : %.2f\n", total_liquido / total_pedidos);
    pausar();
}

void exportar_pedidos_txt(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║   EXPORTAR PEDIDOS PARA ARQUIVO .TXT     ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");

    FILE *dat = ped_abrir("rb");
    if (!dat) { printf("\n Nenhum pedido registrado.\n"); pausar(); return; }

    char caminho[300]; caminho_arquivo(caminho, "pedidos_export.txt");
    FILE *txt = fopen(caminho, "w");
    if (!txt) { fclose(dat); printf("\n ERRO!\n"); pausar(); return; }

    char agora[20]; data_hoje(agora);
    fprintf(txt, "SISTEMA MERCADINHO v%s — HISTORICO DE PEDIDOS\nGerado em: %s\n\n", VERSAO, agora);
    sep_txt(txt, '-', 70);
    fprintf(txt, "%-6s %-20s %-10s %-12s %s\n", "Nº", "DATA/HORA", "TOTAL(R$)", "PAGAMENTO", "CLIENTE");
    sep_txt(txt, '-', 70);

    Pedido p; int count = 0; float receita = 0.0f;
    while (fread(&p, sizeof(Pedido), 1, dat) == 1) {
        fprintf(txt, "%-6d %-20s %-10.2f %-12s %s\n",
                p.numero, p.data_hora, p.total_final, p.forma_pagamento, p.nome_cliente);
        if (p.status == 2) receita += p.total_final;
        count++;
    }
    sep_txt(txt, '-', 70);
    fprintf(txt, "Total: %d pedido(s) | Receita acumulada: R$ %.2f\n", count, receita);

    fclose(dat); fclose(txt);
    log_evento("PEDIDO", "EXPORT", "Exportacao de pedidos para TXT realizada");
    printf("\n " GREEN "Arquivo gerado: %s" RESET "\n", caminho);
    pausar();
}

void submenu_vendas(void) {
    int op;
    do {
        limpar_tela();
        printf("\n ╔══════════════════════════════════════════╗\n");
        printf(" ║       VENDAS E RELATÓRIOS                ║\n");
        printf(" ╠══════════════════════════════════════════╣\n");
        printf(" ║ [1] Registrar venda (PDV)                ║\n");
        printf(" ║ [2] Listar pedidos                       ║\n");
        printf(" ║ [3] Relatório de vendas                  ║\n");
        printf(" ║ [4] Exportar pedidos para TXT             ║\n");
        printf(" ║ [0] Voltar ao menu principal             ║\n");
        printf(" ╚══════════════════════════════════════════╝\n\n");
        printf(" Opção: "); scanf("%d", &op); limpar_buffer();

        switch (op) {
            case 1: realizar_venda();      break;
            case 2: listar_pedidos();      break;
            case 3: relatorio_vendas();    break;
            case 4: exportar_pedidos_txt();break;
            case 0: break;
            default: printf("\n Opção inválida!\n"); pausar();
        }
    } while (op != 0);
}
