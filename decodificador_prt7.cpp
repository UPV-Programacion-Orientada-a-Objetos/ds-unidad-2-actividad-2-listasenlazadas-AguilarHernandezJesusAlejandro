#include "decodificador_prt7.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace std;

// ===== IMPLEMENTACIÓN DE TRAMAS =====

TramaLoad::TramaLoad(char c) : caracter(c) {
    
}

void TramaLoad::procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) {
    char caracterDecodificado = rotor->getMapeo(caracter);
    
    carga->insertarAlFinal(caracterDecodificado);
    
    cout << "  -> Fragmento '" << caracter << "' decodificado como '" 
         << caracterDecodificado << "'." << endl;
}

TramaLoad::~TramaLoad() {
    
}

TramaMap::TramaMap(int rot) : rotacion(rot) {
    
}

void TramaMap::procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) {
    rotor->rotar(rotacion);
    
    cout << "  -> ROTANDO ROTOR " << (rotacion >= 0 ? "+" : "") << rotacion << "." << endl;
}

TramaMap::~TramaMap() {
    
}

// ===== IMPLEMENTACIÓN DEL ROTOR DE MAPEO =====

NodoRotor::NodoRotor(char c) : dato(c), siguiente(nullptr), previo(nullptr) {
    
}

RotorDeMapeo::RotorDeMapeo() : cabeza(nullptr), tamanio(0) {
    NodoRotor* primerNodo = nullptr;
    NodoRotor* ultimoNodo = nullptr;
    
    for (char c = 'A'; c <= 'Z'; c++) {
        NodoRotor* nuevoNodo = new NodoRotor(c);
        tamanio++;
        
        if (primerNodo == nullptr) {
            primerNodo = nuevoNodo;
            cabeza = primerNodo;
        } else {
            ultimoNodo->siguiente = nuevoNodo;
            nuevoNodo->previo = ultimoNodo;
        }
        
        ultimoNodo = nuevoNodo;
    }
    
    NodoRotor* nodoEspacio = new NodoRotor(' ');
    ultimoNodo->siguiente = nodoEspacio;
    nodoEspacio->previo = ultimoNodo;
    ultimoNodo = nodoEspacio;
    tamanio++;
    
    if (primerNodo != nullptr && ultimoNodo != nullptr) {
        ultimoNodo->siguiente = primerNodo;
        primerNodo->previo = ultimoNodo;
    }
}

RotorDeMapeo::~RotorDeMapeo() {
    if (cabeza == nullptr) return;
    
    NodoRotor* actual = cabeza;
    NodoRotor* primero = cabeza;
    
    cabeza->previo->siguiente = nullptr;
    
    while (actual != nullptr) {
        NodoRotor* siguiente = actual->siguiente;
        delete actual;
        actual = siguiente;
    }
    
    cabeza = nullptr;
}

void RotorDeMapeo::rotar(int n) {
    if (cabeza == nullptr || n == 0) return;
    
    int rotacionNormalizada = n % tamanio;
    
    if (rotacionNormalizada > 0) {
        for (int i = 0; i < rotacionNormalizada; i++) {
            cabeza = cabeza->siguiente;
        }
    } else if (rotacionNormalizada < 0) {
        for (int i = 0; i > rotacionNormalizada; i--) {
            cabeza = cabeza->previo;
        }
    }
}

char RotorDeMapeo::getMapeo(char in) {
    if (cabeza == nullptr) return in;
    
    if (in == ' ') return ' ';
    
    char inMayuscula = in;
    if (in >= 'a' && in <= 'z') {
        inMayuscula = in - 32;
    }
    
    NodoRotor* nodoEncontrado = cabeza;
    int posicionRelativa = 0;
    bool encontrado = false;
    
    for (int i = 0; i < tamanio; i++) {
        if (nodoEncontrado->dato == inMayuscula) {
            encontrado = true;
            posicionRelativa = i;
            break;
        }
        nodoEncontrado = nodoEncontrado->siguiente;
    }
    
    if (!encontrado) return in;
    
    NodoRotor* nodoMapeado = cabeza;
    for (int i = 0; i < posicionRelativa; i++) {
        nodoMapeado = nodoMapeado->siguiente;
    }
    
    return nodoMapeado->dato;
}

void RotorDeMapeo::mostrarEstado() {
    if (cabeza == nullptr) {
        cout << "Rotor vacío" << endl;
        return;
    }
    
    cout << "Estado del rotor (cabeza en '" << cabeza->dato << "'): ";
    NodoRotor* actual = cabeza;
    for (int i = 0; i < tamanio && i < 10; i++) {
        cout << actual->dato << " ";
        actual = actual->siguiente;
    }
    cout << "..." << endl;
}

// ===== IMPLEMENTACIÓN DE LA LISTA DE CARGA =====

NodoCarga::NodoCarga(char c) : dato(c), siguiente(nullptr), previo(nullptr) {
    
}

ListaDeCarga::ListaDeCarga() : cabeza(nullptr), cola(nullptr), tamanio(0) {
    
}

