/*****************************************************************************************
 * Some useful forward declaration of HSFC API classes.
 *****************************************************************************************/
#ifndef HSFC_FWD_DECL_H
#define HSFC_FWD_DECL_H

#include <utility>
#include <map>
#include <boost/unordered_map.hpp>

namespace HSFC
{

class Game;

class State;
class Move;
class Player;

typedef std::pair<Player,Move> PlayerMove;
typedef std::pair<Player,unsigned int> PlayerGoal;

typedef boost::unordered_map<Player, Move> JointMove;
typedef boost::unordered_map<Player, unsigned int> JointGoal;

class PortableState;
class PortablePlayer;
class PortableMove;

typedef std::pair<PortablePlayer, PortableMove> PortablePlayerMove;
typedef std::pair<PortablePlayer, unsigned int> PortablePlayerGoal;

typedef std::map<PortablePlayer, PortableMove> PortableJointMove;
typedef std::map<PortablePlayer, unsigned int> PortableJointGoal;


};


#endif /* HSFC_FWD_DECL_H */
