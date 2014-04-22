#include <iterator>
#include <sstream>
#include <boost/functional/hash.hpp>
#include <hsfc/hsfc.h>
#include <hsfc/portable.h>

namespace HSFC
{

/*****************************************************************************************
 * PortableState
 *****************************************************************************************/

PortableState::PortableState() : round_(0), currentstep_(0)
{ }


template<>
PortableState::PortableState<const State>(const State& state)
{
    state.manager_->GetStateData(*state.state_, relationlist_, round_, currentstep_);
}

template<>
PortableState::PortableState<State>(State& state)
{
    state.manager_->GetStateData(*state.state_, relationlist_, round_, currentstep_);
}

template<>
PortableState::PortableState<const PortableState>(const PortableState& other) :
round_(other.round_), currentstep_(other.currentstep_),
relationlist_(other.relationlist_)
{ }

template<>
PortableState::PortableState<PortableState>(PortableState& other) :
round_(other.round_), currentstep_(other.currentstep_),
relationlist_(other.relationlist_)
{ }

// Actually have to instantiate the objects.
template PortableState::PortableState<const State>(const State&);
template PortableState::PortableState<State>(State&);
template PortableState::PortableState<const PortableState>(const PortableState&);
template PortableState::PortableState<PortableState>(PortableState&);



PortableState& PortableState::operator=(const PortableState& other)
{
    round_ = other.round_;
    currentstep_ = other.currentstep_;
    relationlist_ = other.relationlist_;
    return *this;
}


bool PortableState::operator==(const PortableState& other) const
{
    return (round_ == other.round_ && currentstep_ == other.currentstep_ &&
            relationlist_ == other.relationlist_);
}

bool PortableState::operator!=(const PortableState& other) const
{
    return (round_ != other.round_ || currentstep_ != other.currentstep_ ||
            relationlist_ != other.relationlist_);
}

bool PortableState::operator<(const PortableState& other) const
{
    if (round_ != other.round_) return round_ < other.round_;
    if (currentstep_ != other.currentstep_) return currentstep_ < other.currentstep_;
    return relationlist_ < other.relationlist_;
}


std::size_t PortableState::hash_value() const
{
    size_t seed = 0;
    boost::hash_combine(seed, round_);
    boost::hash_combine(seed, currentstep_);
    boost::hash_combine(seed, relationlist_);
    return seed;
}

std::size_t hash_value(const PortableState& ps)
{
    return ps.hash_value();
}

/*****************************************************************************************
 * PortablePlayer
 *****************************************************************************************/
PortablePlayer::PortablePlayer() : roleid_(0)
{ }

PortablePlayer::PortablePlayer(const Player& player) : roleid_(player.roleid_)
{ }

PortablePlayer::PortablePlayer(const PortablePlayer& other) : roleid_(other.roleid_)
{ }

PortablePlayer& PortablePlayer::operator=(const PortablePlayer& other)
{
    roleid_ == other.roleid_;
    return *this;
}

bool PortablePlayer::operator==(const PortablePlayer& other) const
{
    return (roleid_ == other.roleid_);
}

bool PortablePlayer::operator!=(const PortablePlayer& other) const
{
    return (roleid_ != other.roleid_);
}

bool PortablePlayer::operator<(const PortablePlayer& other) const
{
    return (roleid_ < other.roleid_);
}

std::size_t PortablePlayer::hash_value() const
{
    size_t seed = 0;
    boost::hash_combine(seed, roleid_);
    return seed;
}

std::size_t hash_value(const PortablePlayer& pp)
{
    return pp.hash_value();
}

/*****************************************************************************************
 * PortableMove
 *****************************************************************************************/
PortableMove::PortableMove() : RoleIndex_(0), RelationIndex_(0), ID_(0)
{ }

PortableMove::PortableMove(const Move& move) : 
    RoleIndex_(move.move_.RoleIndex), Text_(move.move_.Text), 
    RelationIndex_(move.move_.Tuple.RelationIndex), ID_(move.move_.Tuple.ID)
{ }

PortableMove::PortableMove(const PortableMove& other) : 
    RoleIndex_(other.RoleIndex_), Text_(other.Text_),
    RelationIndex_(other.RelationIndex_), ID_(other.ID_)
{ }

PortableMove& PortableMove::operator=(const PortableMove& other)
{
    RoleIndex_ = other.RoleIndex_;
    RelationIndex_ = other.RelationIndex_;
    ID_ = other.ID_;
    Text_ = other.Text_;
    return *this;
}

bool PortableMove::operator==(const PortableMove& other) const
{
    return (RoleIndex_ == other.RoleIndex_ && RelationIndex_ == other.RelationIndex_ &&
            ID_ == other.ID_ && Text_ == other.Text_);
}

bool PortableMove::operator!=(const PortableMove& other) const
{
    return (RoleIndex_ != other.RoleIndex_ || RelationIndex_ != other.RelationIndex_ ||
            ID_ != other.ID_ || Text_ != other.Text_);
}

bool PortableMove::operator<(const PortableMove& other) const
{
    if (RoleIndex_ != other.RoleIndex_) return RoleIndex_ < other.RoleIndex_;
    if (RelationIndex_ != other.RelationIndex_) return RelationIndex_ < other.RelationIndex_;
    if (ID_ != other.ID_) return ID_ < other.ID_;
    return Text_ < other.Text_;
}

std::size_t PortableMove::hash_value() const
{
    size_t seed = 0;
    boost::hash_combine(seed, RoleIndex_);
    boost::hash_combine(seed, RelationIndex_);
    boost::hash_combine(seed, ID_);
    boost::hash_combine(seed, Text_);   
    return seed;
}

std::size_t hash_value(const PortableMove& pm)
{
    return pm.hash_value();
}

}; /* namespace HSFC */

