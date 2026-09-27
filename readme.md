# Optimización por Colonia de Hormigas (ACO) para el TSP Masivo en C++

Este proyecto implementa una solución de alto rendimiento basada en el algoritmo de **Optimización por Colonia de Hormigas (Ant Colony Optimization - ACO)** para resolver el **Problema del Viajante de Comercio (Traveling Salesperson Problem - TSP)** escalado masivamente hasta **200,000 ciudades**, superando las limitaciones de memoria RAM y tiempo de cómputo mediante técnicas avanzadas en C++ y Qt Creator.

---

## 📂 Estructura del Workspace y Archivos

El repositorio contiene las diferentes versiones evolutivas y la versión final optimizada del sistema:

* **`main.cpp`**: **Versión Definitiva Masiva**. Implementa el grafo disperso $k$-NN ($k = 40$), paralelización masiva con OpenMP, eliminación de raíces cuadradas redundantes en el ordenamiento, sistema de caché binaria en disco (`knn_cache_200k.bin`) y soporte completo para **200,000 ciudades**.
* **`main_2mil_ciudades.cpp`**: Versión intermedia orientada a pruebas de escalabilidad media (2,000 ciudades).
* **`main_20ciudades.cpp`**: Versión inicial de depuración y validación lógica con un set reducido de 20 ciudades.
* **`CMakeLists.txt`**: Archivo de configuración de compilación optimizado con integración obligatoria de OpenMP (`find_package(OpenMP REQUIRED)`).

---

## 🚀 Optimizaciones Clave Implementadas

Para lograr procesar un mapa masivo de 200,000 ciudades en una laptop sin colapsar el sistema, se aplicaron cuatro pilares de ingeniería de software:

1. **Grafos Dispersos ($N \times k$):** 
   Se eliminó la matriz densa tradicional de $N \times N$ (la cual habría demandado ~160 GB de RAM). En su lugar, cada ciudad almacena únicamente sus **$k = 40$ vecinos más cercanos**, reduciendo drásticamente el consumo de memoria.
2. **Paralelización con OpenMP:** 
   La costosa fase de precomputación de distancias utiliza la directiva `#pragma omp parallel for schedule(dynamic, 100)` para distribuir la carga de trabajo de manera equitativa entre todos los núcleos disponibles de la CPU.
3. **Optimización Matemática sin Raíz Cuadrada:** 
   Durante el ordenamiento masivo de distancias, se evaluó la **distancia al cuadrado** ($dx^2 + dy^2$) en lugar de aplicar la función pesada `std::sqrt()` a billones de combinaciones. La raíz cuadrada se aplica únicamente al resultado final de los $k$ vecinos seleccionados.
4. **Sistema de Caché Binaria en Disco:** 
   La primera ejecución realiza la precomputación y almacena la estructura en un archivo binario local (`knn_cache_200k.bin`). Las ejecuciones posteriores leen el archivo directamente a la memoria RAM en milisegundos, evitando recalcular la matriz.

---

## 📊 Métricas de Desempeño (Ejecución Real)

* **Volumen evaluado:** 200,000 ciudades con $k = 40$.
* **Parámetros ACO:** 50 hormigas, 10 iteraciones, $\alpha = 1.0$, $\beta = 3.0$, $\rho = 0.5$.
* **Tiempo de precomputación inicial (con caché generada):** ~7,275 segundos en la primera pasada (con guardado exitoso en `knn_cache_200k.bin`).
* **Mejor costo global encontrado:** $2.46276 \times 10^7$ en la iteración 10.

---

## 🛠️ Compilación y Ejecución

1. Abre el proyecto utilizando **Qt Creator**.
2. Asegúrate de que el archivo `CMakeLists.txt` reconozca el soporte para OpenMP en tu entorno de compilación (GCC/Clang en Linux).
3. Compila y ejecuta el archivo principal **`main.cpp`**. 
   * *Nota:* La primera ejecución generará automáticamente el archivo `knn_cache_200k.bin` en el directorio de trabajo. Las siguientes ejecuciones cargarán los datos de forma instantánea.
