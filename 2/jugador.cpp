#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <set>
#include <stack>
#include <queue>
#include <map>

// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
Action ComportamientoJugador::think(Sensores sensores) {
	Action accion = actIDLE;
	
	// Actualizar el estado según la última acción
	switch (ultimaAccion)
	{
		case actTURN_R: brujula = (brujula + 1) % 4; break;
		case actTURN_L: brujula = (brujula + 3) % 4; break;
		case actFORWARD:
			switch (brujula)
			{
				case 0: fil--; break;
				case 1: col++; break;
				case 2: fil++; break;
				case 3: col--; break;
			}
		break;
	}
	
	// Nivel 1
	if (sensores.nivel != 4)
	{
		// Situarse al recibir posicion
		if (sensores.mensajeF != -1 && !situado)
		{
			fil = sensores.mensajeF;
			col = sensores.mensajeC;

			destino.fila = sensores.destinoF;
			destino.columna = sensores.destinoC;
			situado = true;
		}
	
		if (!plan.empty())
		{
			accion = plan.front();
			plan.pop_front();
		}
		
		if (!hayplan)
		{
			actual.fila = fil;
			actual.columna = col;
			actual.orientacion = brujula;
			hayplan = pathFinding (sensores.nivel, actual, destino, plan);
		}
	} // Nivel 2
	else 
	{
		// Situarse al recibir posicion
		if (sensores.mensajeF != -1 && !situado)
		{
			int difFil = fil - sensores.mensajeF;
			int difCol = col - sensores.mensajeC;
			fil = sensores.mensajeF;
			col = sensores.mensajeC;

			copiarMapa(difFil, difCol);
			destino.fila = sensores.destinoF;
			destino.columna = sensores.destinoC;
			situado = true;
		}
		// Si hay plan seguirlo
		if (hayplan && !plan.empty())
		{
			Action sigAccion = plan.front();
				
			if (sigAccion == actFORWARD)
			{
				if (sensores.terreno[2]=='P' || sensores.terreno[2]=='M' || sensores.terreno[2]=='D')
				{
					hayplan = false;
				}
				else if (sensores.superficie[2]=='a') {}
				else
				{
					accion = sigAccion;
					plan.pop_front();
				}
			}
			else
			{
				accion = sigAccion;
				plan.pop_front();
			}
		}
	
		
		// Actualizar destino
		if (sensores.destinoF != destino.fila || sensores.destinoC != destino.columna)
		{
			destino.fila = sensores.destinoF;
			destino.columna = sensores.destinoC;
			hayplan = false;
		}
		
		// Si esta situado actualizar mapa y plan, si no actuar reactivamente
		if (situado)
		{
			actualizarMapa(sensores);
			
			if (!hayplan)
			{
				actual.fila = fil;
				actual.columna = col;
				actual.orientacion = brujula;
				hayplan = pathFinding(sensores.nivel, actual, destino, plan);
				accion = actIDLE;
			}
		}
		else
		{
			mapaReactivo[fil][col] = contador++;
			actualizarMapaReactivo(sensores);
			actualizarMapaPreliminar(sensores);
			if (sensores.terreno[2]=='P' || sensores.terreno[2]=='M' || sensores.terreno[2]=='D')
			{
				switch (brujula) {
					case 0: mapaReactivo[fil-1][col] = 99999; break;
					case 1: mapaReactivo[fil][col+1] = 99999; break;
					case 2: mapaReactivo[fil+1][col] = 99999; break;
					case 3: mapaReactivo[fil][col-1] = 99999; break;
				}
				accion = actTURN_R;
			}
			else
			{
				if (elegirCasilla() == brujula)
				{
					accion = actFORWARD;
					if (sensores.superficie[2]=='a')
						accion = actIDLE;
				}
				else
					accion = actTURN_R;
			}
		}
	}
	ultimaAccion = accion;
	
	return accion;
}


// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const estado &destino, list<Action> &plan){
	switch (level){
		case 1: cout << "Busqueda en profundidad\n";
				return pathFinding_Profundidad(origen,destino,plan);
						break;
		case 2: cout << "Busqueda en Anchura\n";
				return pathFinding_Anchura(origen,destino,plan);
						break;
		case 3: cout << "Busqueda Costo Uniforme\n";
				return pathFinding_CosteUniforme(origen,destino,plan);
						break;
		case 4: cout << "Busqueda para el reto\n";
				return pathFinding_CosteUniforme(origen,destino,plan);
						break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M' or casilla =='D')
		return true;
	else
	  return false;
}


// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
		st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}

// Devuelve lo que costaria avanzar por una casilla segun su tipo
unsigned int ComportamientoJugador::costeCasilla(const estado &a)
{
		unsigned char tipo = mapaResultado[a.fila][a.columna];
		
		switch (tipo)
		{
			case 'S': return 1;
			case 'T': return 2;
			case '?': return 3;
			case 'B': return 5;
			case 'A': return 10;
			default : return 1;
		}
}

// Actualiza la informacion del mapa segun los sensores
void ComportamientoJugador::actualizarMapa(Sensores sensores)
{
	int aux;
	mapaResultado[fil][col] = sensores.terreno[0];
	switch (brujula) 
	{
		case 0:
			aux = fil;
			aux--;
			mapaResultado[aux][col-1] = sensores.terreno[1];
			mapaResultado[aux][col] = sensores.terreno[2];
			mapaResultado[aux][col+1] = sensores.terreno[3];
			aux--;
			mapaResultado[aux][col-2] = sensores.terreno[4];
			mapaResultado[aux][col-1] = sensores.terreno[5];
			mapaResultado[aux][col] = sensores.terreno[6];
			mapaResultado[aux][col+1] = sensores.terreno[7];
			mapaResultado[aux][col+2] = sensores.terreno[8];
			aux--;
			mapaResultado[aux][col-3] = sensores.terreno[9];
			mapaResultado[aux][col-2] = sensores.terreno[10];
			mapaResultado[aux][col-1] = sensores.terreno[11];
			mapaResultado[aux][col] = sensores.terreno[12];
			mapaResultado[aux][col+1] = sensores.terreno[13];
			mapaResultado[aux][col+2] = sensores.terreno[14];
			mapaResultado[aux][col+3] = sensores.terreno[15];
			break;
		case 1:
			aux = col;
			aux++;
			mapaResultado[fil-1][aux] = sensores.terreno[1];
			mapaResultado[fil][aux] = sensores.terreno[2];
			mapaResultado[fil+1][aux] = sensores.terreno[3];
			aux++;
			mapaResultado[fil-2][aux] = sensores.terreno[4];
			mapaResultado[fil-1][aux] = sensores.terreno[5];
			mapaResultado[fil][aux] = sensores.terreno[6];
			mapaResultado[fil+1][aux] = sensores.terreno[7];
			mapaResultado[fil+2][aux] = sensores.terreno[8];
			aux++;
			mapaResultado[fil-3][aux] = sensores.terreno[9];
			mapaResultado[fil-2][aux] = sensores.terreno[10];
			mapaResultado[fil-1][aux] = sensores.terreno[11];
			mapaResultado[fil][aux] = sensores.terreno[12];
			mapaResultado[fil+1][aux] = sensores.terreno[13];
			mapaResultado[fil+2][aux] = sensores.terreno[14];
			mapaResultado[fil+3][aux] = sensores.terreno[15];
			break;
		case 2:
			aux = fil;
			aux++;
			mapaResultado[aux][col+1] = sensores.terreno[1];
			mapaResultado[aux][col] = sensores.terreno[2];
			mapaResultado[aux][col-1] = sensores.terreno[3];
			aux++;
			mapaResultado[aux][col+2] = sensores.terreno[4];
			mapaResultado[aux][col+1] = sensores.terreno[5];
			mapaResultado[aux][col] = sensores.terreno[6];
			mapaResultado[aux][col-1] = sensores.terreno[7];
			mapaResultado[aux][col-2] = sensores.terreno[8];
			aux++;
			mapaResultado[aux][col+3] = sensores.terreno[9];
			mapaResultado[aux][col+2] = sensores.terreno[10];
			mapaResultado[aux][col+1] = sensores.terreno[11];
			mapaResultado[aux][col] = sensores.terreno[12];
			mapaResultado[aux][col-1] = sensores.terreno[13];
			mapaResultado[aux][col-2] = sensores.terreno[14];
			mapaResultado[aux][col-3] = sensores.terreno[15];
			break;
		case 3:
			aux = col;
			aux--;
			mapaResultado[fil+1][aux] = sensores.terreno[1];
			mapaResultado[fil][aux] = sensores.terreno[2];
			mapaResultado[fil-1][aux] = sensores.terreno[3];
			aux--;
			mapaResultado[fil+2][aux] = sensores.terreno[4];
			mapaResultado[fil+1][aux] = sensores.terreno[5];
			mapaResultado[fil][aux] = sensores.terreno[6];
			mapaResultado[fil-1][aux] = sensores.terreno[7];
			mapaResultado[fil-2][aux] = sensores.terreno[8];
			aux--;
			mapaResultado[fil+3][aux] = sensores.terreno[9];
			mapaResultado[fil+2][aux] = sensores.terreno[10];
			mapaResultado[fil+1][aux] = sensores.terreno[11];
			mapaResultado[fil][aux] = sensores.terreno[12];
			mapaResultado[fil-1][aux] = sensores.terreno[13];
			mapaResultado[fil-2][aux] = sensores.terreno[14];
			mapaResultado[fil-3][aux] = sensores.terreno[15];
			break;
	}
}

