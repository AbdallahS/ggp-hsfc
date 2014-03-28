/*****************************************************************************************
 * This is a thin wrapper around the HSFC:
 * 1) wrapper class is in the HSFC namespace.
 * 2) is const correct
 * 3) adds a few extra useful functions
 *****************************************************************************************/

#ifndef HSFC_WRAPPER_H
#define HSFC_WRAPPER_H

#include <string>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <hsfc/impl/hsfcAPI.h>

namespace HSFC
{

class HSFCManager
{
private:
	boost::scoped_ptr<hsfcGDLManager> internal_;

	// Because the hsfcGDLManager doesn't provide an easy way to get
	// the role names we need to jump through some hoops. Running
	// populatePlayerNamesFromLegalMoves() is also important because 
	// the internals are not properly initialised until a state
	// has been created.
	std::vector<std::string> playernames_;
	void populatePlayerNamesFromLegalMoves();

public:
	HSFCManager();

	/* Functions from hsfcGDLManager with const fixes */
	void Initialise(const std::string& GDLFileName, const hsfcGDLParamaters& Paramaters);
	hsfcState* CreateGameState();
	void FreeGameState(hsfcState* GameState);
	void CopyGameState(hsfcState& Destination, const hsfcState& Source);
	void SetInitialGameState(hsfcState& GameState);
	void GetLegalMoves(const hsfcState& GameState, std::vector<hsfcLegalMove>& LegalMove) const;
	void DoMove(hsfcState& GameState, const std::vector<hsfcLegalMove>& LegalMove);
	bool IsTerminal(const hsfcState& GameState) const;
	void GetGoalValues(const hsfcState& GameState, std::vector<int>& GoalValue) const;
	void PlayOut(hsfcState& GameState, std::vector<int>& GoalValue);
	void PrintState(const hsfcState& GameState) const;

	/* Additional functions - note: capitalised first letters for class consistency. */
	unsigned int NumPlayers() const;  
	std::ostream& PrintPlayer(std::ostream& os, unsigned int roleid) const;
	std::ostream& PrintMove(std::ostream& os, const hsfcLegalMove& legalmove) const;	
};

};

#endif /* HSFC_WRAPPER_H */