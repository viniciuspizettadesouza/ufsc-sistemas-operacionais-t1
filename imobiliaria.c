

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "imobiliaria.h"
#include "include/arraylist.c"
 
  #define NUM_THREADS_INQ 10
  #define NUM_THREADS_CORRETOR 5

  pthread_mutex_t mutex_adiciona;
  pthread_mutex_t mutex_remove;
  pthread_mutex_t mutex_move;
  //sao os mutexes que controlarao as regioes criticas
  pthread_mutex_t mutex_aluga;
  pthread_mutex_t mutex_entrega;

   typedef struct argumentos {
  arraylist* listaDisponveis;
  arraylist* listaEntregues;
  arraylist* listaNovos;
  int nThread;

} argumentos;

typedef struct imovel_t {
  unsigned int codigo;
  char *endereco;
  double preco;
  char *bairro;

} imovel_t;

  long random_at_most(long max){
  unsigned long

    num_bins = (unsigned long) max + 1,
    num_rand = (unsigned long) RAND_MAX + 1,
    bin_size = num_rand / num_bins,
    defect   = num_rand % num_bins;

  long x;
  do {
   x = random();
  }
  // This is carefully written not to overflow
  while (num_rand - defect <= (unsigned long)x);

  // Truncated division is intentional
  return x/bin_size;
}

void *t_function_corretor_remove_imovel(void *arg){
  argumentos *args;
  args = (argumentos *) arg;
  struct imovel_t* imovelRemover;
  pthread_mutex_lock(&mutex_remove); //protecao via mutex
  imovelRemover = (imovel_t *) arraylist_pop(args->listaDisponveis);
  pthread_mutex_unlock(&mutex_remove);
  printf("Corretor %i removeu o imovel da lista de disponiveis: %i \n",args->nThread, imovelRemover->codigo);
    //imoveis_disponiveis_->remove((imovel *)arg); // corretor remove um imovel da lista
    return 0;
}

void *t_function_corretor_adiciona_imovel(void *arg){
  argumentos *args;
  args = (argumentos *) arg;
  int index = (int) random_at_most(args->listaNovos->size-1);
   if(args->listaEntregues->size > 0){
  struct imovel_t* imovelAdicionar;
  imovelAdicionar = (imovel_t *) arraylist_get(args->listaNovos, index);
    pthread_mutex_lock(&mutex_adiciona);
    arraylist_add(args->listaDisponveis, imovelAdicionar); //adiciona ao pool de disponiveis
    arraylist_remove(args->listaNovos, index); //remove do pool de novos
    pthread_mutex_unlock(&mutex_adiciona);
    printf("Corretor %i adicionou o imovel: %i \n",args->nThread, imovelAdicionar->codigo);
    //imoveis_disponiveis_->insert_sorted((imovel *)arg); // corretor insere um imovel na lista
   }
   return 0;
}

void *t_function_corretor_move_imovel(void *arg){
  argumentos *args;
  args = (argumentos *) arg;
  
  if(args->listaEntregues->size > 0){
    struct imovel_t* imovelEntregue;
    pthread_mutex_lock(&mutex_move);
    imovelEntregue = (imovel_t *) arraylist_pop(args->listaEntregues);
    arraylist_add(args->listaDisponveis, imovelEntregue);
    pthread_mutex_unlock(&mutex_move);
    printf("Corretor %i disponibilizou o imovel: %i \n",args->nThread, imovelEntregue->codigo);
    //imoveis_disponiveis_->insert_sorted(imoveis_entregues_->pop_back()); // corretor remove imovel da lista de entregues e insere na lista de disp.
  }
    return 0;
}

void *t_function_cliente_pega_imovel(void *arg){
  while(1){
    argumentos *args;
    args = (argumentos *) arg;

    int index = (int) random_at_most(args->listaDisponveis->size-1); //-1 pois eh o tamanho e n a posicao do array
    struct imovel_t* imovelAlugado;
    imovelAlugado = (imovel_t *) arraylist_get(args->listaDisponveis, index);
    pthread_mutex_lock(&mutex_aluga);
    arraylist_remove(args->listaDisponveis, index); //remove do pool de novos
    //imovel* imovelAlugado = imoveis_disponiveis_->pop_back();
    pthread_mutex_unlock(&mutex_aluga);
    printf("Cliente %i alugou o imovel: %i \n",args->nThread, imovelAlugado->codigo);
    sleep(5);
    pthread_mutex_lock(&mutex_entrega);
    arraylist_add(args->listaEntregues, imovelAlugado);
    pthread_mutex_unlock(&mutex_entrega);
    //imoveis_entregues_->push_front(imovelAlugado);
    printf("Cliente %i entregou o imovel: %i \n",args->nThread, imovelAlugado->codigo);
    sleep(5);
  }   
}


