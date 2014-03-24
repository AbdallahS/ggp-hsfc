#!/usr/bin/env python

import unittest
from pyhsfc import *

class TictactoeTest(unittest.TestCase):
    
    #-----------------------------
    # Run a playout from any (non-terminal) tictactoe
    # game state, and ensure that someone is either a 
    # winner or it is a 50-50 draw.
    #-----------------------------
    def playout_check(self, state):
        if state.IsTerminal(): return
        tmpstate = State(state)
        results = []
        tmpstate.Playout(results)
        self.assertEqual(len(results),2)
        if results[0].goal == 100:
            self.assertEqual(results[1].goal,0)
        elif results[1].goal == 100:
            self.assertEqual(results[0].goal,0)
        else:
            self.assertEqual(results[0].goal,50)
            self.assertEqual(results[1].goal,50)

    #-----------------------------
    # Pick the first available move for the player
    #-----------------------------
    def pick_first_legal_move(self, moves, player):
        for m in moves:
            if m.player == player:
                return m


    #-----------------------------
    # Count the legal moves for a given player
    #-----------------------------
    def count_legal_moves(self, moves, player):
        count = 0
        for m in moves:
            if m.player == player:
                count = count + 1
        return count

    #-----------------------------
    # The main test - load up tictactoe and pick
    # moves until the game terminates while running
    # random playouts from each game state.
    #-----------------------------

    def test_tictactoe(self):
        game = Game("./tictactoe.gdl")
        self.assertEqual(game.NumPlayers(), 2)

        state = State(game)
        step=9
        turn=0
        while not state.IsTerminal():
            legals = state.Legals()
            if turn == 0:
                self.assertEqual(self.count_legal_moves(legals, "player0"), step)
                self.assertEqual(self.count_legal_moves(legals, "player1"), 1)
            else:
                self.assertEqual(self.count_legal_moves(legals, "player0"), 1)
                self.assertEqual(self.count_legal_moves(legals, "player1"), step)

            movep0 = pick_first(legals, "player0")
            movep1 = pick_first(legals, "player1")
            state.Play((movep0,movep1))
            turn = (turn + 1) % 2
            step = step - 1
        
        # game has terminated so check for a valid result
	# Tictactoe will terminate early only if there is a winner
	# so test for this and also that a draw is 50/50.
        self.assertTrue(step < 5)
        goals = state.Goals()
        self.assertEqual(goals, 2)
        if step > 1:
            self.assertTrue(((results[0].goal == 100) and (results[1].goal == 0)) or 
                            ((results[1].goal == 100) and (results[0].goal == 0)))
        else:
            self.assertTrue(((results[0].goal == 100) and (results[1].goal == 0)) or 
                            ((results[1].goal == 100) and (results[0].goal == 0)) or
                            ((results[0].goal == 50) and (results[0].goal == 50)))

#-----------------------------
# main
#-----------------------------

def main():
    unittest.main()

if __name__ == '__main__':
    main()
