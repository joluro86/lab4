#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Estructura para pasar argumentos a cada hilo
typedef struct {
    int id;
    int n;
    int num_threads;
    double fH;
} ThreadArgs;

double GetTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

// Rutina que ejecutará cada hilo
void* ThreadCalcPi(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    double fH = args->fH;
    int n = args->n;
    int T = args->num_threads;
    int id = args->id;
    
    // Variable local para acumular la suma sin introducir contención (sin mutex)
    double local_sum = 0.0; 
    
    // Reparto iterativo de las iteraciones (Interleaved/Cyclic or Blocked distribution)
    // Usamos distribución cíclica por simplicidad y balance de carga balanceado
    for (int i = id; i < n; i += T) {
        double fX = fH * ((double)i + 0.5);
        local_sum += 4.0 / (1.0 + fX * fX);
    }
    
    // Almacenamos el resultado en memoria dinámica para retornarlo al main
    double* thread_result = malloc(sizeof(double));
    *thread_result = local_sum;
    
    pthread_exit((void*)thread_result);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <número de hilos>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]); // Recibir T por argumento de línea de comandos
    int n = 2000000000;              // Configuración requerida por la guía
    double fH = 1.0 / (double)n;
    double total_sum = 0.0;

    pthread_t threads[num_threads];
    ThreadArgs thread_args[num_threads];

    printf("Calculando Pi en paralelo con %d hilos y n = %d...\n", num_threads, n);

    double start = GetTime();

    // 1. Creación de hilos
    for (int i = 0; i < num_threads; i++) {
        thread_args[i].id = i;
        thread_args[i].n = n;
        thread_args[i].num_threads = num_threads;
        thread_args[i].fH = fH;
        
        pthread_create(&threads[i], NULL, ThreadCalcPi, (void*)&thread_args[i]);
    }

    // 2. Sincronización y recolección de resultados parciales
    for (int i = 0; i < num_threads; i++) {
        double* partial_sum;
        pthread_join(threads[i], (void**)&partial_sum);
        
        total_sum += *partial_sum;
        free(partial_sum); // Liberar la memoria asignada en el hilo
    }

    // 3. Resultado final multiplicado por fH
    double pi = fH * total_sum;
    
    double end = GetTime();

    printf("Resultado de Pi: %.15f\n", pi);
    printf("Tiempo de ejecución (CalcPi Paralelo): %.6f segundos\n", end - start);

    return 0;
}
