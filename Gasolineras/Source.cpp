#include <iostream>
#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <fstream>

#include <json/json.h>
#include <json/reader.h>
#include <json/writer.h>
#include <json/value.h>

using namespace std;

const int N = 100;

typedef struct{
    string provincia;
    string localidad;
    double precio;
    double latitud;
    double longitud;
}tGasolineras;

typedef tGasolineras* tArrayPunt;

typedef struct {
    tArrayPunt registros;
    int contador;
    int DimAct;
    string tipoGasolina;
}tListaGasolineras;


void cargarDatosGasolineras(tListaGasolineras& listaGasolineras, bool &ok);
static Json::Value ParseJson(string text);
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
void mostrarGasolineras(tListaGasolineras lista);
void redimensionar(tListaGasolineras& listaGasolineras);
void liberar(tListaGasolineras& listaGasolineras);
void merge_sort(tListaGasolineras& gasolineras, int low, int high);
void merge(tListaGasolineras& gasolineras, int low, int high, int mid);
bool operator<(tGasolineras op1, tGasolineras op2);
void menu(tListaGasolineras& listaGasolineras);
int mostrarMenu();
void accederLocalizacion(tListaGasolineras listaGasolineras, int posicion);

int main() {
    tListaGasolineras listaGasolineras;
    menu(listaGasolineras);
    liberar(listaGasolineras);
    return 0;
}

void menu(tListaGasolineras &listaGasolineras) {
    int op;
    bool ok = false;
    do {
        op = mostrarMenu();
        switch (op) {
            case 1: {
                int opGasolina;
                cout << "--Elegir tipo de gasolina--" << endl;
                cout << "1. Gasolina 95" << endl;
                cout << "2. Gasolina 98" << endl;
                cout << "3. Biodiesel" << endl;
                cout << "4. Bioetanol" << endl;
                cout << "Opcion:";
                cin >> opGasolina;
                switch (opGasolina)
                {
                    case 1: {
                        listaGasolineras.tipoGasolina = "precio_gasolina_95";
                    }break;
                    case 2: {
                        listaGasolineras.tipoGasolina = "precio_gasolina_98";
                    }break;
                    case 3: {
                        listaGasolineras.tipoGasolina = "precio_biodiesel";
                    }break;
                    case 4: {
                        listaGasolineras.tipoGasolina = "precio_bioetanol";
                    }
                }
                cerr << "Cargando datos y ordenandolos....Por favor espere. \n";
                cargarDatosGasolineras(listaGasolineras, ok);
                merge_sort(listaGasolineras, 0, listaGasolineras.contador - 1);
                cerr << "Datos cargados y ordenados con exito" << endl;

                system("pause");
                system("cls");
            }break;
            case 2: {
                if (ok) {
                    mostrarGasolineras(listaGasolineras);
                }
                else {
                    cout << "Nada que mostrar, carga primero los datos" << endl;
                }
                system("pause");
                system("cls");
            }break;

            case 3: {
                if (ok) {
                    cout << "La gasolina del tipo " << listaGasolineras.tipoGasolina << " mas barata, se encuentra..." << endl;
                    cout << "Localidad: " << listaGasolineras.registros[0].localidad << endl;
                    cout << "Provincia: " << listaGasolineras.registros[0].provincia << endl;
                    cout << "Precio gasolina: " << listaGasolineras.registros[0].precio << endl;

                    accederLocalizacion(listaGasolineras, 0);
                }
                else {
                    cout << "Nada que mostrar, carga primero los datos" << endl;
                }
                system("pause");
                system("cls");
            }break;

            case 4: {
                if (ok) {
                    cout << "La gasolina del tipo " << listaGasolineras.tipoGasolina << " mas barata, se encuentra..." << endl;
                    cout << "Localidad: " << listaGasolineras.registros[listaGasolineras.contador-1].localidad << endl;
                    cout << "Provincia: " << listaGasolineras.registros[listaGasolineras.contador-1].provincia << endl;
                    cout << "Precio gasolina: " << listaGasolineras.registros[listaGasolineras.contador-1].precio << endl;

                    accederLocalizacion(listaGasolineras, listaGasolineras.contador - 1);
                }
                else {
                    cout << "Nada que mostrar, carga primero los datos" << endl;
                }

                system("pause");
                system("cls");
            }
        }
    
    } while (op != 0);
}


