/*****************************************************************************************
 * The GDL High Speed Forward Chainer (HSFC).
 *****************************************************************************************/

#ifndef HSFC_HSFC_H
#define HSFC_HSFC_H

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
#include <boost/filesystem.hpp>

#include <hsfc/hsfcexception.h>
#include <hsfc/impl/fwd_decl.h>
#include <hsfc/impl/hsfcwrapper.h>

namespace HSFC
{

/*****************************************************************************************
 * Player represents a GDL role.
 *****************************************************************************************/

class Player
{
public:
    Player(const Player& other);  
    std::string tostring() const;
    bool operator==(const Player& other) const;
    bool operator!=(const Player& other) const;
    Player& operator=(const Player& other);

    std::size_t hash_value() const;

    friend class State;
    friend class Game;
    friend std::ostream& operator<<(std::ostream& os, const Player& player);

private:
    const HSFCManager* manager_;
    unsigned int roleid_;
    Player(const HSFCManager* manager_, unsigned int roleid);  
};

std::size_t hash_value(const Player& player); /* can be a key in boost::unordered_*  */
std::ostream& operator<<(std::ostream& os, const Player& player);

/*****************************************************************************************
 * Move class. 
 * Note: To clear any ambiguity Move represents the move as independent from a player,
 * and NOT the move as taken by a player.
 *****************************************************************************************/

class Move
{
public:
    Move(const Move& other);
    std::string tostring() const;
    bool operator==(const Move& other) const;
    bool operator!=(const Move& other) const;
    Move& operator=(const Move& other);

    std::size_t hash_value() const;

private:

    friend class State;
    friend std::ostream& operator<<(std::ostream& os, const Move& move);
  
    HSFCManager* manager_;
    hsfcLegalMove move_;
    Move(HSFCManager* manager, const hsfcLegalMove& move);
};

std::size_t hash_value(const Move& move); /* can be a key in boost::unordered_*  */
std::ostream& operator<<(std::ostream& os, const Move& move);

/*****************************************************************************************
 * A game object - only one per loaded GDL game.
 *****************************************************************************************/


class Game
{
public:
    // Game constructor that takes a string containing a GDL description
    // NOTE: 1) this is not a filename!.
    //       2) Probably not necessary, but I provide a char* version 
    //          to make sure a char* can't accidentally be converted 
    //          to a boost::filesystem::path.
    Game(const std::string& gdldescription, bool usegadelac = false);
    Game(const char* gdldescription, bool usegadelac = false);

    // Game constructor that takes a file.
    Game(const boost::filesystem::path& gdlfile, bool usegadelac = false);

    unsigned int numPlayers() const;
    // Return the initial state
    const State& initState() const;

    bool operator==(const Game& other) const;
    bool operator!=(const Game& other) const;


    /*
     * Returns the players 
     */
    void players(std::vector<Player>& plyrs) const;
    boost::shared_ptr<std::vector<Player> > players() const;

    template<typename OutputIterator>
    void players(OutputIterator dest) const;

protected:
    void initialise(const std::string& gdldescription, bool usegadelac);
    void initialise(const boost::filesystem::path& gdlfile, bool usegadelac);

    // make sure users can't copy or default construct.
    Game(const Game& other);  
    Game();
    
private:
    friend class State;

    HSFCManager manager_;
    boost::scoped_ptr<State> initstate_; // Useful to maintain an init state

    // FIXUP: Get Michael to fix: spelling mistake in the type
    void initInternals(hsfcGDLParamaters& params);
};


template<typename OutputIterator>
void Game::players(OutputIterator dest) const
{    
    for (unsigned int i = 0; i < manager_.NumPlayers(); ++i)
    {
        *dest++=Player(&manager_, i); 
    }
}


/*****************************************************************************************
 * A Game State
 *****************************************************************************************/


typedef std::pair<Player,Move> PlayerMove;
typedef std::pair<Player,unsigned int> PlayerGoal;

class State
{
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
     * TODO: boost::shared_ptr<PlayerMove> legals() and same for goal()

     */
    void legals(std::vector<PlayerMove>& moves) const;
    boost::shared_ptr<std::vector<PlayerMove> > legals() const;

    template<typename OutputIterator>
    void legals(OutputIterator dest) const;

    /* 
     * Return the goals. Must be called only in terminal states.
     *
     */
    void goals(std::vector<PlayerGoal>& results) const;
    boost::shared_ptr<std::vector<PlayerGoal> > goals() const;

    template<typename OutputIterator>
    void goals(OutputIterator dest) const;

    /* 
     * Return the goals after a playout. There must be exactly one move per player.
     */
    void playout(std::vector<PlayerGoal>& dest);

    template<typename OutputIterator>
    void playout(OutputIterator dest);
  
    /*
     * Make a move. 
     *
     */
    void play(const std::vector<PlayerMove>& moves);

    template<typename Iterator>
    void play(Iterator begin, Iterator end);

    /* FIXUP: Deprecated - to be removed */
    /*
     * Convert to and from a PortableState object
     */
    boost::shared_ptr<PortableState> CreatePortableState() const;
    void LoadPortableState(const PortableState& ps);

private:
    friend class PortableState;

    hsfcState* state_;
    HSFCManager* manager_;  
};


template<typename OutputIterator>
void State::legals(OutputIterator dest) const
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

template<typename OutputIterator>
void State::goals(OutputIterator dest) const
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

template<typename OutputIterator>
void State::playout(OutputIterator dest)
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

template<typename Iterator>
void State::play(Iterator begin, Iterator end)
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


}; /* namespace HSFC */


#endif // HSFC_HSFC_H
