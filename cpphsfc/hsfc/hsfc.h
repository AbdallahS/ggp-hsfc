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
#include <boost/unordered_map.hpp>

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
    Player(Game& game, const PortablePlayer& pp);
    Player& operator=(const Player& other);

    bool operator==(const Player& other) const;
    bool operator!=(const Player& other) const;
    bool operator<(const Player& other) const;
    bool operator>(const Player& other) const;

    std::string tostring() const;
    std::size_t hash_value() const;

private:
    friend class State;
    friend class Game;
    friend class PortablePlayer;
    friend std::ostream& operator<<(std::ostream& os, const Player& player);

    boost::shared_ptr<const HSFCManager> manager_;
    unsigned int roleid_;
    Player(boost::shared_ptr<const HSFCManager> manager, unsigned int roleid);
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
    Move(Game& game, const PortableMove& pm);
    Move& operator=(const Move& other);

    bool operator==(const Move& other) const;
    bool operator!=(const Move& other) const;

    std::string tostring() const;
    std::size_t hash_value() const;

private:
    friend class State;
    friend class PortableMove;
    friend std::ostream& operator<<(std::ostream& os, const Move& move);

    boost::shared_ptr<HSFCManager> manager_;
    hsfcLegalMove move_;
    Move(boost::shared_ptr<HSFCManager> manager, const hsfcLegalMove& move);
};

std::size_t hash_value(const Move& move); /* can be a key in boost::unordered_*  */
std::ostream& operator<<(std::ostream& os, const Move& move);

std::size_t hash_value(const JointMove& move); /* can be a key in boost::unordered_*  */
std::ostream& operator<<(std::ostream& os, const JointMove& move);

std::ostream& operator<<(std::ostream& os, const JointGoal& move);

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


    /* Get the players for the game */
    std::vector<Player> players() const;

    template<typename OutputIterator>
    void players(OutputIterator dest) const;

protected:
    void initialise(const std::string& gdldescription, bool usegadelac);
    void initialise(const boost::filesystem::path& gdlfile, bool usegadelac);

    // Default construction is only allowed by the PyGame python binding
    // because we want to allow for a different constructor. Note: in future
    // look at fixing this by not sub-classing Game for the python bindings.
    Game();

private:
    // make sure users can't copy construct.
    Game(const Game& other);

    friend class State;
    friend class Player;
    friend class Move;

    boost::shared_ptr<HSFCManager> manager_;
    boost::scoped_ptr<State> initstate_; // Useful to maintain an init state

    // FIXUP: Get Michael to fix: spelling mistake in the type
    void initInternals(hsfcGDLParamaters& params);
};


template<typename OutputIterator>
void Game::players(OutputIterator dest) const
{
    for (unsigned int i = 0; i < manager_->NumPlayers(); ++i)
    {
        *dest++=Player(manager_, i);
    }
}

/*****************************************************************************************
 * A Game State
 *****************************************************************************************/

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
     */
    template<typename OutputIterator>
    void legals(OutputIterator dest) const;

    boost::unordered_map<Player, std::vector<Move> > legals() const;
    std::vector<JointMove> joints() const;

    /*
     * Return the goals. Must be called only in terminal states.
     *
     */
    template<typename OutputIterator>
    void goals(OutputIterator dest) const;

    JointGoal goals() const;

    /*
     * Return the goals after a playout. There must be exactly one move per player.
     */
    void playout(std::vector<PlayerGoal>& dest);

    template<typename OutputIterator>
    void playout(OutputIterator dest);
    JointGoal playout();

    /*
     * Make a move.
     *
     */
    void play(const JointMove& moves);
    void play(const std::vector<PlayerMove>& moves); // deprecated!

    template<typename Iterator>
    void play(Iterator begin, Iterator end);


    /*
     * For examining the internal state
     */
    const hsfcState& internal(){ return *state_; }

