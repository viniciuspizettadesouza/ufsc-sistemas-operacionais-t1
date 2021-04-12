#ifndef IMOBILIARIA_H   // guardas de cabeçalho, impedem inclusões cíclicas
#define IMOBILIARIA_H

long random_at_most(long max);
void *t_function_corretor_remove_imovel(void *arg); // declaração de uma função
void *t_function_corretor_adiciona_imovel(void *arg);     
void *t_function_corretor_move_imovel(void *arg);
void *t_function_inquilino_pega_imovel(void *arg);
void *t_function_corretor_decideAcao(void *arg);

#endif