void *t_function_corretor_decideAcao(void *arg){
  long decisao;
  while(1){//loop infinito
    decisao = random_at_most(2); // na teoria esse cara vai ser um dos 3, toda vez

    switch (decisao)
    {
    case 0:
      t_function_corretor_adiciona_imovel(arg);
      break;
    case 1:
      t_function_corretor_remove_imovel(arg);
      break;
    case 2:
      t_function_corretor_move_imovel(arg);
      break;    
    default:
      break;
    }
    sleep(3);
  }
}


int main(int argc, char *argv[]) {


 // ArrayListImovel* imoveis_disponiveis_ = new ArrayListImovel();
  arraylist* imoveis_disponiveis_;
  imoveis_disponiveis_ = arraylist_create();

  //ArrayListImovel* imoveis_entregues_ = new ArrayListImovel();
  arraylist* imoveis_entregues_;
  imoveis_entregues_ = arraylist_create();

  //ArrayListImovel* imoveis_novos_ = new ArrayListImovel();
  arraylist* imoveis_novos_;
  imoveis_novos_ = arraylist_create();
  int i;
  for(i = 0; i < 100; i++){ //inicializa a lista de disponiveis com 20 imoveis
    struct imovel_t *disp = malloc(sizeof(imovel_t));
    disp->codigo = i + 1; //nao pode ser random aqui porque deve ser UNIQUE 
    char *bairro;
    bairro = (char *) 'a'; //apenas um placeholder pro bairro
    disp->bairro = bairro;
    disp->preco = (double) random_at_most(1000); //gera valor aleatorio pro imovel
    arraylist_add(imoveis_disponiveis_, disp);
    //imoveis_disponiveis_->push_back(disp); //joga imovel no array de imoveis disp
  }
    
    for(i = 0; i < 100; i++){ //inicializa a lista de imoveis novos a serem inseridos na lista de disponveis pelos corretores 
    
    struct imovel_t* novo = malloc(sizeof(imovel_t));
    novo->codigo = i + 101; //nao pode ser random aqui porque deve ser UNIQUE, 21 pois eh o primeiro codigo apos os ja disponveis
    char *bairro;
    bairro = (char *) 'a'; //apenas um placeholder pro bairro
    novo->bairro = bairro;
    novo->preco = (double) random_at_most(1000); //gera valor aleatorio pro imovel entre 0 e 1000
    arraylist_add(imoveis_novos_, novo); //joga imovel no array de imoveis novos
    }
  sleep(3);
  pthread_t tInq[NUM_THREADS_INQ]; //array das threads cliente
  pthread_t tCor[NUM_THREADS_CORRETOR]; //array das threads corretor

  //void *vptr;  pthread_t tInq[NUM_THREADS_INQ]; //array das threads cliente


  

   long *nomet1= (long *) malloc(sizeof(long));       
    *nomet1 = 1;

  

  argumentos* arrayArgs[10];

  for(i = 0; i < 10; i++){
    struct argumentos* args = malloc(sizeof(argumentos));
    args->listaDisponveis = imoveis_disponiveis_;
    args->listaEntregues = imoveis_entregues_;
    args->listaNovos = imoveis_novos_;
    args->nThread = i +1;
    arrayArgs[i] = args;
  }
  
while(1){
  for(i = 0; i < 5; i++){
    printf("Corretor %i se junta a imobiliaria \n", i + 1);
    pthread_create(&tCor[i], NULL, t_function_corretor_decideAcao, (void *) arrayArgs[i]);
  }
  for(i = 5; i < 10; i++){
    printf("Cliente %i se torna cliente da imobiliaria \n", i);
    pthread_create(&tInq[i], NULL, t_function_cliente_pega_imovel, (void *) arrayArgs[i]); // 1 eh numero da thread
  }
  sleep(9999);
}
  return 0;
}