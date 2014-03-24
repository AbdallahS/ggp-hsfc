#!/usr/bin/env python

from pyhsfc import *

#-----------------------------
# Pick the first available move for the player
#-----------------------------

def pick_first(moves, player):
    for m in moves:
        if m.player == player:
            return m


#def tictactoe_playout_check(state):
#    if state.IsTerminal(): return;
    
#-----------------------------
# main
#-----------------------------

game = Game("./tictactoe.gdl")
print "Loaded tictactoe. Num players = {0}".format(game.NumPlayers())
for p in game.Players():
    print "Player: {0}".format(p)

state = State(game)
while not state.IsTerminal():
    legals = state.Legals()
    movep0 = pick_first(legals, "player0")
    movep1 = pick_first(legals, "player1")
    print "Chosen joint move: {0} and {1}".format(movep0, movep1)
    state.Play((movep0,movep1))

for g in state.Goals():
    print g
