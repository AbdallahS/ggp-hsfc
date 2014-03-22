//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HSFCTest

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <boost/foreach.hpp>
#include <hsfc/hsfc.h>



using namespace HSFC;

bool exceptioncheck(const HSFCException& e){ return true; }

unsigned int count_player_moves(const std::vector<PlayerMove> moves, const std::string& player)
{
	unsigned int count = 0;
	BOOST_FOREACH(const PlayerMove& pm, moves)
	{
		std::string tmpstr1;
		if (pm.first.tostring(tmpstr1) == player) ++count;
	}
//	cout << "COUNT " << player << ": " << count << std::endl;
	return count;
}

PlayerMove pick_first(const std::vector<PlayerMove> moves, const std::string& player)
{
	BOOST_FOREACH(const PlayerMove& pm, moves)
	{
		std::string tmpstr1;
		if (pm.first.tostring(tmpstr1) == player) return pm;
	}
	BOOST_CHECK(false);
}

void tictactoe_playout_check(State &state)
{
	if (state.isTerminal()) return;
	State tmpstate(state);
	std::vector<PlayerGoal> results;
	tmpstate.playOut(results);
	bool haswinner = 
		(results[0].second == 100 && results[1].second == 0) || 
		(results[1].second == 100 && results[0].second == 0);
	if (!haswinner)
	{
		BOOST_CHECK(results[0].second == 50);
		BOOST_CHECK(results[1].second == 50);
	}	
}

// NOTE: WE CURRENTLY HAVE NO WAY OF PICKING A MOVE EXPLICITLY. SO THE BEST
// THAT WE CAN DO IS TO PICK THE FIRST FOR EACH PLAYER AND THEN TEST THAT
// IT IS NOT TERMINAL FOR THE FIRST 5 MOVES.
BOOST_AUTO_TEST_CASE(tictactoe)
{
//	BOOST_CHECK_THROW(Game game1("./nonexistentfile"), HSFCException);

	Game game("./tictactoe.gdl");
	State state(game);
	BOOST_CHECK(!state.isTerminal());

	// Tictactoe is turn taking with available moves counting
	// down from 9.
	unsigned int turn = 0;
	int step= 9;
	while (!state.isTerminal())
	{		
		tictactoe_playout_check(state);
		std::vector<PlayerMove> legs;
		state.legals(legs);	   
		if (turn == 0)
		{
			BOOST_CHECK_EQUAL(count_player_moves(legs, "player0"), step);
			BOOST_CHECK_EQUAL(count_player_moves(legs, "player1"), 1);			
		}
		else
		{
			BOOST_CHECK_EQUAL(count_player_moves(legs, "player0"), 1);
			BOOST_CHECK_EQUAL(count_player_moves(legs, "player1"), step);
		}
		std::vector<PlayerMove> does;
		does.push_back(pick_first(legs, "player0"));
		does.push_back(pick_first(legs, "player1"));
		state.play(does);
		turn = (++turn) % 2;
		--step;
	}
	// Tictactoe will terminate early only if there is a winner
	// so test for this and also that a draw is 50/50.
	BOOST_CHECK(step < 5);
	std::vector<PlayerGoal> results;
	state.goals(results);
	BOOST_CHECK_EQUAL(results.size(), 2);
	bool haswinner = 
		(results[0].second == 100 && results[1].second == 0) || 
		(results[1].second == 100 && results[0].second == 0);
	if (step > 1) BOOST_CHECK(haswinner);
	if (!haswinner)
	{
		BOOST_CHECK(results[0].second == 50);
		BOOST_CHECK(results[1].second == 50);
	}
}