void accederLocalizacion(tListaGasolineras listaGasolineras, int posicion) {
    int confirmacion;
    cout << "Desea ver la localizacion en GoogleMaps?" << endl;
    cout << "1. Acceder a localizacion." << endl;
    cout << "2. Quizas en otro momento." << endl;
    cout << "Opcion: ";
    cin >> confirmacion;
    while ((confirmacion < 1) || (confirmacion > 2)) {
        cout << "Elija una opcion valida: ";
        cin >> confirmacion;
    }
    if (confirmacion == 1) {
        string url = "https://www.google.es/maps/place/" + to_string(listaGasolineras.registros[posicion].latitud) + ',' + to_string(listaGasolineras.registros[posicion].longitud);
        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
}

int mostrarMenu() {
    int op = 0;
    cout << "1. Cargar listado de gasolineras en base a tipo de gasolina" << endl;
    cout << "2. Ver listado de gasolineras" << endl;
    cout << "3. Ver datos gasolinera con precio mas barato" << endl;
    cout << "4. Ver datos gasolinera con precio mas caro" << endl;
    cout << "0. Salir" << endl;
    cout << "Opcion: ";
    cin >> op;
    while ((op < 0) || (op > 4)) {
        cout << "Opcion no valida, elige de nuevo: ";
        cin >> op;
    }

    return op;
}

void cargarDatosGasolineras(tListaGasolineras& listaGasolineras, bool &ok) {
    
    //Usando CURL
    
    CURL* curl;
    CURLcode res;
    string readBuffer;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.mapabase.es/arcgis/rest/services/Otros/Gasolineras/FeatureServer/0/query?where=1%3D1&outFields=*&outSR=4326&f=json");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    Json::StreamWriterBuilder builder;
    Json::Value json = ParseJson(readBuffer);
    string output = Json::writeString(builder, json);

    tGasolineras gasolinera;
    listaGasolineras.contador = 0;
    listaGasolineras.registros = new tGasolineras[N];
    listaGasolineras.DimAct = N;

    const Json::Value& features = json["features"]; // ArrayOfFeatures

    int i = 0;
    while (i < features.size()) {
        gasolinera.localidad = features[i]["attributes"]["localidad"].asString();
        gasolinera.provincia = features[i]["attributes"]["provincia"].asString();
        gasolinera.precio = features[i]["attributes"][listaGasolineras.tipoGasolina].asDouble();
        gasolinera.latitud = features[i]["attributes"]["latitud"].asDouble();
        gasolinera.longitud = features[i]["attributes"]["longitud"].asDouble();
        if (listaGasolineras.contador == listaGasolineras.DimAct) {
            redimensionar(listaGasolineras);
        }
        if (features[i]["attributes"][listaGasolineras.tipoGasolina].asDouble() != 0) {
            listaGasolineras.registros[listaGasolineras.contador] = gasolinera;
            listaGasolineras.contador++;
        }
        i++;
    }

    ok = true;
}

void mostrarGasolineras(tListaGasolineras lista) {
    for (int i = 0; i < lista.contador; i++) {
        cout << "Localidad: " << lista.registros[i].localidad << endl;
        cout << "Provincia: " << lista.registros[i].provincia << endl;
        cout << "Precio gasolina " + lista.tipoGasolina + " : " << lista.registros[i].precio << endl;
        cout << "Latitud: " << lista.registros[i].latitud << endl;
        cout << "Longitud: " << lista.registros[i].longitud << endl;
        cout << endl << endl;
    }
}


void redimensionar(tListaGasolineras &listaGasolineras) {
    tListaGasolineras listaAuxiliar;
    listaGasolineras.DimAct += N;
    listaAuxiliar.registros = new tGasolineras[listaGasolineras.DimAct];
    for (int i = 0; i < listaGasolineras.DimAct - N; i++) {
        listaAuxiliar.registros[i] = listaGasolineras.registros[i];
    }
    listaGasolineras.registros = NULL;
    listaGasolineras.registros = listaAuxiliar.registros;
    listaAuxiliar.registros = NULL;
    delete[] listaAuxiliar.registros;
}

void liberar(tListaGasolineras& listaGasolineras) {
    delete[] listaGasolineras.registros;
    listaGasolineras.contador = 0;
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static Json::Value ParseJson(string text) {
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();

    Json::Value root;
    string errors;

    bool parsingSuccessful = reader->parse(text.c_str(), text.c_str() + text.size(), &root, &errors);
    delete reader;

    if (!parsingSuccessful)
    {
        cout << text << endl;
        cout << errors << endl;
        return 0;
    }
    return root;
}

void merge_sort(tListaGasolineras& gasolineras, int low, int high) {
    int mid;
    if (low < high) {
        //divide the array at mid and sort independently using merge sort
        mid = (low + high) / 2;
        merge_sort(gasolineras, low, mid);
        merge_sort(gasolineras, mid + 1, high);
        //merge or conquer sorted arrays
        merge(gasolineras, low, high, mid);
    }
}

void merge(tListaGasolineras& gasolineras, int low, int high, int mid) {
    int i, j, k;
    tGasolineras c[1000];
    i = low;
    k = low;
    j = mid + 1;
    while (i <= mid && j <= high) {
        if (gasolineras.registros[i] < gasolineras.registros[j]) {
            c[k] = gasolineras.registros[i];
            k++;
            i++;
        }
        else {
            c[k] = gasolineras.registros[j];
            k++;
            j++;
        }
    }
    while (i <= mid) {
        c[k] = gasolineras.registros[i];
        k++;
        i++;
    }
    while (j <= high) {
        c[k] = gasolineras.registros[j];
        k++;
        j++;
    }
    for (i = low; i < k; i++) {
        gasolineras.registros[i] = c[i];
    }
}

//Sobreescribimos el operador < para que la comparacion sea en base a los precios
bool operator<(tGasolineras op1, tGasolineras op2) {
    return op1.precio < op2.precio;
}