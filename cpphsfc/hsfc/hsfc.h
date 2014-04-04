/*****************************************************************************************
 * The GDL High Speed Forward Chainer (HSFC).
 *****************************************************************************************/

#ifndef HSFC_H
#define HSFC_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <memory>
#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/exception/all.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/filesystem.hpp>

#include <hsfc/hsfcexception.h>
#include <hsfc/impl/hsfcwrapper.h>

namespace HSFC
{


/*****************************************************************************************
 * Player (role) and Move classes. Note: To clear any ambiguity Move represents the 
 * move as independent from a player, so not the move as take by a player.
 *****************************************************************************************/

class Game;

class Player
{
	friend class State;
	friend class Game;
	friend std::ostream& operator<<(std::ostream& os, const Player& player);

	const HSFCManager* manager_;
	unsigned int roleid_;
	Player(const HSFCManager* manager_, unsigned int roleid);	
public:
	Player(const Player& other);	
	std::string tostring() const;
	bool operator==(const Player& other) const;
	bool operator!=(const Player& other) const;
	Player& operator=(const Player& other);

	std::size_t hash_value() const;
};

std::size_t hash_value(const Player& player);

std::ostream& operator<<(std::ostream& os, const Player& player);

class Move
{
	friend class State;
	friend std::ostream& operator<<(std::ostream& os, const Move& move);
	
	HSFCManager* manager_;
	hsfcLegalMove move_;
	Move(HSFCManager* manager, const hsfcLegalMove& move);
public:
	Move(const Move& other);
	std::string tostring() const;
	bool operator==(const Move& other) const;
	bool operator!=(const Move& other) const;
	Move& operator=(const Move& other);

	std::size_t hash_value() const;
};
std::size_t hash_value(const Move& move);

std::ostream& operator<<(std::ostream& os, const Move& move);


/*****************************************************************************************
 * A game object - only one per loaded GDL game.
 *****************************************************************************************/

class State;

class Game
{
	friend class State;

	HSFCManager manager_;
	boost::scoped_ptr<State> initstate_; // Useful to maintain an init state

	// FIXUP: Get Michael to fix: spelling mistake in the type name
    void hsfcGDLParamsInit(hsfcGDLParamaters& params);

	Game(const Game& other);  // make sure we can't copy this object

public:
    // Game constructor that takes a string containing a GDL description
    // (note: this is not a filename!).
	explicit Game(const std::string& gdldescription, bool usegadelac = false);
	explicit Game(const char* gdldescription, bool usegadelac = false);

    // Game constructor that takes a file.
	explicit Game(const boost::filesystem::path& gdlfile, bool usegadelac = false);

	unsigned int numPlayers() const;
	// Return the initial state
	const State& initState() const;

	bool operator==(const Game& other) const;


	/*
	 * Returns the players 
	 */
	void players(std::vector<Player>& plyrs) const;

	template<typename OutputIterator>
	void players(OutputIterator dest) const
	{		
		for (unsigned int i = 0; i < manager_.NumPlayers(); ++i)
		{
			*dest++=Player(&manager_, i); 
		}
	}
    
};


/*****************************************************************************************
 * A Game State
 *****************************************************************************************/


typedef std::pair<Player,Move> PlayerMove;
typedef std::pair<Player,unsigned int> PlayerGoal;

class PortableState;

class State
{
    friend class PortableState;

	hsfcState* state_;
	HSFCManager* manager_;

public:
    State(Game& game);
    State(Game& game, const PortableState& ps);
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
		manager_->GetLegalMoves(*state_, lms);
		BOOST_FOREACH( hsfcLegalMove& lm, lms)
		{
			dest++=PlayerMove(Player(manager_, lm.RoleIndex), Move(manager_, lm));
			ok.insert(lm.RoleIndex);
		}
		if (ok.size() != manager_->NumPlayers())
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
		manager_->GetGoalValues(*state_, vals);
		if (vals.size() != manager_->NumPlayers())
		{
			throw HSFCException() << ErrorMsgInfo("HSFC internal error: no goal value for some players");
		}
		
		for (unsigned int i = 0; i < vals.size(); ++i)
		{
			dest++=PlayerGoal(Player(manager_, i), (unsigned int)vals[i]);
		}		
	}

	/* 
	 * Return the goals after a playout. There must be exactly one move per player.
	 */
	void playout(std::vector<PlayerGoal>& dest);
	
	template<typename OutputIterator>
	void playout(OutputIterator dest)
	{
		std::vector<int> vals;
		BOOST_ASSERT_MSG(!this->isTerminal(), "Test for terminal state before calling playOut()");
		manager_->PlayOut(*state_, vals);
		if (vals.size() != manager_->NumPlayers())
		{
			throw HSFCException() << ErrorMsgInfo("HSFC internal error: no goal value for some players");
		}
		
		for (unsigned int i = 0; i < vals.size(); ++i)
		{
			dest++= PlayerGoal(Player(manager_,i), (unsigned int)vals[i]);
		}
	}

	/*
	 * Make a move. 
	 */

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
		BOOST_ASSERT_MSG(ok.size() == manager_->NumPlayers(), "Must be exactly one move per player");
		manager_->DoMove(*state_, lms);	   		
	}  

	/*
	 * Convert to and from a PortableState object
	 */
	boost::shared_ptr<PortableState> CreatePortableState() const;
	void LoadPortableState(const PortableState& ps);
	
};


/*****************************************************************************************
 * The PortableState is a semi-portable representation of a state that can be serialised
 * and loaded between any HSFC instances loaded with the same GDL (we hope!!!).
 *****************************************************************************************/

class PortableState
{
	friend class State;
	friend class boost::serialization::access;

	int round_;
	int currentstep_;
	std::vector<std::pair<int,int> > relationlist_;

	template<typename Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & round_;
		ar & currentstep_;
		ar & relationlist_;
	}
	PortableState();

    // Because we want this to be immutable (from the users perspective)
    // we ensure that you cannot assign to it
    PortableState& operator=(const PortableState& other);
public:
    template<typename Archive>
    explicit PortableState(Archive& ar)
    {
        ar >> *this;
    }

	bool operator==(const PortableState& other) const;
	bool operator!=(const PortableState& other) const;

	std::size_t hash_value() const;
};

// Explicit specialisations of the constructor so that it is
// not captured by the constructor intended for archive objects.
template<> PortableState::PortableState<State>(State& state);
template<> PortableState::PortableState<const State>(const State& state);
template<> PortableState::PortableState<PortableState>(PortableState& other);
template<> PortableState::PortableState<const PortableState>(const PortableState& other);


std::size_t hash_value(const PortableState& move);


}; /* namespace HSFC */


#endif // HSFC_H