private:
    friend class PortableState;

    hsfcState* state_;
    boost::shared_ptr<HSFCManager> manager_;

    friend std::ostream& operator<<(std::ostream& os, const State& state);

    void initialize();

    /*
     * Internal format to return the legals in a structure that is easy
     * to check if some move is legal.
     */
    void get_legals(boost::unordered_map<Player, boost::unordered_set<Move> >& lgls) const;
    void throw_on_illegal_move(const PlayerMove& pm,
                               boost::unordered_map<Player, boost::unordered_set<Move> >& legals) const;
};

std::ostream& operator<<(std::ostream& os, const State& state);


template<typename OutputIterator>
void State::legals(OutputIterator dest) const
{
    boost::unordered_set<int> ok;
    std::vector<hsfcLegalMove> lms;
    if (this->isTerminal())
        throw HSFCValueError() << ErrorMsgInfo("Cannot cal legals() on a terminal state");

    manager_->GetLegalMoves(*state_, lms);
    BOOST_FOREACH( hsfcLegalMove& lm, lms)
    {
        *dest++=PlayerMove(Player(manager_, lm.RoleIndex), Move(manager_, lm));
        ok.insert(lm.RoleIndex);
    }
    if (ok.size() != manager_->NumPlayers())
    {
        throw HSFCInternalError()
            << ErrorMsgInfo("HSFC internal error: missing moves for some players");
    }
}

template<typename OutputIterator>
void State::goals(OutputIterator dest) const
{
    std::vector<int> vals;
    if (!(this->isTerminal()))
        throw HSFCValueError() << ErrorMsgInfo("Cannot call goals() on a non-terminal state");
    manager_->GetGoalValues(*state_, vals);
    if (vals.size() != manager_->NumPlayers())
    {
        throw HSFCInternalError()
            << ErrorMsgInfo("HSFC internal error: no goal value for some players");
    }

    for (unsigned int i = 0; i < vals.size(); ++i)
    {
        *dest++=PlayerGoal(Player(manager_, i), (unsigned int)vals[i]);
    }
}

template<typename OutputIterator>
void State::playout(OutputIterator dest)
{
    std::vector<int> vals;
    if (this->isTerminal())
        throw HSFCValueError() << ErrorMsgInfo("Cannot playout() on a terminal state");
    manager_->PlayOut(*state_, vals);
    if (vals.size() != manager_->NumPlayers())
    {
        throw HSFCInternalError()
            << ErrorMsgInfo("HSFC internal error: no goal value for some players");
    }

    // Not sure if this is necessary, but added a test for termination
    // because it turns out that checking for termination does change
    // some internal structures in the state.
    if (!this->isTerminal())
        throw HSFCInternalError() << ErrorMsgInfo("State is not terminal after a playout()");

    for (unsigned int i = 0; i < vals.size(); ++i)
    {
        *dest++= PlayerGoal(Player(manager_,i), (unsigned int)vals[i]);
    }
}

template<typename Iterator>
void State::play(Iterator begin, Iterator end)
{
    boost::unordered_set<int> ok;
    std::vector<hsfcLegalMove> lms;

    boost::unordered_map<Player, boost::unordered_set<Move> > lglmap;
    get_legals(lglmap);
    if (this->isTerminal())
        throw HSFCValueError() << ErrorMsgInfo("Cannot play() on a terminal state");
    while (begin != end)
    {
        throw_on_illegal_move(*begin, lglmap);
        if (begin->first.roleid_ != begin->second.move_.RoleIndex)
            throw HSFCValueError() << ErrorMsgInfo("Mismatched Player and Move");
        lms.push_back(begin->second.move_);
        ok.insert(begin->first.roleid_);
        ++begin;
    }
    if (ok.size() != manager_->NumPlayers())
        throw HSFCValueError() << ErrorMsgInfo("Must be exactly one move per player");
    manager_->DoMove(*state_, lms);

    initialize();
}




}; /* namespace HSFC */


#endif // HSFC_HSFC_H
