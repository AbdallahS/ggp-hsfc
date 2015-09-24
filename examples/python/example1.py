#!/usr/bin/env python
#------------------------------------------------------------------------------
# Example script of using the Python HSFC interface and random playout.
# Given a gdl file and desired player, at each non-terminal state it runs 
# a single playout from each joint move and picks the one with the highest score. 
# This is repeated until the game terminates.
#-------------------------------------------------------------------------------
import sys
from pyhsfc import *

#-----------------------------------------------------
# Try to choose a joint move for the state that is best 
# for the player. For each joint move run a single playout,
# then pick the one with the highest score.
#-----------------------------------------------------
def choose_joint_moves(state, player):
    jointmoves = state.joints()
    bestscore = -1
    bestjm = None

    for jm in jointmoves:
        playergoals = None
        tmpstate = State(state)
        tmpstate.play(jm)
        if tmpstate.is_terminal():
            playergoals = tmpstate.goals()
        else:
            playergoals = tmpstate.playout()
        score = playergoals[player]
        if score > bestscore:
            bestscore = score
            bestjm = jm
    if bestscore == -1:
        raise ValueError("No best move for '{0}'".format(player))
    return (bestjm, bestscore)

#-----------------------------------------------------
# run the game to termination
#-----------------------------------------------------
def run(gdlfilename, playername):
    game = Game(file=gdlfilename)
    player = next((r for r in game.players() if str(r) == playername), None)
    if player is None:
        raise ValueError("Player {0} is not a valid player".format(player))
        
    state = State(game)
    while not state.is_terminal():
        (jmv,score) = choose_joint_moves(state, player)
        print "Player {0} score {1} => {2}".format(player, score, jmv.items())
        state.play(jmv)
    playergoals = state.goals()
    print "Final score for player {0} is {1}".format(player, playergoals[player])

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
