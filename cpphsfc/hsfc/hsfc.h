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
	std::string tostring() const;
};

std::ostream& operator<<(std::ostream& os, const Player& player);

class Move
{
	friend class State;
	friend std::ostream& operator<<(std::ostream& os, const Move& move);

	hsfcLegalMove move_;
	Move(const hsfcLegalMove& move);
public:
	std::string tostring() const;
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
	// Note: might look at changing this to either have an extra 
	// constructor that works from strings or an istream that will
    // work for both files and strings.
	Game(const std::string& gdlfilename);
	void players(std::vector<Player>& plyrs) const;
	unsigned int numPlayers() const;
	bool operator==(const Game& other) const;

	template<typename OutputIterator>
	void players(OutputIterator dest) const
	{
		// Note: currently no way to match the roleid to the name.
        //  Also assuming RoleIndex starts at 0.
		for (unsigned int i = 0; i < manager_.NumRoles; ++i)
		{
			*dest++=Player(i); 
		}

	}
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

	template<typename OutputIterator>
	void legals(OutputIterator dest) const
	{
		boost::unordered_set<int> ok;
		std::vector<hsfcLegalMove> lms;
		BOOST_ASSERT_MSG(!(this->isTerminal()), "Test for non-terminal state before calling legals()");
		game_.manager_.GetLegalMoves(state_, lms);
		BOOST_FOREACH( hsfcLegalMove& lm, lms)
		{
			dest++=PlayerMove(lm.RoleIndex, lm);
			ok.insert(lm.RoleIndex);
		}
		if (ok.size() != game_.numPlayers())
		{
			throw HSFCException() << ErrorMsgInfo("HSFC internal error: missing moves for some players");
		}
	}


	/* 
	 * Return the goals. Must be called only in terminal states.
	 *
	 * Will throw an exception if there is not exactly one goal per player.
	 */
	void goals(std::vector<PlayerGoal>& results) const;

	template<typename OutputIterator>
	void goals(OutputIterator dest) const
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
			dest++=PlayerGoal(i, (unsigned int)vals[i]);
		}		
	}

	/* 
	 * Return the goals after a playout. There must be exactly one move per player.
	 */
	void playOut(std::vector<PlayerGoal>& dest);
	
	template<typename OutputIterator>
	void playOut(OutputIterator dest)
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
			dest++= PlayerGoal(i, (unsigned int)vals[i]);
		}
	}

	void play(const std::vector<PlayerMove>& moves);


	template<typename Iterator>
	void play(Iterator begin, Iterator end)
	{
		boost::unordered_set<int> ok;
		std::vector<hsfcLegalMove> lms;
		BOOST_ASSERT_MSG(!(this->isTerminal()), "Test for non-terminal state before calling play()");
		while (begin != end)
		{
			BOOST_ASSERT_MSG(begin->first.roleid_ == begin->second.move_.RoleIndex, "Player and Move do not match");
			lms.push_back(begin->second.move_);
			ok.insert(begin->first.roleid_);
			++begin;
		}
		BOOST_ASSERT_MSG(ok.size() == game_.numPlayers(), "Must be exactly one move per player");
		game_.manager_.DoMove(state_, lms);	   		
	}  
};

};


#endif // HSFC_H
