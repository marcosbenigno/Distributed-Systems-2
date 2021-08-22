#include <stdio.h>

#include <stdlib.h>

#include <pthread.h>

#include <time.h>

#include <math.h>

#include <atomic>


//COMPILACAO COM g++ -pthread ex1.c -o ex1

struct lock {
    std::atomic_flag held = ATOMIC_FLAG_INIT;
};

int8_t * array;
int soma = 0;
int nums;
struct lock lock;

struct Intervalo {
    int inicio;
    int fim;
};

void acquire(struct lock * lock) {
    while (lock -> held.test_and_set());
}

void release(struct lock * lock) {
    lock -> held.clear();
}

int8_t numeroAleatorio() {

    int8_t numeroFinal;
    numeroFinal = rand() % 201 + (-100);

    return numeroFinal;
}

int8_t * createArray(int nums) {
    int8_t * array = (int8_t * ) malloc(nums * sizeof(int8_t));
    srand(time(NULL));
    for (int i = 0; i < nums; i++) {

        array[i] = numeroAleatorio();

    }

    return array;
}

struct Intervalo * createIntervalos(int numeroDeThreads, int nums) {
    //funcao que calcula o intervalo do array a ser calculado por cada thread
    struct Intervalo * intervalos = (struct Intervalo * ) malloc(numeroDeThreads * sizeof(struct Intervalo));
    int numsPorThread = nums / numeroDeThreads;
    int inicioAtual = 0;
    //esse ternario e pro caso de nem todos os numeros se distribuem uniformemente no numero de threads requeridos
    for (int i = 0; i < (nums % numeroDeThreads == 0 ? numeroDeThreads : numeroDeThreads - 1); i++) {
        struct Intervalo intervalo;
        intervalo.inicio = inicioAtual;
        inicioAtual = inicioAtual + numsPorThread;
        intervalo.fim = inicioAtual;
        intervalos[i] = intervalo;

    }

    //tratando a intervalo nao calculada controlada pelo ternario
    if ((nums % numeroDeThreads) != 0) {
        struct Intervalo intervalo;
        intervalo.inicio = inicioAtual;
        intervalo.fim = nums;
        intervalos[numeroDeThreads - 1] = intervalo;
    }
    return intervalos;
}

void * thread_func(void * intervalo) {
    //funcao a ser rodada por cada thread
    struct Intervalo * vars = (Intervalo * ) intervalo;

    int a = 0;

    for (int i = vars -> inicio; i < vars -> fim; i++) {

        a += array[i];
    }

    acquire( & lock);
    soma += a;
    release( & lock);
}

int main() {
    printf("Quantos numeros devem ser somados?\n");
    scanf("%d", & nums);

    array = createArray(nums);
    int numeroDeThreads;
    printf("Quantas threads serao utilizadas para a soma?\n");
    scanf("%d", & numeroDeThreads);

    pthread_t threads[numeroDeThreads];
    struct lock * lock;
    //criacao de array com o intervalo do array a ser somado por cada thread
    struct Intervalo * intervalos = createIntervalos(numeroDeThreads, nums);
    clock_t start = clock();
    //criacao de threads
    for (int i = 0; i < numeroDeThreads; i++) {
        pthread_create( & (threads[i]), NULL, & thread_func, & intervalos[i]);

    }

    for (int i = 0; i < numeroDeThreads; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_t stop = clock();

    printf("\nSoma por threads: %d\n", soma);
    double tempo = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("Tempo de execucao (ms): %f", tempo);

    //linhas abaixo calculam o valor da soma do array para confererencia de valores
    int localAc = 0;
    int i = 0;
    while (i < nums) {
        localAc += array[i];
        i++;

    }
    printf("\nSoma: %d", localAc);
    return 0;
}
