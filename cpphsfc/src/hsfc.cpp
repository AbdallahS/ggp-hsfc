#include <iterator>
#include <sstream>
#include <cstring>
#include <boost/variant/get.hpp>
#include <hsfc/hsfc.h>
#include "sexprtoflat.h"

namespace HSFC
{

/*****************************************************************************************
 * Implementation of Player
 *****************************************************************************************/
Player::Player(const Game* game, unsigned int roleid): game_(game), roleid_(roleid)
{ }

Player::Player(const Player& other): game_(other.game_), roleid_(other.roleid_)
{ }

std::string Player::tostring() const
{
	std::ostringstream ss;
	ss << *this;
	return ss.str();	
}

bool Player::operator==(const Player& other) const
{
	return roleid_ == other.roleid_;
}

bool Player::operator!=(const Player& other) const
{
	return roleid_ != other.roleid_;
}

Player& Player::operator=(const Player& other)
{
	this->game_ = other.game_;
	this->roleid_ = other.roleid_;
	return *this;
}

std::ostream& operator<<(std::ostream& os, const Player& player)
{
	return os << player.game_->getPlayerName(player.roleid_);
}


/*****************************************************************************************
 * Implementation of Move
 *****************************************************************************************/

Move::Move(const hsfcLegalMove& move): move_(move)
{ }

Move::Move(const Move& other): move_(other.move_)
{ }

std::string Move::tostring() const
{
	std::ostringstream ss;
	ss << *this;
	return ss.str();	
}

bool operator==(const hsfcLegalMove& a, const hsfcLegalMove& b)
{
	return (a.RoleIndex == b.RoleIndex && 
			strcmp(a.Text, b.Text) == 0 &&
			a.Tuple.RelationIndex == b.Tuple.RelationIndex &&
			a.Tuple.ID == b.Tuple.ID);
}

bool operator!=(const hsfcLegalMove& a, const hsfcLegalMove& b)
{
	return !(a == b);
}


bool Move::operator==(const Move& other) const 
{
	return move_ == other.move_;
}

bool Move::operator!=(const Move& other) const
{
	return move_ != other.move_;
}

Move& Move::operator=(const Move& other)
{
	move_ = other.move_;
	return *this;
}

struct gdl_move_visitor : public boost::static_visitor<std::ostream&>
{
	std::ostream& os_;
	gdl_move_visitor(std::ostream& os) : os_(os){}
	std::ostream& operator()(const std::string& name) { return os_ << name; }
	std::ostream& operator()(const Term& t){ return generate_sexpr(t,os_); }
};

std::ostream& operator<<(std::ostream& os, const Move& move)
{
	Term term;
	parse_flat(move.move_.Text, term);
	if (term.children_.size() != 3)
		throw HSFCException() << ErrorMsgInfo("HSFC internal error: move_.Text term != 3");
	if (boost::get<std::string>(term.children_[0]) != "does")
		throw HSFCException() << ErrorMsgInfo("HSFC internal error: move_.Text not 'does' relation");
	gdl_move_visitor gmv(os);
	return boost::apply_visitor(gmv, term.children_[2]);
}


/*****************************************************************************************
 * Implementation of Game
 *****************************************************************************************/
Game::Game(const Game& other) { }

Game::Game(const std::string& gdlfilename)
{
	// Note: not really sure what are sane options here so copying
	// from Michael's example code.
	hsfcGDLParamaters params; // FIXUP: Get Michael to fix: spelling mistake in the type name
	params.ReadGDLOnly = false;			// Validate the GDL without creating the schema
	params.SchemaOnly = false;			// Validate the GDL & Schema without grounding the rules
	params.MaxRelationSize = 1000000;	// Max bytes per relation for high speed storage
	params.MaxReferenceSize = 1000000;	// Max bytes per lookup table for grounding
	params.OrderRules = true;			// Optimise the rule execution cost

	string tmpstr(gdlfilename); // needed to avoid non-const in hsfcGDLManager::Initialise
	int result = manager_.Initialise(&tmpstr, params);
	if (result != 0)
	{
		std::ostringstream ss;
		ss << "Failed to initialise the HSFC engine. Error code: " << result;
		throw HSFCException() << ErrorMsgInfo(ss.str());
	}	

	// Usefull to maintain a link to an initial state. Also within the HSFC internals, 
	// the roles (and maybe other things) are not initialised until a state is created.	
	initstate_.reset(new State(*this));

	// Now jump through hoops to setup workout the names of the players.
	playernames_.assign(this->numPlayers(), std::string());	
	populatePlayerNamesFromLegalMoves(*initstate_);
}

void Game::populatePlayerNamesFromLegalMoves(State& state)
{
	std::vector<PlayerMove> pms;
	state.legals(pms);
	BOOST_FOREACH(PlayerMove& pm, pms)
	{
		Term term;
		parse_flat(pm.second.move_.Text, term);
		if (term.children_.size() != 3)
			throw HSFCException() << ErrorMsgInfo("HSFC internal error: move_.Text term != 3");
		if (boost::get<std::string>(term.children_[0]) != "does")
			throw HSFCException() << ErrorMsgInfo("HSFC internal error: move_.Text not 'does' relation");
		playernames_[pm.second.move_.RoleIndex] = boost::get<std::string>(term.children_[1]);
	}
	BOOST_FOREACH(const std::string& s, playernames_)
	{
		if (s.empty())
			throw HSFCException() << ErrorMsgInfo("HSFC internal error: Failed to find names for roles");
	}
}

const std::string& Game::getPlayerName(unsigned int roleid) const
{
	if (roleid >= this->numPlayers())
		throw HSFCException() << ErrorMsgInfo("HSFC internal error: not a valid roleid");
	return playernames_[roleid];
}


void Game::players(std::vector<Player>& plyrs) const
{
	this->players(std::back_inserter(plyrs));
}

unsigned int Game::numPlayers() const
{
	return (unsigned int)manager_.NumRoles;
}

const State& Game::initState() const
{
	return *initstate_;
}

bool Game::operator==(const Game& other) const
{
	// Note: because I disable the Game copy constructor I
	// use a pointer check. Maybe this is a bit dodgy.
	return this == &other;
}

/*****************************************************************************************
 * Implementation of State
 *****************************************************************************************/

State::State(Game& game): game_(game), state_(NULL)
{  
	if (game_.manager_.CreateGameState(&state_))
	{
		throw HSFCException() << ErrorMsgInfo("Failed to create HSFC game state");
	}	
	game_.manager_.SetInitialGameState(state_);
}

State::State(const State& other) : game_(other.game_), state_(NULL)
{
	if (game_.manager_.CreateGameState(&state_))
	{
		throw HSFCException() << ErrorMsgInfo("Failed to create HSFC game state");
	}	
	game_.manager_.CopyGameState(state_, other.state_);
}

State& State::operator=(const State& other)
{
	BOOST_ASSERT_MSG(game_ == other.game_, "Cannot assign to States from different games");
	if (state_ != NULL)
		game_.manager_.FreeGameState(state_);
	game_.manager_.CopyGameState(state_, other.state_);
	return *this;
}

State::~State()
{
	if (state_ != NULL)
	game_.manager_.FreeGameState(state_);
}

bool State::isTerminal() const
{
	return game_.manager_.IsTerminal(state_);
}

void State::legals(std::vector<PlayerMove>& moves) const
{
	this->legals(std::back_inserter(moves));
}

void State::goals(std::vector<PlayerGoal>& results) const
{
	this->goals(std::back_inserter(results));
}

void State::playout(std::vector<PlayerGoal>& results) 
{
	this->playout(std::back_inserter(results));
}

void State::play(const std::vector<PlayerMove>& moves)
{
	this->play(moves.begin(), moves.end());
}


/*
//  OLD VERSION BEFORE THE TEMPLATED FUNCTIONS WERE INTRODUCED

void Game::players(std::vector<Player>& plyrs) const
{
	// Note: currently no way to match the roleid to the name.
	for (unsigned int i = 0; i < manager_.NumRoles; ++i)
	{
		players.push_back(Player(i)); //  Note: assuming index starts at 0.
	}
}

void State::legals(std::vector<PlayerMove>& moves) const
{
	boost::unordered_set<int> ok;
	std::vector<hsfcLegalMove> lms;
	BOOST_ASSERT_MSG(!(this->isTerminal()), "Test for non-terminal state before calling legals()");
	game_.manager_.GetLegalMoves(state_, lms);
	BOOST_FOREACH( hsfcLegalMove& lm, lms)
	{
		moves.push_back(PlayerMove(lm.RoleIndex, lm));
		ok.insert(lm.RoleIndex);
	}
	if (ok.size() != game_.numPlayers())
	{
		throw HSFCException() << ErrorMsgInfo("HSFC internal error: missing moves for some players");
	}
}

void State::goals(std::vector<PlayerGoal>& results) const
{
	std::vector<int> vals;
	BOOST_ASSERT_MSG(this->isTerminal(), "Test for terminal state before calling goals()");
	game_.manager_.GetGoalValues(state_, vals);
	if (vals.size() != game_.numPlayers())
	{
		throw HSFCException() << ErrorMsgInfo("HSFC internal error: no goal value for some players");
	}
	
	for (unsigned int i = 0; i < vals.size(); ++i)
	{
		results.push_back(PlayerGoal(i, (unsigned int)vals[i]));
	}
}

void State::playOut(std::vector<PlayerGoal>& results) 
{
	std::vector<int> vals;
	BOOST_ASSERT_MSG(!this->isTerminal(), "Test for terminal state before calling playOut()");
	game_.manager_.PlayOut(state_, vals);
	if (vals.size() != game_.numPlayers())
	{
		throw HSFCException() << ErrorMsgInfo("HSFC internal error: no goal value for some players");
	}
	
	for (unsigned int i = 0; i < vals.size(); ++i)
	{
		results.push_back(PlayerGoal(i, (unsigned int)vals[i]));
	}
}

void State::play(const std::vector<PlayerMove>& moves)
{
	boost::unordered_set<int> ok;
	std::vector<hsfcLegalMove> lms;
	BOOST_ASSERT_MSG(!(this->isTerminal()), "Test for non-terminal state before calling play()");
	BOOST_FOREACH(const PlayerMove& lm, moves)
	{
		BOOST_ASSERT_MSG(lm.first.roleid_ == lm.second.move_.RoleIndex, "Player and Move do not match");
		lms.push_back(lm.second.move_);
		ok.insert(lm.first.roleid_);
	}
	BOOST_ASSERT_MSG(ok.size() == game_.numPlayers(), "Must be exactly one move per player");
	game_.manager_.DoMove(state_, lms);
}

*/

};