void ComportamientoJugador::actualizarMapaPreliminar(Sensores sensores)
{
	int aux;
	mapaPreliminar[fil][col] = sensores.terreno[0];
	switch (brujula) 
	{
		case 0:
			aux = fil;
			aux--;
			mapaPreliminar[aux][col-1] = sensores.terreno[1];
			mapaPreliminar[aux][col] = sensores.terreno[2];
			mapaPreliminar[aux][col+1] = sensores.terreno[3];
			aux--;
			mapaPreliminar[aux][col-2] = sensores.terreno[4];
			mapaPreliminar[aux][col-1] = sensores.terreno[5];
			mapaPreliminar[aux][col] = sensores.terreno[6];
			mapaPreliminar[aux][col+1] = sensores.terreno[7];
			mapaPreliminar[aux][col+2] = sensores.terreno[8];
			aux--;
			mapaPreliminar[aux][col-3] = sensores.terreno[9];
			mapaPreliminar[aux][col-2] = sensores.terreno[10];
			mapaPreliminar[aux][col-1] = sensores.terreno[11];
			mapaPreliminar[aux][col] = sensores.terreno[12];
			mapaPreliminar[aux][col+1] = sensores.terreno[13];
			mapaPreliminar[aux][col+2] = sensores.terreno[14];
			mapaPreliminar[aux][col+3] = sensores.terreno[15];
			break;
		case 1:
			aux = col;
			aux++;
			mapaPreliminar[fil-1][aux] = sensores.terreno[1];
			mapaPreliminar[fil][aux] = sensores.terreno[2];
			mapaPreliminar[fil+1][aux] = sensores.terreno[3];
			aux++;
			mapaPreliminar[fil-2][aux] = sensores.terreno[4];
			mapaPreliminar[fil-1][aux] = sensores.terreno[5];
			mapaPreliminar[fil][aux] = sensores.terreno[6];
			mapaPreliminar[fil+1][aux] = sensores.terreno[7];
			mapaPreliminar[fil+2][aux] = sensores.terreno[8];
			aux++;
			mapaPreliminar[fil-3][aux] = sensores.terreno[9];
			mapaPreliminar[fil-2][aux] = sensores.terreno[10];
			mapaPreliminar[fil-1][aux] = sensores.terreno[11];
			mapaPreliminar[fil][aux] = sensores.terreno[12];
			mapaPreliminar[fil+1][aux] = sensores.terreno[13];
			mapaPreliminar[fil+2][aux] = sensores.terreno[14];
			mapaPreliminar[fil+3][aux] = sensores.terreno[15];
			break;
		case 2:
			aux = fil;
			aux++;
			mapaPreliminar[aux][col+1] = sensores.terreno[1];
			mapaPreliminar[aux][col] = sensores.terreno[2];
			mapaPreliminar[aux][col-1] = sensores.terreno[3];
			aux++;
			mapaPreliminar[aux][col+2] = sensores.terreno[4];
			mapaPreliminar[aux][col+1] = sensores.terreno[5];
			mapaPreliminar[aux][col] = sensores.terreno[6];
			mapaPreliminar[aux][col-1] = sensores.terreno[7];
			mapaPreliminar[aux][col-2] = sensores.terreno[8];
			aux++;
			mapaPreliminar[aux][col+3] = sensores.terreno[9];
			mapaPreliminar[aux][col+2] = sensores.terreno[10];
			mapaPreliminar[aux][col+1] = sensores.terreno[11];
			mapaPreliminar[aux][col] = sensores.terreno[12];
			mapaPreliminar[aux][col-1] = sensores.terreno[13];
			mapaPreliminar[aux][col-2] = sensores.terreno[14];
			mapaPreliminar[aux][col-3] = sensores.terreno[15];
			break;
		case 3:
			aux = col;
			aux--;
			mapaPreliminar[fil+1][aux] = sensores.terreno[1];
			mapaPreliminar[fil][aux] = sensores.terreno[2];
			mapaPreliminar[fil-1][aux] = sensores.terreno[3];
			aux--;
			mapaPreliminar[fil+2][aux] = sensores.terreno[4];
			mapaPreliminar[fil+1][aux] = sensores.terreno[5];
			mapaPreliminar[fil][aux] = sensores.terreno[6];
			mapaPreliminar[fil-1][aux] = sensores.terreno[7];
			mapaPreliminar[fil-2][aux] = sensores.terreno[8];
			aux--;
			mapaPreliminar[fil+3][aux] = sensores.terreno[9];
			mapaPreliminar[fil+2][aux] = sensores.terreno[10];
			mapaPreliminar[fil+1][aux] = sensores.terreno[11];
			mapaPreliminar[fil][aux] = sensores.terreno[12];
			mapaPreliminar[fil-1][aux] = sensores.terreno[13];
			mapaPreliminar[fil-2][aux] = sensores.terreno[14];
			mapaPreliminar[fil-3][aux] = sensores.terreno[15];
			break;
	}
}

