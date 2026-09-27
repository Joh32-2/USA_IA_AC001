#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <iomanip>
#include <chrono> // Librería para medición de tiempo

struct Ciudad {
    double x, y;
};

// Función para calcular la distancia euclidiana entre dos ciudades
double calcularDistancia(const Ciudad& c1, const Ciudad& c2) {
    return std::sqrt(std::pow(c1.x - c2.x, 2) + std::pow(c1.y - c2.y, 2));
}

int main() {
    int numCiudades = 20;
    int numHormigas = 20;
    int maxIteraciones = 100;

    // Parámetros ACO
    double alfa = 1.0;
    double beta = 3.0;
    double rho = 0.5;
    double Q = 100.0;

    // Iniciar cronómetro de alta resolución
    auto tiempoInicio = std::chrono::high_resolution_clock::now();

    // 1. Generar 20 ciudades aleatorias
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 100.0);

    std::vector<Ciudad> ciudades(numCiudades);
    for (int i = 0; i < numCiudades; ++i) {
        ciudades[i] = {dist(rng), dist(rng)};
    }

    // 2. Calcular matriz de distancias y matriz de feromonas iniciales
    std::vector<std::vector<double>> d(numCiudades, std::vector<double>(numCiudades));
    std::vector<std::vector<double>> feromona(numCiudades, std::vector<double>(numCiudades, 1.0));

    for (int i = 0; i < numCiudades; ++i) {
        for (int j = 0; j < numCiudades; ++j) {
            if (i == j) d[i][j] = 0.0;
            else d[i][j] = calcularDistancia(ciudades[i], ciudades[j]);
        }
    }

    std::vector<int> mejorRutaGlobal;
    double mejorCostoGlobal = 1e9;

    // 3. Ciclo principal de iteraciones del ACO
    for (int iter = 0; iter < maxIteraciones; ++iter) {
        std::vector<std::vector<int>> rutasHormigas(numHormigas);
        std::vector<double> costosHormigas(numHormigas, 0.0);

        for (int k = 0; k < numHormigas; ++k) {
            std::vector<bool> visitados(numCiudades, false);
            std::vector<int> ruta;

            std::uniform_int_distribution<int> distribNode(0, numCiudades - 1);
            int actual = distribNode(rng);
            ruta.push_back(actual);
            visitados[actual] = true;

            for (int paso = 1; paso < numCiudades; ++paso) {
                std::vector<double> probabilidades(numCiudades, 0.0);
                double sumaProbabilidad = 0.0;

                for (int siguiente = 0; siguiente < numCiudades; ++siguiente) {
                    if (!visitados[siguiente]) {
                        double heuristica = (d[actual][siguiente] > 0) ? (1.0 / d[actual][siguiente]) : 0.0;
                        probabilidades[siguiente] = std::pow(feromona[actual][siguiente], alfa) * std::pow(heuristica, beta);
                        sumaProbabilidad += probabilidades[siguiente];
                    }
                }

                std::uniform_real_distribution<double> distProb(0.0, sumaProbabilidad);
                double r = distProb(rng);
                double acumulado = 0.0;
                int siguienteSeleccionado = -1;

                for (int siguiente = 0; siguiente < numCiudades; ++siguiente) {
                    if (!visitados[siguiente]) {
                        acumulado += probabilidades[siguiente];
                        if (acumulado >= r) {
                            siguienteSeleccionado = siguiente;
                            break;
                        }
                    }
                }

                if (siguienteSeleccionado == -1) {
                    for (int siguiente = 0; siguiente < numCiudades; ++siguiente) {
                        if (!visitados[siguiente]) {
                            siguienteSeleccionado = siguiente;
                            break;
                        }
                    }
                }

                actual = siguienteSeleccionado;
                ruta.push_back(actual);
                visitados[actual] = true;
            }

            rutasHormigas[k] = ruta;

            double costoRuta = 0.0;
            for (int i = 0; i < numCiudades; ++i) {
                int desde = ruta[i];
                int hasta = ruta[(i + 1) % numCiudades];
                costoRuta += d[desde][hasta];
            }
            costosHormigas[k] = costoRuta;

            if (costoRuta < mejorCostoGlobal) {
                mejorCostoGlobal = costoRuta;
                mejorRutaGlobal = ruta;
            }
        }

        // 4. Evaporación de feromonas
        for (int i = 0; i < numCiudades; ++i) {
            for (int j = 0; j < numCiudades; ++j) {
                feromona[i][j] *= (1.0 - rho);
            }
        }

        // 5. Depósito de nuevas feromonas
        for (int k = 0; k < numHormigas; ++k) {
            double aporte = Q / costosHormigas[k];
            const auto& ruta = rutasHormigas[k];
            for (int i = 0; i < numCiudades; ++i) {
                int desde = ruta[i];
                int hasta = ruta[(i + 1) % numCiudades];
                feromona[desde][hasta] += aporte;
                feromona[hasta][desde] += aporte;
            }
        }
    }

    // Detener cronómetro
    auto tiempoFin = std::chrono::high_resolution_clock::now();

    // Calcular duración en milisegundos y microsegundos
    std::chrono::duration<double, std::milli> duracionMilisegundos = tiempoFin - tiempoInicio;
    std::chrono::duration<double, std::micro> duracionMicrosegundos = tiempoFin - tiempoInicio;

    // Resultados
    std::cout << "========================================\n";
    std::cout << "METRICAS DE DESEMPENO (20 Ciudades)\n";
    std::cout << "========================================\n";
    std::cout << "Mejor costo encontrado: " << mejorCostoGlobal << "\n";
    std::cout << "Tiempo de ejecucion: " << duracionMilisegundos.count() << " ms ("
              << duracionMicrosegundos.count() << " us)\n";
    std::cout << "Ruta: ";
    for (int nodo : mejorRutaGlobal) {
        std::cout << nodo << " ";
    }
    std::cout << "\n========================================\n";

    return 0;
}
