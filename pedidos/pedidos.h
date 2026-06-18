/* ============================================================
 *  pedidos.h — Módulo de vendas (Ponto de Venda)
 * ============================================================ */
#ifndef PEDIDOS_H
#define PEDIDOS_H

#include "../comum/comum.h"

int  ped_proximo_numero(void);
void imprimir_cupom(const Pedido *ped);

void realizar_venda(void);
void listar_pedidos(void);
void relatorio_vendas(void);
void exportar_pedidos_txt(void);

void submenu_vendas(void);

#endif /* PEDIDOS_H */