void ComportamientoJugador::actualizarMapaReactivo(Sensores sensores)
{
	int auxfil = fil, auxcol = col;
	
	switch (brujula) 
	{
		case 0:
			--auxfil;
			--auxcol;
			for (int i=0; i < 3; ++i)
			{
				if (sensores.terreno[i+1] == 'K')
				{
					mapaReactivo[auxfil][auxcol + i] = -2;
					mapaReactivo[auxfil][auxcol + i + 1] = -1;
					mapaReactivo[auxfil][auxcol + i + 2] = -1;
					mapaReactivo[auxfil][auxcol + i - 1] = -1;
					mapaReactivo[auxfil][auxcol + i - 2] = -1;
					mapaReactivo[auxfil + 1][auxcol + i] = -1;
					mapaReactivo[auxfil + 2][auxcol + i] = -1;
					mapaReactivo[auxfil - 1][auxcol + i] = -1;
					mapaReactivo[auxfil - 2][auxcol + i] = -1;
				}
			}
			--auxfil;
			--auxcol;
			for (int i=0; i < 5; ++i)
			{
				if (sensores.terreno[i+4] == 'K')
				{
					mapaReactivo[auxfil][auxcol + i] = -2;
					mapaReactivo[auxfil][auxcol + i + 1] = -1;
					mapaReactivo[auxfil][auxcol + i + 2] = -1;
					mapaReactivo[auxfil][auxcol + i - 1] = -1;
					mapaReactivo[auxfil][auxcol + i - 2] = -1;
					mapaReactivo[auxfil + 1][auxcol + i] = -1;
					mapaReactivo[auxfil + 2][auxcol + i] = -1;
					mapaReactivo[auxfil - 1][auxcol + i] = -1;
					mapaReactivo[auxfil - 2][auxcol + i] = -1;
				}
			}
			--auxfil;
			--auxcol;
			for (int i=0; i < 7; ++i)
			{
				if (sensores.terreno[i+9] == 'K')
				{
					mapaReactivo[auxfil][auxcol + i] = -2;
					mapaReactivo[auxfil][auxcol + i + 1] = -1;
					mapaReactivo[auxfil][auxcol + i + 2] = -1;
					mapaReactivo[auxfil][auxcol + i - 1] = -1;
					mapaReactivo[auxfil][auxcol + i - 2] = -1;
					mapaReactivo[auxfil + 1][auxcol + i] = -1;
					mapaReactivo[auxfil + 2][auxcol + i] = -1;
					mapaReactivo[auxfil - 1][auxcol + i] = -1;
					mapaReactivo[auxfil - 2][auxcol + i] = -1;
				}
			}
			break;
		case 1:
			++auxcol;
			--auxfil;
			for (int i=0; i < 3; ++i)
			{
				if (sensores.terreno[i+1] == 'K')
				{
					mapaReactivo[auxfil+i][auxcol] = -2;
					mapaReactivo[auxfil+i][auxcol + 1] = -1;
					mapaReactivo[auxfil+i][auxcol - 1] = -1;
					mapaReactivo[auxfil+i + 1][auxcol] = -1;
					mapaReactivo[auxfil+i - 1][auxcol] = -1;
					mapaReactivo[auxfil+i][auxcol + 2] = -1;
					mapaReactivo[auxfil+i][auxcol - 2] = -1;
					mapaReactivo[auxfil+i + 2][auxcol] = -1;
					mapaReactivo[auxfil+i - 2][auxcol] = -1;
				}
			}
			++auxcol;
			--auxfil;
			for (int i=0; i < 5; ++i)
			{
				if (sensores.terreno[i+4] == 'K')
				{
					mapaReactivo[auxfil+i][auxcol] = -2;
					mapaReactivo[auxfil+i][auxcol + 1] = -1;
					mapaReactivo[auxfil+i][auxcol - 1] = -1;
					mapaReactivo[auxfil+i + 1][auxcol] = -1;
					mapaReactivo[auxfil+i - 1][auxcol] = -1;
					mapaReactivo[auxfil+i][auxcol + 2] = -1;
					mapaReactivo[auxfil+i][auxcol - 2] = -1;
					mapaReactivo[auxfil+i + 2][auxcol] = -1;
					mapaReactivo[auxfil+i - 2][auxcol] = -1;
				}
			}
			++auxcol;
			--auxfil;
			for (int i=0; i < 7; ++i)
			{
				if (sensores.terreno[i+9] == 'K')
				{
					mapaReactivo[auxfil+i][auxcol] = -2;
					mapaReactivo[auxfil+i][auxcol + 1] = -1;
					mapaReactivo[auxfil+i][auxcol - 1] = -1;
					mapaReactivo[auxfil+i + 1][auxcol] = -1;
					mapaReactivo[auxfil+i - 1][auxcol] = -1;
					mapaReactivo[auxfil+i][auxcol + 2] = -1;
					mapaReactivo[auxfil+i][auxcol - 2] = -1;
					mapaReactivo[auxfil+i + 2][auxcol] = -1;
					mapaReactivo[auxfil+i - 2][auxcol] = -1;
				}
			}
			break;
		case 2:
			++auxfil;
			++auxcol;
			for (int i=0; i < 3; ++i)
			{
				if (sensores.terreno[i+1] == 'K')
				{
					mapaReactivo[auxfil][auxcol-i] = -2;
					mapaReactivo[auxfil][auxcol-i + 1] = -1;
					mapaReactivo[auxfil][auxcol-i - 1] = -1;
					mapaReactivo[auxfil + 1][auxcol-i] = -1;
					mapaReactivo[auxfil - 1][auxcol-i] = -1;
					mapaReactivo[auxfil][auxcol-i + 2] = -1;
					mapaReactivo[auxfil][auxcol-i - 2] = -1;
					mapaReactivo[auxfil + 2][auxcol-i] = -1;
					mapaReactivo[auxfil - 2][auxcol-i] = -1;
				}
			}
			++auxfil;
			++auxcol;
			for (int i=0; i < 5; ++i)
			{
				if (sensores.terreno[i+4] == 'K')
				{
					mapaReactivo[auxfil][auxcol-i] = -2;
					mapaReactivo[auxfil][auxcol-i + 1] = -1;
					mapaReactivo[auxfil][auxcol-i - 1] = -1;
					mapaReactivo[auxfil + 1][auxcol-i] = -1;
					mapaReactivo[auxfil - 1][auxcol-i] = -1;
					mapaReactivo[auxfil][auxcol-i + 2] = -1;
					mapaReactivo[auxfil][auxcol-i - 2] = -1;
					mapaReactivo[auxfil + 2][auxcol-i] = -1;
					mapaReactivo[auxfil - 2][auxcol-i] = -1;
				}
			}
			++auxfil;
			++auxcol;
			for (int i=0; i < 7; ++i)
			{
				if (sensores.terreno[i+9] == 'K')
				{
					mapaReactivo[auxfil][auxcol-i] = -2;
					mapaReactivo[auxfil][auxcol-i + 1] = -1;
					mapaReactivo[auxfil][auxcol-i - 1] = -1;
					mapaReactivo[auxfil + 1][auxcol-i] = -1;
					mapaReactivo[auxfil - 1][auxcol-i] = -1;
					mapaReactivo[auxfil][auxcol-i + 2] = -1;
					mapaReactivo[auxfil][auxcol-i - 2] = -1;
					mapaReactivo[auxfil + 2][auxcol-i] = -1;
					mapaReactivo[auxfil - 2][auxcol-i] = -1;
				}
			}
			break;
		case 3:
			--auxcol;
			++auxfil;
			for (int i=0; i < 3; ++i)
			{
				if (sensores.terreno[i+1] == 'K')
				{
					mapaReactivo[auxfil-i][auxcol] = -2;
					mapaReactivo[auxfil-i][auxcol + 1] = -1;
					mapaReactivo[auxfil-i][auxcol - 1] = -1;
					mapaReactivo[auxfil-i + 1][auxcol] = -1;
					mapaReactivo[auxfil-i - 1][auxcol] = -1;
					mapaReactivo[auxfil-i][auxcol + 2] = -1;
					mapaReactivo[auxfil-i][auxcol - 2] = -1;
					mapaReactivo[auxfil-i + 2][auxcol] = -1;
					mapaReactivo[auxfil-i - 2][auxcol] = -1;
				}
			}
			--auxcol;
			++auxfil;
			for (int i=0; i < 5; ++i)
			{
				if (sensores.terreno[i+4] == 'K')
				{
					mapaReactivo[auxfil-i][auxcol] = -2;
					mapaReactivo[auxfil-i][auxcol + 1] = -1;
					mapaReactivo[auxfil-i][auxcol - 1] = -1;
					mapaReactivo[auxfil-i + 1][auxcol] = -1;
					mapaReactivo[auxfil-i - 1][auxcol] = -1;
					mapaReactivo[auxfil-i][auxcol + 2] = -1;
					mapaReactivo[auxfil-i][auxcol - 2] = -1;
					mapaReactivo[auxfil-i + 2][auxcol] = -1;
					mapaReactivo[auxfil-i - 2][auxcol] = -1;
				}
			}
			--auxcol;
			++auxfil;
			for (int i=0; i < 7; ++i)
			{
				if (sensores.terreno[i+9] == 'K')
				{
					mapaReactivo[auxfil-i][auxcol] = -2;
					mapaReactivo[auxfil-i][auxcol + 1] = -1;
					mapaReactivo[auxfil-i][auxcol - 1] = -1;
					mapaReactivo[auxfil-i + 1][auxcol] = -1;
					mapaReactivo[auxfil-i - 1][auxcol] = -1;
					mapaReactivo[auxfil-i][auxcol + 2] = -1;
					mapaReactivo[auxfil-i][auxcol - 2] = -1;
					mapaReactivo[auxfil-i + 2][auxcol] = -1;
					mapaReactivo[auxfil-i - 2][auxcol] = -1;
				}
			}
			break;
	}
}

