/*****************************************************************************************
 *
 * Portable classes for the HSFC
 * These are (semi-)portable representations of a state that can be serialised
 * and loaded between any HSFC instances loaded with the same GDL (we hope!!!).
 *
 *****************************************************************************************/

#ifndef HSFC_PORTABLE_H
#define HSFC_PORTABLE_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <boost/unordered_set.hpp>
#include <boost/assert.hpp>
#include <boost/exception/all.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/access.hpp>

#include <hsfc/hsfcexception.h>
#include <hsfc/impl/fwd_decl.h>


namespace HSFC
{

/*****************************************************************************************
 * PortableState.
 *****************************************************************************************/

class PortableState
{
public:
    PortableState();
    PortableState& operator=(const PortableState& other);

/* FIXUP: Deprecated constructor - to be removed */
    template<typename Archive>
    explicit PortableState(Archive& ar);

    bool operator==(const PortableState& other) const;
    bool operator!=(const PortableState& other) const;

    std::size_t hash_value() const;

private:
    friend class State;
    friend class boost::serialization::access;

    int round_;
    int currentstep_;
    std::vector<std::pair<int,int> > relationlist_;

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version);

};

std::size_t hash_value(const PortableState& ps); /* Can be used as a key in boost::unordered_* */


/* FIXUP: Deprecated constructor - to be removed */
template<typename Archive>
PortableState::PortableState(Archive& ar)
{
    ar >> *this;
}

// Explicit specialisations of the constructor so that it is
// not captured by the constructor intended for archive objects.
template<> PortableState::PortableState<State>(State& state);
template<> PortableState::PortableState<const State>(const State& state);
template<> PortableState::PortableState<PortableState>(PortableState& other);
template<> PortableState::PortableState<const PortableState>(const PortableState& other);


template<typename Archive>
void PortableState::serialize(Archive& ar, const unsigned int version)
{
    ar & round_;
    ar & currentstep_;
    ar & relationlist_;
}


/*****************************************************************************************
 * PortablePlayer
 *****************************************************************************************/

class PortablePlayer
{
public:
    PortablePlayer(const Player& player);
    PortablePlayer(const PortablePlayer& other);
    PortablePlayer& operator=(const PortablePlayer& other);
    
    bool operator==(const PortablePlayer& other) const;
    bool operator!=(const PortablePlayer& other) const;

    std::size_t hash_value() const;

private:
    friend class Player;
    friend class PortablePlayerMove;
    friend class PortablePlayerGoal;
    friend class boost::serialization::access;

    PortablePlayer();
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version);

    unsigned int roleid_;
};

template<typename Archive>
void PortablePlayer::serialize(Archive& ar, const unsigned int version)
{
    ar & roleid_;
}

std::size_t hash_value(const PortablePlayer& pp); /* Can be used as a key in boost::unordered_* */

/*****************************************************************************************
 * PortableMove
 *****************************************************************************************/

class PortableMove
{
public:
    PortableMove(const Move& move);
    PortableMove(const PortableMove& other);
    PortableMove& operator=(const PortableMove& other);
    
    bool operator==(const PortableMove& other) const;
    bool operator!=(const PortableMove& other) const;

    std::size_t hash_value() const;

private:
    friend class Move;
    friend class PortablePlayerMove;
    friend class boost::serialization::access;

    PortableMove();
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version);

    unsigned int RoleIndex_;
    std::string Text_;
    int RelationIndex_;
    int ID_;
};

template<typename Archive>
void PortableMove::serialize(Archive& ar, const unsigned int version)
{
    ar & RoleIndex_;
    ar & Text_;
    ar & RelationIndex_;
    ar & ID_;
}

std::size_t hash_value(const PortableMove& pm); /* Can be used as a key in boost::unordered_* */


/*****************************************************************************************
 * PortablePlayerMove
 *****************************************************************************************/

//typedef std::pair<PortablePlayer, PortableMove> PortablePlayerMove;


class PortablePlayerMove : public std::pair<PortablePlayer, PortableMove>
{
public:
    PortablePlayerMove(const PortablePlayer& pp, const PortableMove& pm);
    PortablePlayerMove(const std::pair<PortablePlayer, PortableMove>& other);
    PortablePlayerMove(const PlayerMove& pm);
    operator std::pair<PortablePlayer, PortableMove>&();

private:
    friend class boost::serialization::access;
    PortablePlayerMove();

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version);    
};

