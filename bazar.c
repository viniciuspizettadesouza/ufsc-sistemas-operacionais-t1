#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "bazar.h"
#include "include/arraylist.c"

#define NUM_THREADS_CLIENTE 10
#define NUM_THREADS_VOLUNTARIO 5

pthread_mutex_t mutex_doa;
pthread_mutex_t mutex_compra;
pthread_mutex_t mutex_move;

pthread_mutex_t mutex_aluga;
pthread_mutex_t mutex_entrega;

typedef struct argumentos
{
  arraylist *listaCompra;
  arraylist *listaReparo;
  arraylist *listaNovos;
  int nThread;

} argumentos;

typedef struct roupa_t
{
  unsigned int codigo;
  char *modelo;
  double preco;
  char *tamanho;

} roupa_t;

long random_at_most(long max)
{
  unsigned long

      num_bins = (unsigned long)max + 1,
      num_rand = (unsigned long)RAND_MAX + 1,
      bin_size = num_rand / num_bins,
      defect = num_rand % num_bins;

  long x;
  do
  {
    x = random();
  }

  // This is carefully written not to overflow
  while (num_rand - defect <= (unsigned long)x);

  // Truncated division is intentional
  return x / bin_size;
}

void *t_function_cliente_compra_roupa(void *arg)
{
  argumentos *args;
  args = (argumentos *)arg;
  struct roupa_t *roupaComprar;
  pthread_mutex_lock(&mutex_compra);
  roupaComprar = (roupa_t *)arraylist_pop(args->listaCompra);
  pthread_mutex_unlock(&mutex_compra);
  printf("Cliente %i comprou uma peça de roupa: %i \n", args->nThread, roupaComprar->codigo);
  return 0;
}

void *t_function_cliente_doa_roupa(void *arg)
{
  argumentos *args;
  args = (argumentos *)arg;
  int index = (int)random_at_most(args->listaNovos->size - 1);

  if (args->listaCompra->size > 0)
  {
    struct roupa_t *doarRoupa;
    doarRoupa = (roupa_t *)arraylist_get(args->listaNovos, index);

    pthread_mutex_lock(&mutex_doa);
    arraylist_add(args->listaCompra, doarRoupa);
    arraylist_remove(args->listaNovos, index);
    pthread_mutex_unlock(&mutex_doa);
    printf("Cliente %i doou uma roupa: %i \n", args->nThread, doarRoupa->codigo);
  }
  return 0;
}

void *t_function_voluntario_move_roupa(void *arg)
{
  argumentos *args;
  args = (argumentos *)arg;

  if (args->listaReparo->size > 0)
  {
    struct roupa_t *roupaEntregue;
    pthread_mutex_lock(&mutex_move);
    roupaEntregue = (roupa_t *)arraylist_pop(args->listaReparo);
    arraylist_add(args->listaCompra, roupaEntregue);
    pthread_mutex_unlock(&mutex_move);
    printf("Voluntario %i disponibilizou o roupa: %i \n", args->nThread, roupaEntregue->codigo);
    //imoveis_disponiveis_->insert_sorted(imoveis_entregues_->pop_back()); // voluntario remove roupa da lista de entregues e insere na lista de disp.
  }
  return 0;
}

void *t_function_cliente_pega_roupa(void *arg)
{
  while (1)
  {
    argumentos *args;
    args = (argumentos *)arg;

    int index = (int)random_at_most(args->listaCompra->size - 1); //-1 pois eh o tamanho e n a posicao do array
    struct roupa_t *roupaAlugado;
    roupaAlugado = (roupa_t *)arraylist_get(args->listaCompra, index);
    pthread_mutex_lock(&mutex_aluga);
    arraylist_remove(args->listaCompra, index); //remove do pool de novos
    //roupa* roupaAlugado = imoveis_disponiveis_->pop_back();
    pthread_mutex_unlock(&mutex_aluga);
    printf("Cliente %i alugou o roupa: %i \n", args->nThread, roupaAlugado->codigo);
    sleep(5);
    pthread_mutex_lock(&mutex_entrega);
    arraylist_add(args->listaReparo, roupaAlugado);
    pthread_mutex_unlock(&mutex_entrega);
    //imoveis_entregues_->push_front(roupaAlugado);
    printf("Cliente %i entregou o roupa: %i \n", args->nThread, roupaAlugado->codigo);
    sleep(5);
  }
}

