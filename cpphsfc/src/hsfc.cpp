#include <sstream>
#include <hsfc/hsfc.h>

namespace HSFC
{

/*****************************************************************************************
 * Implementation of Player
 *****************************************************************************************/
Player::Player(unsigned int roleid): roleid_(roleid)
{ }

const std::string& Player::tostring(std::string& str) const
{
//	static std::ostringstream ss;
	std::ostringstream ss;
	ss << "player" << roleid_;
	str = ss.str();
	//return ss.str();
    return str;
}

/*****************************************************************************************
 * Implementation of Move
 *****************************************************************************************/

Move::Move(const hsfcLegalMove& move): move_(move)
{ }

const std::string& Move::tostring(std::string& str) const
{
	std::ostringstream ss;
	ss << move_.Text;

	str = ss.str();
	return str;
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
}

void Game::players(std::vector<Player>& players) const
{

	// Note: currently no way to match the roleid to the name.
	for (unsigned int i = 0; i < manager_.NumRoles; ++i)
	{
		players.push_back(Player(i)); //  Note: assuming index starts at 0.
	}
}

unsigned int Game::numPlayers() const
{
	return (unsigned int)manager_.NumRoles;
}

bool Game::operator==(const Game& other) const
{
	// Note: because I disable the Game copy constructor I
	// use a pointer check. Maybe this is a bit dodgy.
	return this == &other;
}



State::State(Game& game): game_(game)
{  
	if (game_.manager_.CreateGameState(&state_))
	{
		throw HSFCException() << ErrorMsgInfo("Failed to create HSFC game state");
	}	
	game_.manager_.SetInitialGameState(state_);
}

State::State(const State& other) : game_(other.game_)
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
	game_.manager_.CopyGameState(state_, other.state_);
}

State::~State()
{
	game_.manager_.FreeGameState(state_);
}

bool State::isTerminal() const
{
	return game_.manager_.IsTerminal(state_);
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

};