void ComportamientoJugador::copiarMapa(int difFil, int difCol)
{
	for (unsigned int i = 0; i < mapaPreliminar.size()/2; ++i)
		for (unsigned int j = 0; j < mapaPreliminar.size()/2; ++j)
			mapaResultado[i][j] = mapaPreliminar[i+difFil][j+difCol];
}

int ComportamientoJugador::elegirCasilla()
{
	int min;
	int pos;
	
	switch(brujula)
	{
		case 0:
			min = mapaReactivo[fil-1][col];
			pos = 0;
			break;
		case 1:
			min = mapaReactivo[fil][col+1];
			pos = 1;
			break;
		case 2:
			min = mapaReactivo[fil+1][col];
			pos = 2;
			break;
		case 3:
			min = mapaReactivo[fil][col-1];
			pos = 3;
			break;
	}
	
	if (mapaReactivo[fil-1][col] < min)
	{
		min = mapaReactivo[fil-1][col];
		pos = 0;
	}
	if (mapaReactivo[fil][col+1] < min)
	{
		min = mapaReactivo[fil][col+1];
		pos = 1;
	}
	if (mapaReactivo[fil+1][col] < min)
	{
		min = mapaReactivo[fil+1][col];
		pos = 2;
	}
	if (mapaReactivo[fil][col-1] < min)
	{
		min = mapaReactivo[fil][col-1];
		pos = 3;
	}
	
	return pos;
}