void *t_function_voluntario_decideAcao(void *arg)
{
  long decisao;
  while (1)
  {                              //loop infinito
    decisao = random_at_most(2); // na teoria esse cara vai ser um dos 3, toda vez

    switch (decisao)
    {
    case 0:
      t_function_voluntario_adiciona_roupa(arg);
      break;
    case 1:
      t_function_voluntario_remove_roupa(arg);
      break;
    case 2:
      t_function_voluntario_move_roupa(arg);
      break;
    default:
      break;
    }
    sleep(3);
  }
}

int main(int argc, char *argv[])
{

  // ArrayListroupa* imoveis_disponiveis_ = new ArrayListRoupa();
  arraylist *imoveis_disponiveis_;
  imoveis_disponiveis_ = arraylist_create();

  //ArrayListRoupa* imoveis_entregues_ = new ArrayListRoupa();
  arraylist *imoveis_entregues_;
  imoveis_entregues_ = arraylist_create();

  //ArrayListRoupa* imoveis_novos_ = new ArrayListRoupa();
  arraylist *imoveis_novos_;
  imoveis_novos_ = arraylist_create();
  int i;
  for (i = 0; i < 100; i++)
  { //inicializa a lista de disponiveis com 20 imoveis
    struct roupa_t *disp = malloc(sizeof(roupa_t));
    disp->codigo = i + 1; //nao pode ser random aqui porque deve ser UNIQUE
    char *tamanho;
    tamanho = (char *)'a'; //apenas um placeholder pro tamanho
    disp->tamanho = tamanho;
    disp->preco = (double)random_at_most(1000); //gera valor aleatorio pro roupa
    arraylist_add(imoveis_disponiveis_, disp);
    //imoveis_disponiveis_->push_back(disp); //joga roupa no array de imoveis disp
  }

  for (i = 0; i < 100; i++)
  { //inicializa a lista de imoveis novos a serem inseridos na lista de disponveis pelos voluntarios

    struct roupa_t *novo = malloc(sizeof(roupa_t));
    novo->codigo = i + 101; //nao pode ser random aqui porque deve ser UNIQUE, 21 pois eh o primeiro codigo apos os ja disponveis
    char *tamanho;
    tamanho = (char *)'a'; //apenas um placeholder pro tamanho
    novo->tamanho = tamanho;
    novo->preco = (double)random_at_most(1000); //gera valor aleatorio pro roupa entre 0 e 1000
    arraylist_add(imoveis_novos_, novo);        //joga roupa no array de imoveis novos
  }
  sleep(3);
  pthread_t tInq[NUM_THREADS_CLIENTE];    //array das threads cliente
  pthread_t tCor[NUM_THREADS_VOLUNTARIO]; //array das threads voluntario

  long *nomet1 = (long *)malloc(sizeof(long));
  *nomet1 = 1;

  argumentos *arrayArgs[10];

  for (i = 0; i < 10; i++)
  {
    struct argumentos *args = malloc(sizeof(argumentos));
    args->listaCompra = imoveis_disponiveis_;
    args->listaReparo = imoveis_entregues_;
    args->listaNovos = imoveis_novos_;
    args->nThread = i + 1;
    arrayArgs[i] = args;
  }

  while (1)
  {
    for (i = 0; i < 5; i++)
    {
      printf("Voluntario %i se junta a imobiliaria \n", i + 1);
      pthread_create(&tCor[i], NULL, t_function_voluntario_decideAcao, (void *)arrayArgs[i]);
    }
    for (i = 5; i < 10; i++)
    {
      printf("Cliente %i se torna cliente da imobiliaria \n", i);
      pthread_create(&tInq[i], NULL, t_function_cliente_pega_roupa, (void *)arrayArgs[i]); // 1 eh numero da thread
    }
    sleep(9999);
  }
  return 0;
}