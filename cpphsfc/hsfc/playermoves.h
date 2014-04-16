/*****************************************************************************************
 * 
 * A Collection of PlayerMoves.
 * This is a convenience class that makes accessing PlayerMove instances easier.
 *
 * The State::legals() functions returns a collection of PlayerMove objects to
 * to represent all the legal moves that are possible for players in a given
 * state. However, there are many different ways you may want to examine 
 * these legal moves:
 *
 * - Simply treat it as a list of PlayerMove objects to iterate over.
 *
 * - Iterate over just the players.
 *
 * - Iterate over the moves for a given player.
 *
 * - Generate all valid joint moves available from this state (eg., for UCT).
 *
 * This class allows you to do all these operations on the same collection of object by 
 * offering different (read-only) views over the data. 
 * 
 * Note: currently, there is no way to remove individual elements (except a global clear). 
 *       May add this in the future if there is a need.
 *
 *****************************************************************************************/
#ifndef HSFC_PLAYERMOVES_H
#define HSFC_PLAYERMOVES_H

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <functional>
#include <iterator>
#include <hsfc/hsfc.h>

namespace HSFC
{

/***************************************************************************************
 * The main PlayerMoves clsas
 ***************************************************************************************/


class PlayerMoves
{
public:
    // The basic/default is to treat the collection as an (unordered) 
    // list of PlayerMove objects.

    typedef std::size_t size_type;

    // Note: iterator and const_iterator are the same because we don't want
    // to allow a value to be modified directly from the iterator as it can
    // break things. This is similar to std::set (at least as of C++11).
    typedef boost::unordered_multimap<Player,Move>::const_iterator const_iterator;
    typedef const_iterator iterator;

    // The reference types are needed by std::inserter
    typedef PlayerMove& reference;
    typedef const PlayerMove& const_reference;

    // Standard STL-like functions
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    // Insert into collection. Note: hint iterator version is needed for std::inserter.
    iterator insert(const PlayerMove& pm);   
    iterator insert(const_iterator posn, const PlayerMove& pm);   

    void clear();
    bool empty() const;
    size_type size() const;

    template <typename Iterator>
    PlayerMoves(Iterator first, Iterator last);

    PlayerMoves();
    PlayerMoves(const PlayerMoves& other);
    PlayerMoves& operator=(const PlayerMoves& other);

    // A Player view to iterate over the players. 
    // Note: there is only a singleton object for this class. It is designed to be accessed
    // directly by the user, hence uses a different naming convention.
    class ViewPlayers
    {
    public:
        typedef boost::unordered_set<Player>::const_iterator const_iterator;
        typedef const_iterator iterator;
        typedef PlayerMoves::size_type size_type;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        iterator find(Player& p);
        const_iterator find(Player& p) const;
                
        bool empty() const;
        size_type size() const;
    private:
        friend class PlayerMoves;
        const PlayerMoves* pms_;
        ViewPlayers(PlayerMoves* pms);
    } viewPlayers;


    // Support functional objects for the boost::transform_iterator
    struct PlayerMoveToMove : public std::unary_function<const PlayerMove&, const Move>
    {
        inline const Move operator()(const PlayerMove &pm) const { return pm.second; }
    };

    //  A view of just the moves of a single player
    class ViewMovesByPlayer
    {
    public:
        typedef boost::transform_iterator<PlayerMoveToMove, 
                                          PlayerMoves::const_iterator> const_iterator;
        typedef const_iterator iterator;
        typedef PlayerMoves::size_type size_type;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        bool empty() const;
        size_type size() const;
    private:
        friend class PlayerMoves;
        
        PlayerMoves::const_iterator begin_;
        PlayerMoves::const_iterator end_;
        ViewMovesByPlayer(PlayerMoves& pms, const Player& p);
    };

    ViewMovesByPlayer viewMovesByPlayer(const Player& p);

    /*

    // A View over the joint moves
    class JointMove
    {
    public:
        typedef PlayerMoves::const_iterator const_iterator;
        typedef const_iterator iterator;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;        

    private:
        JointMove();
    };
    class ViewJointMoves
    {
        
        
    public:
        
        
    };
    */

private:

    typedef boost::unordered_multimap<Player,Move> playermoves_t;
    typedef boost::unordered_set<Player> player_t;

    boost::unordered_multimap<Player,Move> playermoves_;
    boost::unordered_set<Player> players_;

};

/***************************************************************************************
 * PlayerMove template functions and inline functions
 ***************************************************************************************/

template <typename Iterator>
PlayerMoves::PlayerMoves(Iterator first, Iterator last) : viewPlayers(this)
{
    while (first != last)
    {
        playermoves_.insert(*first);
        players_.insert(first->first);
        ++first;
    }
}

inline PlayerMoves::ViewMovesByPlayer PlayerMoves::viewMovesByPlayer(const Player& p)
{
    return PlayerMoves::ViewMovesByPlayer(*this, p);
}


}; /* namespace HSFC */

#endif /* HSFC_PLAYERMOVES_H */
