#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>

struct estado {
  int fila;
  int columna;
  int orientacion;
};

class ComportamientoJugador : public Comportamiento {
  public:
	ComportamientoJugador(unsigned int size) : Comportamiento(size) {
		// Inicializar Variables de Estado
		fil = col = size;
		brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
		contador = 1;
		destino.fila = -1;
		destino.columna = -1;
		destino.orientacion = -1;
		ultimaAccion = actIDLE;
		situado = false;
		hayplan = false;
		planEsCu = false;
		
		vector<unsigned char> aux(size*2, '?');
		vector<int> aux2(size*2, 0);

		for(unsigned int i = 0; i < size*2; i++){
			mapaPreliminar.push_back(aux);
			mapaReactivo.push_back(aux2);
		}
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
		fil = col = mapaR.size();
		brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
		contador = 1;
		destino.fila = -1;
		destino.columna = -1;
		destino.orientacion = -1;
		ultimaAccion = actIDLE;
		situado = false;
		hayplan = false;
		
		vector<unsigned char> aux(mapaR.size()*2, '?');
		vector<int> aux2(mapaR.size()*2, 0);

		for(unsigned int i = 0; i < mapaR.size()*2; i++){
			mapaPreliminar.push_back(aux);
			mapaReactivo.push_back(aux2);
		}
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

  private:
    // Declarar Variables de Estado
    int fil, col, brujula;
	int contador;
	vector< vector< unsigned char> > mapaPreliminar;
	vector< vector< int > > mapaReactivo;
	
    estado actual, destino;
    list<Action> plan;
	bool situado, hayplan, planEsCu;
	Action ultimaAccion;

    // Métodos privados de la clase
    bool pathFinding(int level, const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
	bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);
	bool pathFinding_CosteUniforme(const estado &origen, const estado &destino, list<Action> &plan);

    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);
	unsigned int costeCasilla(const estado &a);
	void actualizarMapa(Sensores sensores);
	
	// Parte reactiva
	int elegirCasilla();
	void actualizarMapaReactivo(Sensores sensores);
	void actualizarMapaPreliminar(Sensores sensores);
	void copiarMapa(int difFil, int difCol);
};

#endif
