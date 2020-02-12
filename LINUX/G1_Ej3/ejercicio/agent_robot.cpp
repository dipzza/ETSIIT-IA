#include "agent_robot.h"
#include "environment.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <utility>

using namespace std;

// ------------------ Solucion apartado (a) ---------------------------------
/* Agent::ActionType Agent::Think()
{
	ActionType accion = Agent::actFORWARD;
	
	if (girando)
	{
		accion = Agent::actTURN_L;
		girando = false;
	}
	else if (CNY70_ && HeEstadoEnFrontera)
	{
		accion = Agent::actIDLE;
		cout << "Casillas = " << n_pasos << endl;
	}
	else if (CNY70_)
	{
		accion = Agent::actTURN_L;
		n_pasos = 1;
		girando = true;
		HeEstadoEnFrontera = true;
	}
	else 
	{
		accion = Agent::actFORWARD;
		n_pasos++;
	}
	
	return accion;
}
*/
Agent::ActionType Agent::Think()
{
	ActionType accion = Agent::actFORWARD;
	
	if (finalizado)
	{
		accion = Agent::actIDLE;
	}
	else if (BUMPER_ == true)
	{
		accion = Agent::actIDLE;
		cout << "Encontrado" << endl;
	}
	else if (girorecorre == 1)
	{
		if (CNY70_)
		{
			finalizado = true;
			accion = Agent::actIDLE;
		}
		else
		{
			accion = Agent::actFORWARD;
			girorecorre = 2;
		}
	}
	else if (girorecorre == 2)
	{
		if (derecha)
		{
			accion = Agent::actTURN_R;
			derecha = false;
		}
		else
		{
			accion = Agent::actTURN_L;
			derecha = true;
		}
		girorecorre = 0;
	}
	else if (recorriendo && CNY70_)
	{
		if (derecha)
			accion = Agent::actTURN_R;
		else
			accion = Agent::actTURN_L;
		girorecorre = 1;
	}
	else if (recorriendo)
	{
		accion = Agent::actFORWARD;
	}
	else if (CNY70_ && HeEstadoEnFrontera)
	{
		accion = Agent::actTURN_L;
		recorriendo = true;
	}
	else if (CNY70_)
	{
		accion = Agent::actTURN_L;
		HeEstadoEnFrontera = true;
	}
	else 
	{
		accion = Agent::actFORWARD;
	}
	
	return accion;
}

// -----------------------------------------------------------
void Agent::Perceive(const Environment &env)
{
	CNY70_ = env.isFrontier();
	BUMPER_ = env.isBump();
}
// -----------------------------------------------------------
string ActionStr(Agent::ActionType accion)
{
	switch (accion)
	{
	case Agent::actFORWARD: return "FORWARD";
	case Agent::actTURN_L: return "TURN LEFT";
	case Agent::actTURN_R: return "TURN RIGHT";
	case Agent::actBACKWARD: return "BACKWARD";
	case Agent::actPUSH: return "PUSH";
	case Agent::actIDLE: return "IDLE";
	default: return "???";
	}
}
