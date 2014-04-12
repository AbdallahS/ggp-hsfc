/*****************************************************************************************
 * Portable classes for the HSFC
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
 * The PortableState is a semi-portable representation of a state that can be serialised
 * and loaded between any HSFC instances loaded with the same GDL (we hope!!!).
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

std::size_t hash_value(const PortableState& move); /* Can be used as a key in boost::unordered_* */


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




}; /* namespace HSFC */


#endif // HSFC_PORTABLE_H
