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
#include <set>
#include <memory>
#include <iterator>
#include <algorithm>
#include <stdint.h>
#include <boost/assert.hpp>
#include <boost/exception/all.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
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
    PortableState(const State& state);
    PortableState(const PortableState& other);
    PortableState& operator=(const PortableState& other);

    bool operator==(const PortableState& other) const;
    bool operator!=(const PortableState& other) const;
    bool operator<(const PortableState& other) const;

    std::size_t hash_value() const;

private:
    friend class State;
    friend class boost::serialization::access;

    int round_;
    int currentstep_;
    std::set<std::pair<int,int> > relationset_;
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version);

};

std::size_t hash_value(const PortableState& ps); /* Can be used as a key in boost::unordered_* */

template<typename Archive>
void PortableState::serialize(Archive& ar, const unsigned int version)
{
    ar & round_;
    ar & currentstep_;
    ar & relationset_;
}


/*****************************************************************************************
 * PortablePlayer
 *****************************************************************************************/

class PortablePlayer
{
public:
    PortablePlayer();
    PortablePlayer(const Player& player);
    PortablePlayer(const PortablePlayer& other);
    PortablePlayer& operator=(const PortablePlayer& other);

    bool operator==(const PortablePlayer& other) const;
    bool operator!=(const PortablePlayer& other) const;
    bool operator<(const PortablePlayer& other) const;

    std::size_t hash_value() const;

private:
    friend class Player;
    friend class boost::serialization::access;

/* PortablePlayer default constructor is now public!
    // NOTE: 1) Need to make PortablePlayerMove and PortablePlayerGoal friends
    //          to allow the deserialization of these objects to work.
    //          This is ugly and does in theory allow empty Portable objects to
    //          be created by a user (by way of the friend pair).
    //       2) cannot be a friend of a typedef. I think this has changed in C++11.
    friend class std::pair<PortablePlayer, unsigned int>;
    friend class std::pair<const PortablePlayer, unsigned int>;
    friend class std::pair<PortablePlayer, uint64_t>;
    friend class std::pair<const PortablePlayer, uint64_t>;

    friend class std::pair<PortablePlayer, PortableMove>;
    friend class std::pair<const PortablePlayer, PortableMove>;
    friend class std::pair<PortablePlayer, const PortableMove>;
    friend class std::pair<const PortablePlayer, const PortableMove>;
*/
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
    PortableMove();
    PortableMove(const Move& move);
    PortableMove(const PortableMove& other);
    PortableMove& operator=(const PortableMove& other);

    bool operator==(const PortableMove& other) const;
    bool operator!=(const PortableMove& other) const;
    bool operator<(const PortableMove& other) const;

    std::size_t hash_value() const;

private:
    friend class Move;
    friend class boost::serialization::access;

/*
  PortableMove default constructor is now public!
    // NOTE: 1) Need to make PortablePlayerMove a friend to allow the deserialization
    //          of these objects to work. This is ugly and does in theory allow empty
    //          Portable objects to be created by a user (by way of the friend pair).
    //       2) cannot be a friend of a typedef. I think this has changed in C++11.
    friend class std::pair<PortablePlayer, PortableMove>;
    friend class std::pair<const PortablePlayer, PortableMove>;
    friend class std::pair<PortablePlayer, const PortableMove>;
    friend class std::pair<const PortablePlayer, const PortableMove>;
*/

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version);

    int RoleIndex_;
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
 * Support functor to convert to/from collection of PortableX's. Here is an example
 * of how to use it:
 *
 *     Game game(<some_game>);
 *     std::vector<PortablePlayerMove> ppmoves;
 *
 *     ....   // Assign to ppmoves;
 *
 *     std::vector<PlayerMove> pmoves;
 *     std::transform(ppmoves.begin(), ppmoves.end(),
 *                    std::back_inserter(pmoves), FromPortable(game));
 *
 * To convert in the opposite direction use ToPortable(). This is only strictly necessary
 * necessary for PortableJointMove and PortableJointGoal conversion, since in the other
 * cases the std::copy could be used. However, these other functions have been defined for
 * conversion to all portable types for simplicity/completeness.
 *
 *     std::transform(pmoves.begin(), pmoves.end(), std::back_inserter(ppmoves), ToPortable());
 *
 * NOTE: I think the value_type for PortableJointMove is:
 *       std::pair<const PortablePlayer,PortableMove> This seems to be making conversion for
 *       PortablePlayerMove (which is std::pair<PortablePlayer,PortableMove>) ambiguous. So
 *      explicitly add conversions for this. Doing the same for PortableJointGoal as well.
 *
 *****************************************************************************************/

