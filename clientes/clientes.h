/* ============================================================
 *  clientes.h — Módulo de cadastro de clientes
 * ============================================================ */
#ifndef CLIENTES_H
#define CLIENTES_H

#include "../comum/comum.h"

int  cli_proximo_codigo(void);
int  cli_total_registros(void);
int  cli_buscar_por_codigo(int codigo, Cliente *dest);
void cli_exibir_ficha(const Cliente *c);

void cadastrar_cliente(void);
void listar_clientes(int apenas_ativos);
void consultar_cliente(void);
void editar_cliente(void);
void excluir_cliente(void);
void relatorio_clientes(void);
void exportar_clientes_txt(void);

void submenu_clientes(void);

#endif /* CLIENTES_H */
