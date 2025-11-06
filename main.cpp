#include "decodificador_prt7.h"
#include <iostream>
#include <fstream>

// Para Windows
#ifdef _WIN32
    #include <windows.h>
#else
    // Para Linux/Mac
    #include <fcntl.h>
    #include <unistd.h>
    #include <termios.h>
#endif

using namespace std;

class ComunicacionSerial {
private:
#ifdef _WIN32
    HANDLE handleSerial;
#else
    int descriptorSerial;
#endif
    bool conectado;
    
public:
    ComunicacionSerial(const char* puerto, int baudRate = 9600) {
        conectado = false;
        
#ifdef _WIN32
        handleSerial = CreateFileA(
            puerto,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        
        if (handleSerial == INVALID_HANDLE_VALUE) {
            cout << "ERROR: No se pudo abrir el puerto " << puerto << endl;
            return;
        }
        
        DCB parametros = {0};
        parametros.DCBlength = sizeof(parametros);
        
        if (!GetCommState(handleSerial, &parametros)) {
            cout << "ERROR: No se pudo obtener el estado del puerto" << endl;
            CloseHandle(handleSerial);
            return;
        }
        
        parametros.BaudRate = baudRate;
        parametros.ByteSize = 8;
        parametros.StopBits = ONESTOPBIT;
        parametros.Parity = NOPARITY;
        parametros.fDtrControl = DTR_CONTROL_ENABLE;
        
        if (!SetCommState(handleSerial, &parametros)) {
            cout << "ERROR: No se pudo configurar el puerto" << endl;
            CloseHandle(handleSerial);
            return;
        }
        
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        
        if (!SetCommTimeouts(handleSerial, &timeouts)) {
            cout << "ERROR: No se pudieron configurar los timeouts" << endl;
            CloseHandle(handleSerial);
            return;
        }
        
        conectado = true;
        
#else
        descriptorSerial = open(puerto, O_RDWR | O_NOCTTY | O_NDELAY);
        
        if (descriptorSerial == -1) {
            cout << "ERROR: No se pudo abrir el puerto " << puerto << endl;
            return;
        }
        
        struct termios opciones;
        tcgetattr(descriptorSerial, &opciones);
        
        cfsetispeed(&opciones, B9600);
        cfsetospeed(&opciones, B9600);
        
        opciones.c_cflag &= ~PARENB;
        opciones.c_cflag &= ~CSTOPB;
        opciones.c_cflag &= ~CSIZE;
        opciones.c_cflag |= CS8;
        
        opciones.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        opciones.c_iflag &= ~(IXON | IXOFF | IXANY);
        opciones.c_oflag &= ~OPOST;
        
        tcsetattr(descriptorSerial, TCSANOW, &opciones);
        
        conectado = true;
#endif
        
        if (conectado) {
            cout << "Conexión establecida con " << puerto << endl;
        }
    }
    
    ~ComunicacionSerial() {
        if (conectado) {
#ifdef _WIN32
            CloseHandle(handleSerial);
#else
            close(descriptorSerial);
#endif
        }
    }
    
    bool estaConectado() const {
        return conectado;
    }
    
    int leerLinea(char* buffer, int tamanioMax) {
        if (!conectado) return -1;
        
        int bytesLeidos = 0;
        char c;
        
#ifdef _WIN32
        DWORD bytesReales;
        
        while (bytesLeidos < tamanioMax - 1) {
            if (ReadFile(handleSerial, &c, 1, &bytesReales, NULL)) {
                if (bytesReales > 0) {
                    if (c == '\n') {
                        break;
                    }
                    if (c != '\r') {
                        buffer[bytesLeidos++] = c;
                    }
                }
            } else {
                return -1;
            }
        }
#else
        while (bytesLeidos < tamanioMax - 1) {
            int resultado = read(descriptorSerial, &c, 1);
            if (resultado > 0) {
                if (c == '\n') {
                    break;
                }
                if (c != '\r') {
                    buffer[bytesLeidos++] = c;
                }
            } else if (resultado < 0) {
                return -1;
            }
        }
#endif
        
        buffer[bytesLeidos] = '\0';
        return bytesLeidos;
    }
};

void modoSimulacion(DecodificadorPRT7* decodificador, const char* nombreArchivo) {
    cout << "\n========================================" << endl;
    cout << "  MODO SIMULACIÓN" << endl;
    cout << "========================================" << endl;
    cout << "Leyendo tramas desde archivo: " << nombreArchivo << endl;
    
    ifstream archivo(nombreArchivo);
    
    if (!archivo.is_open()) {
        cout << "ERROR: No se pudo abrir el archivo " << nombreArchivo << endl;
        return;
    }
    
    char linea[100];
    
    while (archivo.getline(linea, 100)) {
        if (linea[0] == '\0') continue;
        
        decodificador->procesarLinea(linea);
        
#ifdef _WIN32
        Sleep(500);
#else
        usleep(500000);
#endif
    }
    
    archivo.close();
}

void modoSerial(DecodificadorPRT7* decodificador, const char* puerto) {
    cout << "\n========================================" << endl;
    cout << "  MODO PUERTO SERIAL" << endl;
    cout << "========================================" << endl;
    cout << "Intentando conectar con " << puerto << "..." << endl;
    
    ComunicacionSerial serial(puerto, 9600);
    
    if (!serial.estaConectado()) {
        cout << "ERROR: No se pudo establecer la conexión serial" << endl;
        return;
    }
    
    cout << "Esperando tramas del Arduino..." << endl;
    cout << "(Presione Ctrl+C para terminar)" << endl;
    
    char buffer[100];
    int lineasVacias = 0;
    
    while (true) {
        int bytesLeidos = serial.leerLinea(buffer, 100);
        
        if (bytesLeidos > 0) {
            decodificador->procesarLinea(buffer);
            lineasVacias = 0;
        } else if (bytesLeidos == 0) {
            lineasVacias++;
            if (lineasVacias > 5) {
                cout << "\nNo se reciben más datos. Finalizando..." << endl;
                break;
            }
        }
        
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
    }
}

int main(int argc, char* argv[]) {
    cout << "========================================" << endl;
    cout << "  DECODIFICADOR PRT-7" << endl;
    cout << "  Sistema de Decodificación Industrial" << endl;
    cout << "========================================\n" << endl;
    
    DecodificadorPRT7* decodificador = new DecodificadorPRT7();
    
    cout << "Seleccione el modo de operación:" << endl;
    cout << "1. Modo Simulación (leer desde archivo)" << endl;
    cout << "2. Modo Puerto Serial (Arduino real)" << endl;
    cout << "Opción: ";
    
    int opcion;
    cin >> opcion;
    cin.ignore();
    
    if (opcion == 1) {
        cout << "\nIngrese el nombre del archivo de prueba (ej. tramas.txt): ";
        char nombreArchivo[100];
        cin.getline(nombreArchivo, 100);
        
        modoSimulacion(decodificador, nombreArchivo);
        
    } else if (opcion == 2) {
        cout << "\nIngrese el puerto serial ";
#ifdef _WIN32
        cout << "(ej. COM3): ";
#else
        cout << "(ej. /dev/ttyUSB0): ";
#endif
        char puerto[100];
        cin.getline(puerto, 100);
        
        modoSerial(decodificador, puerto);
        
    } else {
        cout << "Opción inválida" << endl;
    }
    
    decodificador->mostrarMensajeFinal();
    
    cout << "\nLiberando memoria... ";
    delete decodificador;
    cout << "Sistema apagado." << endl;
    
    return 0;
}