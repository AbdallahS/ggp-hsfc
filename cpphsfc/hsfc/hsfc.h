/*****************************************************************************************
 *
 *****************************************************************************************/

#ifndef HSFC_H
#define HSFC_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/exception/all.hpp>

#include <hsfcAPI.h>

namespace HSFC
{


typedef boost::error_info<struct tag_Message, std::string> ErrorMsgInfo;
class HSFCException : public boost::exception, public std::exception {};

/*****************************************************************************************
 * Player (role) and Move classes. Note: To clear any ambiguity Move represents the 
 * move as independent from a player, so not the move as take by a player.
 *****************************************************************************************/

class Player
{
	friend class Game;
	friend class State;
	friend std::ostream& operator<<(std::ostream& os, const Player& player);

	unsigned int roleid_;
	Player(unsigned int roleid);	
public:
	const std::string& tostring(std::string&) const;
};

std::ostream& operator<<(std::ostream& os, const Player& player);

class Move
{
	friend class State;
	friend std::ostream& operator<<(std::ostream& os, const Move& move);

	hsfcLegalMove move_;
	Move(const hsfcLegalMove& move);
public:
	const std::string& tostring(std::string&) const;
};

std::ostream& operator<<(std::ostream& os, const Move& move);


/*****************************************************************************************
 *
 *****************************************************************************************/

class State;

class Game
{
	friend class State;
	hsfcGDLManager manager_;
	Game(const Game& other);  // make sure we can't copy this object
public:
	Game(const std::string& gdlfilename);
	void players(std::vector<Player>& players) const;
	unsigned int numPlayers() const;
	bool operator==(const Game& other) const;
};


/*****************************************************************************************
 *
 *****************************************************************************************/

typedef std::pair<Player,Move> PlayerMove;
typedef std::pair<Player,unsigned int> PlayerGoal;

class State
{
	hsfcState* state_;
	Game& game_;
public:
	State(Game& game);
	State(const State& other);
	State& operator=(const State& other);
	~State();

	bool isTerminal() const;

	/* 
	 * Return the legal moves. Must be called only in non-terminal states.
	 *
	 * Will throw an exception if there is not at least one move per player.
	 */
	void legals(std::vector<PlayerMove>& moves) const;

	/* 
	 * Return the goals. Must be called only in terminal states.
	 *
	 * Will throw an exception if there is not exactly one goal per player.
	 */
	void goals(std::vector<PlayerGoal>& results) const;

	/* 
	 * Return the goals. There must be exactly one move per player.
	 */
	void play(const std::vector<PlayerMove>& moves);

	
	void playOut(std::vector<PlayerGoal>& results);


/*
	// FOR INTEREST:
	// Interface 2
	template<typename OutputIterator>
	void legals(OutputIterator ot)
	{	
		boost::unordered_set ok;
		std::vector<hsfcLegalMove> lms;
		BOOST_ASSERT(game_.IsTerminal(*state_));
		game_.GetLegalMoves(*state_, lms);
		BOOST_FOREACH( hsfcLegalMove& lm, lms)
		{
			*(ot++) = PlayerMove(lm.RoleIndex, lm);
			ok[lm.RoleIndex];
		}
		if (ok.size() != game_.NumRoles)
		{
			//			throw excemtpin
		}
	}

	// Interface 3
	template<typename Fn>
	void legals(Fn fn)
	{	
		boost::unordered_set ok;
		std::vector<hsfcLegalMove> lms;
		BOOST_ASSERT(game_.IsTerminal(*state_));
		game_.GetLegalMoves(*state_, lms);
		BOOST_FOREACH( hsfcLegalMove& lm, lms)
		{
			PlayerMove pm(lm.RoleIndex, lm);
			ok[lm.RoleIndex];
			fn(pm);
		}
		if (ok.size() != game_.NumRoles)
		{
			//			throw excemtpin
		}
	}
*/


};

};


#endif // HSFC_H
