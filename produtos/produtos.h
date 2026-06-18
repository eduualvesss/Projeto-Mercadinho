/* ============================================================
 *  produtos.h — Módulo de controle de estoque
 * ============================================================ */
#ifndef PRODUTOS_H
#define PRODUTOS_H

#include "comum.h"

void prod_classificar(Produto *p);
const char *prod_label(int prioridade);
const char *prod_cor(int prioridade);

int  prod_proximo_codigo(void);
int  prod_total_registros(void);
int  prod_buscar_por_codigo(int codigo, Produto *dest);
int  prod_atualizar(int codigo, const Produto *novo);
void prod_exibir_ficha(const Produto *p);

void registrar_movimentacao(int cod, const char *tipo, int qtd);

void cadastrar_produto(void);
void listar_produtos(int apenas_ativos);
void consultar_produto(void);
void editar_produto(void);
void excluir_produto(void);
void movimentar_estoque(void);
void relatorio_produtos(void);
void exportar_produtos_txt(void);
void consultar_historico(void);

void submenu_estoque(void);

#endif /* PRODUTOS_H */
