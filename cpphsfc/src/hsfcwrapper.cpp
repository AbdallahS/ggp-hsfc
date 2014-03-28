#include <iterator>
#include <sstream>
#include <cstring>
#include <boost/foreach.hpp>
#include <boost/variant/get.hpp>
#include <boost/functional/hash.hpp>
#include <hsfc/impl/hsfcwrapper.h>
#include <hsfc/hsfcexception.h>
#include "sexprtoflat.h"

namespace HSFC
{

/*****************************************************************************************
 * Implementation of HSFCManager
 *****************************************************************************************/


/*****************************************************************************************
 * Internal extra functions. 
 * Note: must only be called after Initialise()
 *****************************************************************************************/

void HSFCManager::populatePlayerNamesFromLegalMoves()
{
	hsfcState* tmpstate;
	try
	{
		tmpstate = this->CreateGameState();
		
		std::vector<hsfcLegalMove> legalmoves;
		this->GetLegalMoves(*tmpstate, legalmoves);
		BOOST_FOREACH(hsfcLegalMove& lm, legalmoves)
		{
			Term term;
			parse_flat(lm.Text, term);
			if (term.children_.size() != 3)
				throw HSFCException() << ErrorMsgInfo("HSFC internal error: move_.Text term != 3");
			if (boost::get<std::string>(term.children_[0]) != "does")
				throw HSFCException() << ErrorMsgInfo("HSFC internal error: move_.Text not 'does' relation");
			playernames_[lm.RoleIndex] = boost::get<std::string>(term.children_[1]);
		}
		BOOST_FOREACH(const std::string& s, playernames_)
		{
			if (s.empty())
				throw HSFCException() << ErrorMsgInfo("HSFC internal error: Failed to find names for roles");
		}
	} catch (...)
	{
		this->FreeGameState(tmpstate);
	}
}


/*****************************************************************************************
 * Functions that match the hsfcGDLManager
 *****************************************************************************************/

void HSFCManager::Initialise(const std::string& GDLFileName, const hsfcGDLParamaters& Parameters)
{

	std::string tmpstr(GDLFileName); // needed to avoid non-const in hsfcGDLManager::Initialise
	hsfcGDLParamaters params(Parameters);
	int result = internal_->Initialise(&tmpstr, params);
	if (result != 0)
	{
		std::ostringstream ss;
		ss << "Failed to initialise the HSFC engine. Error code: " << result;
		throw HSFCException() << ErrorMsgInfo(ss.str());
	}	

	// Now jump through hoops to setup workout the names of the players.
	playernames_.assign(this->NumPlayers(), std::string());	
	populatePlayerNamesFromLegalMoves();
}

hsfcState* HSFCManager::CreateGameState()
{
	hsfcState* state;	
	if (internal_->CreateGameState(&state))
	{
		throw HSFCException() << ErrorMsgInfo("Failed to create HSFC game state");
	}
	return state;
}

void HSFCManager::FreeGameState(hsfcState* GameState)
{
	internal_->FreeGameState(GameState);
}

void HSFCManager::CopyGameState(hsfcState& Destination, const hsfcState& Source)
{
	hsfcState& tmp = const_cast<hsfcState&>(Source);
	internal_->CopyGameState(&Destination, &tmp);
}

void HSFCManager::SetInitialGameState(hsfcState& GameState)
{
	internal_->SetInitialGameState(&GameState);
}

void HSFCManager::GetLegalMoves(const hsfcState& GameState, std::vector<hsfcLegalMove>& LegalMove) const
{
	internal_->GetLegalMoves(const_cast<hsfcState*>(&GameState), LegalMove);
}

void HSFCManager::DoMove(hsfcState& GameState, const std::vector<hsfcLegalMove>& LegalMove)
{
	internal_->DoMove(&GameState, const_cast<std::vector<hsfcLegalMove>&>(LegalMove));
}

bool HSFCManager::IsTerminal(const hsfcState& GameState) const
{
	internal_->IsTerminal(const_cast<hsfcState*>(&GameState));
}

void HSFCManager::GetGoalValues(const hsfcState& GameState, std::vector<int>& GoalValue) const
{
	internal_->GetGoalValues(const_cast<hsfcState*>(&GameState), GoalValue);
}

void HSFCManager::PlayOut(hsfcState& GameState, std::vector<int>& GoalValue)
{
	internal_->PlayOut(&GameState, GoalValue);
}

void HSFCManager::PrintState(const hsfcState& GameState) const
{
	internal_->PrintState(const_cast<hsfcState*>(&GameState));
}

/*****************************************************************************************
 * Extra functions
 *****************************************************************************************/

unsigned int HSFCManager::NumPlayers() const
{
	return playernames_.size();
}

std::ostream& HSFCManager::PrintPlayer(std::ostream& os, unsigned int roleid) const
{
	if (roleid >= this->NumPlayers())
		throw HSFCException() << ErrorMsgInfo("HSFC internal error: not a valid roleid");
	return os << playernames_[roleid];
}

/*****************************************************************************************
 * To extract the Move text
 *****************************************************************************************/


struct gdl_move_visitor : public boost::static_visitor<std::ostream&>
{
	std::ostream& os_;
	gdl_move_visitor(std::ostream& os) : os_(os){}
	std::ostream& operator()(const std::string& name) { return os_ << name; }
	std::ostream& operator()(const Term& t){ return generate_sexpr(t,os_); }
};

std::ostream& HSFCManager::PrintMove(std::ostream& os, const hsfcLegalMove& legalmove) const
{
	Term term;
	parse_flat(legalmove.Text, term);
	if (term.children_.size() != 3)
		throw HSFCException() << ErrorMsgInfo("HSFC internal error: move_.Text term != 3");
	if (boost::get<std::string>(term.children_[0]) != "does")
		throw HSFCException() << ErrorMsgInfo("HSFC internal error: move_.Text not 'does' relation");
	gdl_move_visitor gmv(os);
	boost::apply_visitor(gmv, term.children_[2]);
	return os;
}


};
