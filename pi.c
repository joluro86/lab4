#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Función para medir el tiempo de forma precisa
double GetTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

double CalcPi(int n) {
    const double fH = 1.0 / (double)n; 
    double fSum = 0.0; 
    double fX; 
    int i; 

    // Bucle principal 
    for (i = 0; i < n; i++) { 
        fX = fH * ((double)i + 0.5); 
        fSum += 4.0 / (1.0 + fX * fX); // Implementación de 4 / (1 + x^2) 
    }
    return fH * fSum; 
}

int main() {
    int n = 2000000000; // 2 mil millones de iteraciones según el lab 
    
    printf("Calculando Pi en versión serial con n = %d...\n", n);
    
    double start = GetTime(); 
    double pi = CalcPi(n); 
    double end = GetTime(); 
    
    printf("Resultado de Pi: %.15f\n", pi);
    printf("Tiempo de ejecución (CalcPi): %.6f segundos\n", end - start); 
    
    return 0;
}