template<typename Archive>
void PortablePlayerMove::serialize(Archive& ar, const unsigned int version)
{
    ar & first;
    ar & second;
}


/*****************************************************************************************
 * PortablePlayerGoal
 *****************************************************************************************/

//typedef std::pair<PortablePlayer, unsigned int> PortablePlayerGoal;


class PortablePlayerGoal : public std::pair<PortablePlayer, unsigned int>
{
public:
    PortablePlayerGoal(const PortablePlayer& pp, unsigned int g);
    PortablePlayerGoal(const std::pair<PortablePlayer, unsigned int>& other);
    PortablePlayerGoal(const PlayerGoal& pg);
    operator std::pair<PortablePlayer, unsigned int>&();

private:
    friend class boost::serialization::access;
    PortablePlayerGoal();

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version);    
};

template<typename Archive>
void PortablePlayerGoal::serialize(Archive& ar, const unsigned int version)
{
    ar & first;
    ar & second;
}



/*****************************************************************************************
 * Support function to convert to/from collection of PortableX's.
 * E.g, Given: 
 *      std::vector<PortableState> input, Game& game, std::vector<State> output
 *      where input is a vector of PortableStates then:
 *
 *        convert_from_portable(input, game, output)
 *
 *      will populate the output vector with the corresponsing states.
 *****************************************************************************************/

template<typename Cin, typename Cout>
void convert_to_portable(const Cin& in, Cout& out)
{
    BOOST_FOREACH(const typename Cin::value_type& np, in)
    {
        out.push_back(typename Cout::value_type(np));
    }
}

template<typename Cin, typename Cout>
void convert_from_portable(const Cin& in, Game& game, Cout& out)
{
    BOOST_FOREACH(const typename Cin::value_type& p, in)
    {
        out.push_back(typename Cout::value_type(game, p));
    }
}

/*****************************************************************************************
 * Support functor to convert from collection of PortableX's. Here is an example
 * of how to use it:
 *
 *     Game game(<some_game>);
 *     std::vector<PortablePlayerMove> ppmoves;
 * 
 *     ....   // Assign to ppmoves;
 *
 *     std::vector<PlayerMove> pmoves;
 *     std::transform(ppmoves.begin(), ppmoves.end(), 
 *                    std::inserter(pmoves, pmoves.begin()), FromPortable(game));
 *
 * NOTE: there is no ToPortable because it is unnecessary. To copy from PortableX to X
 *       you can simply use std::copy. The constructors will take care of any type 
 *       conversions. For example:
 * 
 *     std::copy(pmoves.begin(), pmoves.end(), std::inserter(ppmoves, ppmoves.begin()));
 *
 *****************************************************************************************/

struct FromPortable
{
public:
    FromPortable(Game& game);
    
    PlayerMove operator()(const PortablePlayerMove& ppm);
    PlayerGoal operator()(const PortablePlayerGoal& ppg);
    Player operator()(const PortablePlayer& pp);
    Move operator()(const PortableMove& pm);
    State operator()(const PortableState& ps);
    
private:
    Game& game_;
};

/*****************************************************************************************
 * Inlined implementation of FromPortable.
 *****************************************************************************************/

inline FromPortable::FromPortable(Game& game) : game_(game) { }

inline PlayerMove FromPortable::operator()(const PortablePlayerMove& ppm)
{ return PlayerMove(Player(game_, ppm.first), Move(game_,ppm.second)); }

inline PlayerGoal FromPortable::operator()(const PortablePlayerGoal& ppg)
{ return PlayerGoal(Player(game_, ppg.first), ppg.second); }

inline Player FromPortable::operator()(const PortablePlayer& pp)
{ return Player(game_, pp); }

inline Move FromPortable::operator()(const PortableMove& pm)
{ return Move(game_, pm); }

inline State FromPortable::operator()(const PortableState& ps)
{ return State(game_, ps); }

}; /* namespace HSFC */


#endif // HSFC_PORTABLE_H
