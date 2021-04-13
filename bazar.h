#ifndef BAZAR_H // guardas de cabeçalho, impedem inclusões cíclicas
#define BAZAR_H

long random_at_most(long max);
void *t_function_voluntario_remove_roupa(void *arg); // declaração de uma função
void *t_function_voluntario_doa_roupa(void *arg);
void *t_function_voluntario_move_roupa(void *arg);
void *t_function_cliente_pega_roupa(void *arg);
void *t_function_voluntario_decideAcao(void *arg);

#endif