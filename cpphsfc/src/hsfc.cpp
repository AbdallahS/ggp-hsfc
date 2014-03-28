#include <iterator>
#include <sstream>
#include <cstring>
#include <boost/variant/get.hpp>
#include <boost/functional/hash.hpp>
#include <hsfc/hsfc.h>
#include "sexprtoflat.h"

namespace HSFC
{

/*****************************************************************************************
 * Implementation of Player
 *****************************************************************************************/
Player::Player(const HSFCManager* manager, unsigned int roleid): manager_(manager), roleid_(roleid)
{ }

Player::Player(const Player& other): manager_(other.manager_), roleid_(other.roleid_)
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
	this->manager_ = other.manager_;
	this->roleid_ = other.roleid_;
	return *this;
}

std::size_t Player::hash_value() const
{
	// Note: since there is only 1 manager per game
	// we can use the pointer as a hash value.
	size_t seed = 0;
	boost::hash_combine(seed, roleid_);
	boost::hash_combine(seed, manager_);
	return seed;
}


std::size_t hash_value(const Player& player)
{
	return player.hash_value();
}


std::ostream& operator<<(std::ostream& os, const Player& player)
{
	return player.manager_->PrintPlayer(os, player.roleid_);
}


/*****************************************************************************************
 * Implementation of Move
 *****************************************************************************************/

Move::Move(HSFCManager* manager, const hsfcLegalMove& move): manager_(manager), move_(move)
{ }

Move::Move(const Move& other): manager_(other.manager_), move_(other.move_)
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
	return (manager_ == other.manager_) && (move_ == other.move_);
}

bool Move::operator!=(const Move& other) const
{
	return (manager_ != other.manager_) || (move_ != other.move_);
}

Move& Move::operator=(const Move& other)
{
	manager_ = other.manager_;
	move_ = other.move_;
	return *this;
}

std::ostream& operator<<(std::ostream& os, const Move& move)
{
	return move.manager_->PrintMove(os, move.move_);
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

	manager_.Initialise(gdlfilename, params);
	initstate_.reset(new State(&manager_));
}

const State& Game::initState() const
{
	return *initstate_;
}


unsigned int Game::numPlayers() const
{
	return manager_.NumPlayers();
}


void Game::players(std::vector<Player>& plyrs) const
{
	this->players(std::back_inserter(plyrs));
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

State::State(HSFCManager* manager): manager_(manager), state_(NULL)
{  
	state_ = manager_->CreateGameState();
	manager_->SetInitialGameState(*state_);
}

State::State(const State& other) : manager_(other.manager_), state_(NULL)
{
	state_ = manager_->CreateGameState();
	manager_->CopyGameState(*state_, *(other.state_));
}

State& State::operator=(const State& other)
{
	BOOST_ASSERT_MSG(manager_ == other.manager_, "Cannot assign to States from different games");
	if (state_ != NULL)
		manager_->FreeGameState(state_);
	manager_->CopyGameState(*state_, *(other.state_));
	return *this;
}

State::~State()
{
	if (state_ != NULL)
	manager_->FreeGameState(state_);
}

bool State::isTerminal() const
{
	return manager_->IsTerminal(*state_);
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

};
