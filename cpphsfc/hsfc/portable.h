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
    friend class boost::serialization::access;
    
    // NOTE: 1) Need to make PortablePlayerMove and PortablePlayerGoal friends
    //          to allow the deserialization of these objects to work.
    //          This is ugly and does in theory allow empty Portable objects to 
    //          be created by a user (by way of the friend pair).
    //       2) cannot be a friend of a typedef. I think this has changed in C++11.
    friend class std::pair<PortablePlayer,unsigned int>;
    friend class std::pair<PortablePlayer,PortableMove>;

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
    friend class boost::serialization::access;

    // NOTE: 1) Need to make PortablePlayerMove a friend to allow the deserialization 
    //          of these objects to work. This is ugly and does in theory allow empty
    //          Portable objects to be created by a user (by way of the friend pair).
    //       2) cannot be a friend of a typedef. I think this has changed in C++11.
    friend class std::pair<PortablePlayer,PortableMove>;

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
