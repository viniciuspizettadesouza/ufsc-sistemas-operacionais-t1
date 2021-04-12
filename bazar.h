#ifndef BAZAR_H   // guardas de cabeçalho, impedem inclusões cíclicas
#define BAZAR_H

long random_at_most(long max);
void *t_function_cliente_compra_roupa(void *arg); // declaração de uma função
void *t_function_corretor_adiciona_imovel(void *arg);     
void *t_function_corretor_move_imovel(void *arg);
void *t_function_inquilino_pega_imovel(void *arg);
void *t_function_corretor_decideAcao(void *arg);

#endif
