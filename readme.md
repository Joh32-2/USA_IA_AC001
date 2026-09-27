# Optimización por Colonia de Hormigas (ACO) para el Problema del Viajante (TSP)

Repositorio académico desarrollado en C++ utilizando **Qt Creator**, enfocado en la implementación, evolución y estrategia de escalamiento del algoritmo metaheurístico **Ant Colony Optimization (ACO)** aplicado al Problema del Viajante (*Traveling Salesperson Problem* - TSP).

---

## 📂 Estructura del Proyecto y Versiones

El proyecto cuenta con múltiples puntos de entrada independientes para demostrar la evolución arquitectónica y el rendimiento del algoritmo a diferentes escalas:

* **`main.cpp`** *(Versión Definitiva)*: Implementación a gran escala para **20,000 ciudades**. Utiliza listas de vecinos cercanos ($k$-NN con $k=40$), 150 agentes (hormigas), 20 iteraciones, regla de respaldo (*fallback*) y matrices paralelas para evitar la saturación de la memoria RAM.
* **`main_2mil_ciudades.cpp`** *(Versión Intermedia)*: Configuración optimizada para **2,000 ciudades** utilizando estructuras dispersas $k$-NN ($k=30$).
* **`main_20ciudades.cpp`** *(Versión Base)*: Primera aproximación educativa y de validación lógica del algoritmo ACO para un grafo pequeño de **20 ciudades**.

---

## ⚙️ Características Técnicas y Optimizaciones

Para lograr escalar eficientemente desde 20 hasta 20,000 ciudades sin desbordar los recursos computacionales, se implementaron las siguientes estrategias de ingeniería:

1. **Estructuras Dispersas ($k$-NN):** Se reemplazó la matriz de adyacencia densa tradicional de complejidad espacial $\mathcal{O}(N^2)$ por listas de vecinos cercanos precomputadas de tamaño $N \times k$, reduciendo drásticamente el consumo de memoria y la complejidad de búsqueda por paso a $\mathcal{O}(k)$.
2. **Mecanismo de Respaldo (*Fallback*):** Incluye una rutina de emergencia en caso de que una hormiga agote sus $k$ vecinos locales disponibles, realizando una búsqueda global eficiente sobre los nodos no visitados restantes para evitar bloqueos.
3. **Control de Diversidad y Exploración:** Ajuste dinámico de la cantidad de agentes ($M$), tasa de evaporación ($\rho$), factores heurísticos ($\alpha, \beta$) y el número de iteraciones para equilibrar la convergencia y la exploración espacial.

---

## 📊 Resumen de Desempeño Empírico

| Escenario | Ciudades ($N$) | Vecindario ($k$) | Hormigas ($M$) | Iteraciones | Costo Óptimo Encontrado | Tiempo de Ejecución |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Base** | 20 | N/A (Denso) | 30 | 10 | *Validación lógica* | $< 1$ segundo |
| **Intermedio** | 2,000 | 30 | 150 | 10 - 30 | Variable (Optimizado) | $\approx 15.9$ s - $60.8$ s |
| **Gran Escala** | **20,000** | **40** | **150** | **20** | **1,287,210** | **~1,254.02 segundos (~20.9 min)** |

---

## 🛠️ Instrucciones de Compilación y Ejecución

1. Abre el proyecto en **Qt Creator** cargando el archivo `CMakeLists.txt`.
2. Para compilar y ejecutar una versión específica (por ejemplo, la de 2,000 ciudades), puedes renombrar temporalmente el archivo deseado a `main.cpp` o ajustar la fuente activa en la configuración de compilación de CMake.
3. Ejecuta el programa desde la terminal integrada de Qt Creator para visualizar en tiempo real el progreso de las iteraciones y las métricas finales de rendimiento.

---
*Desarrollado como parte de proyectos de investigación e inteligencia artificial.*
