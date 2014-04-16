/*****************************************************************************************
 * Some useful forward declaration of HSFC API classes.
 *****************************************************************************************/

#ifndef HSFC_FWD_DECL_H
#define HSFC_FWD_DECL_H

namespace HSFC
{

class Game;

class State;
class Move;
class Player;

typedef std::pair<Player,Move> PlayerMove;
typedef std::pair<Player,unsigned int> PlayerGoal;

class PortableState;
class PortablePlayer;
class PortableMove;

typedef std::pair<PortablePlayer, PortableMove> PortablePlayerMove;
typedef std::pair<PortablePlayer, unsigned int> PortablePlayerGoal;

};


#endif /* HSFC_FWD_DECL_H */