ListaDeCarga::~ListaDeCarga() {
    NodoCarga* actual = cabeza;
    
    while (actual != nullptr) {
        NodoCarga* siguiente = actual->siguiente;
        delete actual;
        actual = siguiente;
    }
    
    cabeza = nullptr;
    cola = nullptr;
}

void ListaDeCarga::insertarAlFinal(char dato) {
    NodoCarga* nuevoNodo = new NodoCarga(dato);
    
    if (cabeza == nullptr) {
        cabeza = nuevoNodo;
        cola = nuevoNodo;
    } else {
        cola->siguiente = nuevoNodo;
        nuevoNodo->previo = cola;
        cola = nuevoNodo;
    }
    
    tamanio++;
}

void ListaDeCarga::imprimirMensaje() {
    cout << "  Mensaje actual: [";
    
    NodoCarga* actual = cabeza;
    while (actual != nullptr) {
        if (actual->dato == ' ') {
            cout << "·";
        } else {
            cout << actual->dato;
        }
        actual = actual->siguiente;
    }
    
    cout << "]" << endl;
}

char* ListaDeCarga::obtenerMensaje() {
    char* mensaje = new char[tamanio + 1];
    
    NodoCarga* actual = cabeza;
    int indice = 0;
    
    while (actual != nullptr) {
        mensaje[indice++] = actual->dato;
        actual = actual->siguiente;
    }
    
    mensaje[indice] = '\0';
    return mensaje;
}

int ListaDeCarga::obtenerTamanio() const {
    return tamanio;
}

// ===== IMPLEMENTACIÓN DEL DECODIFICADOR PRINCIPAL =====

DecodificadorPRT7::DecodificadorPRT7() {
    listaCarga = new ListaDeCarga();
    rotor = new RotorDeMapeo();
}

DecodificadorPRT7::~DecodificadorPRT7() {
    delete listaCarga;
    delete rotor;
}

void DecodificadorPRT7::limpiarEspacios(char* str) {
    if (str == nullptr) return;
    
    int i = 0, j = 0;
    
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n') {
        i++;
    }
    
    while (str[i] != '\0') {
        if (str[i] != '\r' && str[i] != '\n') {
            str[j++] = str[i];
        }
        i++;
    }
    
    j--;
    while (j >= 0 && (str[j] == ' ' || str[j] == '\t')) {
        j--;
    }
    
    str[j + 1] = '\0';
}

TramaBase* DecodificadorPRT7::parsearTrama(const char* linea) {
    if (linea == nullptr || linea[0] == '\0') {
        return nullptr;
    }
    
    char lineaCopia[100];
    int i = 0;
    while (linea[i] != '\0' && i < 99) {
        lineaCopia[i] = linea[i];
        i++;
    }
    lineaCopia[i] = '\0';
    
    limpiarEspacios(lineaCopia);
    
    if (lineaCopia[0] == '\0' || lineaCopia[1] != ',') {
        cout << "  ERROR: Formato inválido" << endl;
        return nullptr;
    }
    
    char tipo = lineaCopia[0];
    
    if (tipo == 'L' || tipo == 'l') {
        char caracter = lineaCopia[2];
        
        if (lineaCopia[2] == 'S' || lineaCopia[2] == 's') {
            caracter = ' ';
        }
        
        return new TramaLoad(caracter);
        
    } else if (tipo == 'M' || tipo == 'm') {
        int rotacion = 0;
        
        int indice = 2;
        bool negativo = false;
        
        if (lineaCopia[indice] == '-') {
            negativo = true;
            indice++;
        } else if (lineaCopia[indice] == '+') {
            indice++;
        }
        
        while (lineaCopia[indice] >= '0' && lineaCopia[indice] <= '9') {
            rotacion = rotacion * 10 + (lineaCopia[indice] - '0');
            indice++;
        }
        
        if (negativo) {
            rotacion = -rotacion;
        }
        
        return new TramaMap(rotacion);
        
    } else {
        cout << "  ERROR: Tipo de trama desconocido '" << tipo << "'" << endl;
        return nullptr;
    }
}

bool DecodificadorPRT7::procesarLinea(const char* linea) {
    if (linea == nullptr) return false;
    
    cout << "\nTrama recibida: [" << linea << "]";
    
    TramaBase* trama = parsearTrama(linea);
    
    if (trama == nullptr) {
        return false;
    }
    
    cout << " -> Procesando..." << endl;
    
    trama->procesar(listaCarga, rotor);
    
    listaCarga->imprimirMensaje();
    
    delete trama;
    
    return true;
}

void DecodificadorPRT7::mostrarMensajeFinal() {
    cout << "\n========================================" << endl;
    cout << "Flujo de datos terminado." << endl;
    cout << "MENSAJE OCULTO ENSAMBLADO:" << endl;
    
    char* mensaje = listaCarga->obtenerMensaje();
    cout << mensaje << endl;
    
    delete[] mensaje;
    
    cout << "========================================" << endl;
}

ListaDeCarga* DecodificadorPRT7::obtenerListaCarga() {
    return listaCarga;
}