struct FromPortable
{
public:
    FromPortable(Game& game);

    JointMove operator()(const PortableJointMove& pjm);
    JointGoal operator()(const PortableJointGoal& pjg);
    PlayerMove operator()(const PortablePlayerMove& ppm);
    PlayerMove operator()(const std::pair<const PortablePlayer, PortableMove>& ppm);
    PlayerGoal operator()(const PortablePlayerGoal& ppg);
    PlayerGoal operator()(const std::pair<const PortablePlayer, unsigned int>& ppg);
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

inline JointMove FromPortable::operator()(const PortableJointMove& pjm)
{
    JointMove jm;
    std::transform(pjm.begin(), pjm.end(), std::inserter(jm,jm.begin()),*this);
    return jm;
}

inline JointGoal FromPortable::operator()(const PortableJointGoal& pjg)
{
    JointGoal jg;
    std::transform(pjg.begin(), pjg.end(), std::inserter(jg,jg.begin()),*this);
    return jg;
}

inline PlayerMove FromPortable::operator()(const PortablePlayerMove& ppm)
{ return PlayerMove(Player(game_, ppm.first), Move(game_,ppm.second)); }

inline PlayerMove FromPortable::operator()(const std::pair<const PortablePlayer, PortableMove>& ppm)
{ return PlayerMove(Player(game_, ppm.first), Move(game_,ppm.second)); }

inline PlayerGoal FromPortable::operator()(const PortablePlayerGoal& ppg)
{ return PlayerGoal(Player(game_, ppg.first), ppg.second); }

inline PlayerGoal FromPortable::operator()(const std::pair<const PortablePlayer, unsigned int>& ppg)
{ return PlayerGoal(Player(game_, ppg.first), ppg.second); }

inline Player FromPortable::operator()(const PortablePlayer& pp)
{ return Player(game_, pp); }

inline Move FromPortable::operator()(const PortableMove& pm)
{ return Move(game_, pm); }

inline State FromPortable::operator()(const PortableState& ps)
{ return State(game_, ps); }


/*****************************************************************************************
 * ToPortable. See explanation of FromPortable.
 *****************************************************************************************/

struct ToPortable
{
public:
    ToPortable();
    ToPortable(Game& game);

    PortableJointMove operator()(const JointMove& jm);
    PortableJointGoal operator()(const JointGoal& jg);
    PortablePlayerMove operator()(const PlayerMove& ppm);
    PortablePlayerGoal operator()(const PlayerGoal& ppg);
    PortablePlayer operator()(const Player& pp);
    PortableMove operator()(const Move& pm);
    PortableState operator()(const State& ps);
};

/*****************************************************************************************
 * inlined implementation of ToPortable.
 *****************************************************************************************/

inline ToPortable::ToPortable(){ }

inline ToPortable::ToPortable(Game& game){ }

inline PortableJointMove ToPortable::operator()(const JointMove& jm)
{
    PortableJointMove pjm;
    std::copy(jm.begin(), jm.end(), std::inserter(pjm,pjm.begin()));
    return pjm;
}

inline PortableJointGoal ToPortable::operator()(const JointGoal& jg)
{
    PortableJointGoal pjg;
    std::copy(jg.begin(), jg.end(), std::inserter(pjg,pjg.begin()));
    return pjg;
}

inline PortablePlayerMove ToPortable::operator()(const PlayerMove& pm)
{ return PortablePlayerMove(pm); }

inline PortablePlayerGoal ToPortable::operator()(const PlayerGoal& pg)
{ return PortablePlayerGoal(pg); }

inline PortablePlayer ToPortable::operator()(const Player& p)
{ return PortablePlayer(p); }

inline PortableMove ToPortable::operator()(const Move& m)
{ return PortableMove(m); }

inline PortableState ToPortable::operator()(const State& s)
{ return PortableState(s); }


}; /* namespace HSFC */


#endif // HSFC_PORTABLE_H
