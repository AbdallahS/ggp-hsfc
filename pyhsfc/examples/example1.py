#!/usr/bin/env python
#------------------------------------------------------------------------------
# Example script of using the Python HSFC interface and random playout.
# Given a gdl file and desired player, at each non-terminal state it runs 
# a single playout from each joint move and picks the one with the highest score. 
# This is repeated until the game terminates.
#-------------------------------------------------------------------------------
import sys
from pyhsfc import *
import tools

#-----------------------------------------------------
# make sure player is valid for the game.
#-----------------------------------------------------
def valid_player(game, player):
    for p in game.Players():
        if str(p) == player:
            return True
    return False

#-----------------------------------------------------
# get the score for the given player
#-----------------------------------------------------
def get_goal(playergoals, player):
    for plygl in playergoals:
        if str(plygl.player) == player:
            return plygl.goal
    raise ValueError("Player '{0}' does not have a goal score".format(player))

#-----------------------------------------------------
# Try to choose a joint move for the state that is best 
# for the player. For each joint move run a single playout,
# then pick the one with the highest score.
#-----------------------------------------------------
def choose_joint_moves(state, player):
    playermoves = state.Legals()

    # joint moved defined in tools.py takes a list of legal moves
    # and generates an iteration over all valid joint moves.
    jointmoves = tools.JointMoves(playermoves)
    bestscore = -1
    bestjm = None

    for jm in jointmoves:
        playergoals = None
        tmpstate = State(state)
        tmpstate.Play(jm)
        if tmpstate.IsTerminal():
            playergoals = tmpstate.Goals()
        else:
            playergoals = tmpstate.Playout()
        score = get_goal(playergoals, player)
        if score > bestscore:
            bestscore = score
            bestjm = jm
    if bestscore == -1:
        raise ValueError("No best move for '{0}'".format(player))
    return (bestjm, bestscore)

#-----------------------------------------------------
# run the game to termination
#-----------------------------------------------------
def run(gdlfilename, player):
    game = Game(gdlfilename)
    if not valid_player(game, player):
        raise ValueError("Player {0} is not a valid player".format(player))
        
    state = game.InitState()
    while not state.IsTerminal():
        (jmv,score) = choose_joint_moves(state, player)
        print "Player {0} score {1} => {2}".format(player, score, jmv)
        state.Play(jmv)
    playergoals = state.Goals()
    print "Final score for player {0} is {1}".format(player, get_goal(playergoals, player))

#----------------------------------------------------
# main
#----------------------------------------------------

def main():
    if len(sys.argv) != 3:
        print "usage: {0} <gdlfilename> <player>".format(sys.argv[0])
        return
    run(sys.argv[1], sys.argv[2]);

if __name__ == '__main__':
    main()
