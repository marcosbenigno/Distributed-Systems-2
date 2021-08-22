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

int8_t * dado;
int sum = 0;
int nums;
struct lock lock;

struct Intervalo {
    int myInit;
    int myEnd;
    struct lock * lock;
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

struct Intervalo * createIntervalos(int num_threads, int nums) {
    //funcao que calcula o intervalo do array a ser calculado por cada thread
    struct Intervalo * intervalos = (struct Intervalo * ) malloc(num_threads * sizeof(struct Intervalo));
    int numsPorThread = nums / num_threads;
    int currentInit = 0;
    //esse ternario e pro caso de nem todos os numeros se distribuem uniformemente no numero de threads requeridos
    for (int i = 0; i < (nums % num_threads == 0 ? num_threads : num_threads - 1); i++) {
        struct Intervalo intervalo;
        intervalo.myInit = currentInit;
        currentInit = currentInit + numsPorThread;
        intervalo.myEnd = currentInit;
        intervalos[i] = intervalo;

    }

    //tratando a intervalo nao calculada controlada pelo ternario
    if ((nums % num_threads) != 0) {
        struct Intervalo intervalo;
        intervalo.myInit = currentInit;
        intervalo.myEnd = nums;
        intervalos[num_threads - 1] = intervalo;
    }
    return intervalos;
}

void * thread_func(void * intervalo) {
    //funcao a ser rodada por cada thread
    struct Intervalo * vars = (Intervalo * ) intervalo;

    int a = 0;

    for (int i = vars -> myInit; i < vars -> myEnd; i++) {

        a += dado[i];
    }

    acquire( & lock);
    sum += a;
    release( & lock);
}

int main() {
    printf("Quantos numeros devem ser somados?\n");
    scanf("%d", & nums);

    dado = createArray(nums);
    int num_threads;
    printf("Quantas threads serao utilizadas para a soma?\n");
    scanf("%d", & num_threads);

    pthread_t threads[num_threads];
    struct lock * lock;
    //criacao de array com o intervalo do array a ser somado por cada thread
    struct Intervalo * intervalos = createIntervalos(num_threads, nums);
    clock_t start = clock();
    //criacao de threads
    for (int i = 0; i < num_threads; i++) {
        pthread_create( & (threads[i]), NULL, & thread_func, & intervalos[i]);

    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_t stop = clock();

    printf("\nSoma por threads: %d\n", sum);
    double elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("Tempo de execucao (ms): %f", elapsed);

    //linhas abaixo calculam o valor da soma do array para confererencia de valores
    int localAc = 0;
    int i = 0;
    while (i < nums) {
        localAc += dado[i];
        i++;

    }
    printf("\nSoma: %d", localAc);
    return 0;
}