struct nodo{
	estado st;
	list<Action> secuencia;
	unsigned int coste;
};

struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};

struct ComparaNodos{
	bool operator()(const nodo &a, const nodo &b) const
	{
		return a.coste > b.coste;
	}
};


// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	stack<nodo> abiertos;											// Lista de Abiertos

	nodo current;
	current.st = origen;
	current.secuencia.empty();

	abiertos.push(current);

  while (!abiertos.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		abiertos.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			abiertos.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			abiertos.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				abiertos.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la abiertos
		if (!abiertos.empty()){
			current = abiertos.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados, frontera;
	queue<nodo> abiertos;											// Lista de Abiertos

	nodo current;
	current.st = origen;
	current.secuencia.empty();

	abiertos.push(current);
	frontera.insert(current.st);

  while (!abiertos.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		abiertos.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end())
		{
			if (frontera.find(hijoTurnR.st) == frontera.end())
			{
				hijoTurnR.secuencia.push_back(actTURN_R);
				abiertos.push(hijoTurnR);
				frontera.insert(hijoTurnR.st);
			}
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end())
		{
			if (frontera.find(hijoTurnL.st) == frontera.end())
			{
				hijoTurnL.secuencia.push_back(actTURN_L);
				abiertos.push(hijoTurnL);
				frontera.insert(hijoTurnL.st);
			}
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end())
			{
				if (frontera.find(hijoForward.st) == frontera.end())
				{
					hijoForward.secuencia.push_back(actFORWARD);
					abiertos.push(hijoForward);
					frontera.insert(hijoForward.st);
				}
			}
		}

		// Tomo el siguiente valor de la abiertos
		if (!abiertos.empty()){
			current = abiertos.front();
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

bool ComportamientoJugador::pathFinding_CosteUniforme(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	map<estado, int, ComparaEstados> frontera;
	priority_queue<nodo, vector<nodo>, ComparaNodos> abiertos;											// Lista de Abiertos
	
	nodo current;
	current.st = origen;
	current.secuencia.empty();
	current.coste = 1;

	abiertos.push(current);
	frontera[current.st] = current.coste;

	while (!abiertos.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		abiertos.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		hijoTurnR.coste = current.coste += 1;
		if (generados.find(hijoTurnR.st) == generados.end())
		{
			if (frontera.find(hijoTurnR.st) == frontera.end())
			{
				hijoTurnR.secuencia.push_back(actTURN_R);
				abiertos.push(hijoTurnR);
				frontera[hijoTurnR.st] = hijoTurnR.coste;
			}
			else
			{
				if (hijoTurnR.coste < frontera[hijoTurnR.st])
				{
					hijoTurnR.secuencia.push_back(actTURN_R);
					abiertos.push(hijoTurnR);
					frontera[hijoTurnR.st] = hijoTurnR.coste;
				}
			}
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.coste = current.coste += 1;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end())
		{
			if (frontera.find(hijoTurnL.st) == frontera.end())
			{
				hijoTurnL.secuencia.push_back(actTURN_L);
				abiertos.push(hijoTurnL);
				frontera[hijoTurnL.st] = hijoTurnL.coste;
			}
			else
			{
				if (hijoTurnL.coste < frontera[hijoTurnL.st])
				{
					hijoTurnL.secuencia.push_back(actTURN_L);
					abiertos.push(hijoTurnL);
					frontera[hijoTurnL.st] = hijoTurnL.coste;
				}
			}
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			hijoForward.coste = current.coste += costeCasilla(hijoForward.st);
			if (generados.find(hijoForward.st) == generados.end())
			{
				if (frontera.find(hijoForward.st) == frontera.end())
				{
					hijoForward.secuencia.push_back(actFORWARD);
					abiertos.push(hijoForward);
					frontera[hijoForward.st] = hijoForward.coste;
				}
				else
				{
					if (hijoTurnL.coste < frontera[hijoTurnL.st])
					{
						hijoTurnL.secuencia.push_back(actTURN_L);
						abiertos.push(hijoTurnL);
						frontera[hijoTurnL.st] = hijoTurnL.coste;
					}
				}
			}
		}

		// Tomo el siguiente valor de la abiertos
		if (!abiertos.empty()){
			current = abiertos.top();
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}



void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}


// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}



int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
