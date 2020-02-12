#ifndef AGENT__
#define AGENT__

#include <string>
#include <iostream>
using namespace std;

// -----------------------------------------------------------
//				class Agent
// -----------------------------------------------------------
class Environment;
class Agent
{
public:
	Agent(){
		CNY70_ = false;
		BUMPER_ = false;
		
		HeEstadoEnFrontera = false;
		girando = false;
		recorriendo = false;
		derecha = false;
		girorecorre = 0;
		finalizado = false;
	}

	enum ActionType
	{
	    actFORWARD,
	    actTURN_L,
	    actTURN_R,
		actBACKWARD,
		actPUSH,
		actIDLE
	};

	void Perceive(const Environment &env);
	ActionType Think();

private:
	bool CNY70_;
	bool BUMPER_;
	
	//Variables de estado
	bool HeEstadoEnFrontera;
	bool girando;
	bool recorriendo;
	bool derecha;
	int girorecorre;
	bool finalizado;
};

string ActionStr(Agent::ActionType);

#endif
