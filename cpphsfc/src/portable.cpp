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


std::size_t PortableState::hash_value() const
{
    size_t seed = 0;
    boost::hash_combine(seed, round_);
    boost::hash_combine(seed, currentstep_);
    boost::hash_combine(seed, relationlist_);
    return seed;
}

std::size_t hash_value(const PortableState& state)
{
    return state.hash_value();
}


}; /* namespace HSFC */

