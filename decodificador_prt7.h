/**
 * @file decodificador_prt7.h
 * @brief Sistema de Decodificación de Protocolo Industrial PRT-7
 * @author ITIID
 * @date Noviembre 2025
 * * Este archivo contiene las definiciones de las clases y estructuras
 * necesarias para decodificar el protocolo PRT-7 que envía un Arduino.
 */

#ifndef DECODIFICADOR_PRT7_H
#define DECODIFICADOR_PRT7_H

// Declaraciones adelantadas para evitar dependencias circulares
class ListaDeCarga;
class RotorDeMapeo;

class TramaBase {
public:
    virtual void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) = 0;
    
    virtual ~TramaBase() {}
};

class TramaLoad : public TramaBase {
private:
    char caracter;
    
public:
    TramaLoad(char c);
    
    void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) override;
    
    ~TramaLoad() override;
};

class TramaMap : public TramaBase {
private:
    int rotacion;
    
public:
    TramaMap(int rot);
    
    void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) override;
    
    ~TramaMap() override;
};

// ===== ESTRUCTURAS DE DATOS MANUALES =====

struct NodoRotor {
    char dato;
    NodoRotor* siguiente;
    NodoRotor* previo;
    
    NodoRotor(char c);
};

class RotorDeMapeo {
private:
    NodoRotor* cabeza;
    int tamanio;
    
public:
    RotorDeMapeo();
    
    ~RotorDeMapeo();
    
    void rotar(int n);
    
    char getMapeo(char in);
    
    void mostrarEstado();
};

struct NodoCarga {
    char dato;
    NodoCarga* siguiente;
    NodoCarga* previo;
    
    NodoCarga(char c);
};

class ListaDeCarga {
private:
    NodoCarga* cabeza;
    NodoCarga* cola;
    int tamanio;
    
public:
    ListaDeCarga();
    
    ~ListaDeCarga();
    
    void insertarAlFinal(char dato);
    
    void imprimirMensaje();
    
    char* obtenerMensaje();
    
    int obtenerTamanio() const;
};

class DecodificadorPRT7 {
private:
    ListaDeCarga* listaCarga;
    RotorDeMapeo* rotor;
    
    TramaBase* parsearTrama(const char* linea);
    
    void limpiarEspacios(char* str);
    
public:
    DecodificadorPRT7();
    
    ~DecodificadorPRT7();
    
    bool procesarLinea(const char* linea);
    
    void mostrarMensajeFinal();
    
    ListaDeCarga* obtenerListaCarga();
};

#endif // DECODIFICADOR_PRT7_H