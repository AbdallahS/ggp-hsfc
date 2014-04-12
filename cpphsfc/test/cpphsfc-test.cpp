//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HSFCTest

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/unordered_map.hpp>
#include <hsfc/hsfc.h>
#include <hsfc/portable.h>

using namespace HSFC;

// Count the number of moves that are for the given player
unsigned int count_player_moves(const std::vector<PlayerMove> moves, const std::string& player)
{
	unsigned int count = 0;
	BOOST_FOREACH(const PlayerMove& pm, moves)
	{
		if (pm.first.tostring() == player) ++count;
	}
	return count;
}

// Pick the first available move for the given player
PlayerMove pick_first(const std::vector<PlayerMove> moves, const std::string& player)
{
	BOOST_FOREACH(const PlayerMove& pm, moves)
	{
		if (pm.first.tostring() == player) return pm;
	}
	BOOST_CHECK(false);
}

// Run a playout of from any (non-terminal) tictactoe game state. 
// Someone either wins or it is a draw.
void tictactoe_playout_check(const State &state)
{
	if (state.isTerminal()) return;
	State tmpstate(state);
	std::vector<PlayerGoal> results;
	tmpstate.playout(results);
	bool haswinner = 
		(results[0].second == 100 && results[1].second == 0) || 
		(results[1].second == 100 && results[0].second == 0);
	if (!haswinner)
	{
		BOOST_CHECK(results[0].second == 50);
		BOOST_CHECK(results[1].second == 50);
	}	
}

// Return the player 
Player get_player(const Game& game, const std::string& playername)
{
    std::vector<Player> players;
    game.players(players);
    BOOST_FOREACH(const Player& p, players)
    {
        if (p.tostring() == playername) return p;
	}
    BOOST_CHECK(false);
}

/****************************************************************
 * Test of loading GDL from a string
 ****************************************************************/
const char* g_gdl = " \
;;;; RULES   \
 \
(role xplayer) \
(role oplayer) \
(init (cell 1 1 b)) \
(init (cell 1 2 b)) \
(init (cell 1 3 b)) \
(init (cell 2 1 b)) \
(init (cell 2 2 b)) \
(init (cell 2 3 b)) \
(init (cell 3 1 b)) \
(init (cell 3 2 b)) \
(init (cell 3 3 b)) \
(init (control xplayer)) \
(<= (next (cell ?m ?n x)) (does xplayer (mark ?m ?n)) (true (cell ?m ?n b))) \
(<= (next (cell ?m ?n o)) (does oplayer (mark ?m ?n)) (true (cell ?m ?n b))) \
(<= (next (cell ?m ?n ?w)) (true (cell ?m ?n ?w)) (distinct ?w b)) \
(<= (next (cell ?m ?n b)) (does ?w (mark ?j ?k)) (true (cell ?m ?n b)) (distinct ?m ?j)) \
(<= (next (cell ?m ?n b)) (does ?w (mark ?j ?k)) (true (cell ?m ?n b)) (distinct ?n ?k)) \
(<= (next (control xplayer)) (true (control oplayer))) \
(<= (next (control oplayer)) (true (control xplayer))) \
(<= (row ?m ?x) (true (cell ?m 1 ?x)) (true (cell ?m 2 ?x)) (true (cell ?m 3 ?x))) \
(<= (column ?n ?x) (true (cell 1 ?n ?x)) (true (cell 2 ?n ?x)) (true (cell 3 ?n ?x))) \
(<= (diagonal ?x) (true (cell 1 1 ?x)) (true (cell 2 2 ?x)) (true (cell 3 3 ?x))) \
(<= (diagonal ?x) (true (cell 1 3 ?x)) (true (cell 2 2 ?x)) (true (cell 3 1 ?x))) \
(<= (line ?x) (row ?m ?x)) \
(<= (line ?x) (column ?m ?x)) \
(<= (line ?x) (diagonal ?x)) \
(<= open (true (cell ?m ?n b))) \
(<= (legal ?w (mark ?x ?y)) (true (cell ?x ?y b)) (true (control ?w))) \
(<= (legal xplayer noop) (true (control oplayer))) \
(<= (legal oplayer noop) (true (control xplayer))) \
(<= (goal xplayer 100) (line x)) \
(<= (goal xplayer 50) (not (line x)) (not (line o)) (not open)) \
(<= (goal xplayer 0) (line o)) \
(<= (goal oplayer 100) (line o)) \
(<= (goal oplayer 50) (not (line x)) (not (line o)) (not open)) \
(<= (goal oplayer 0) (line x)) \
(<= terminal (line x)) \
(<= terminal (line o)) \
(<= terminal (not open)) \
 \
;;;; STRATS  \
 \
(strat does 0) \
(strat goal 1) \
(strat init 0) \
(strat legal 0) \
(strat next 0) \
(strat role 0) \
(strat terminal 1) \
(strat true 0) \
(strat cell 0) \
(strat column 0) \
(strat control 0) \
(strat diagonal 0) \
(strat line 0) \
(strat open 0) \
(strat row 0) \
 \
;;;; PATHS   \
 \
(arg does 0 oplayer) \
(arg does 0 xplayer) \
(arg does 1 mark 0 1) \
(arg does 1 mark 0 2) \
(arg does 1 mark 0 3) \
(arg does 1 mark 1 1) \
(arg does 1 mark 1 2) \
(arg does 1 mark 1 3) \
(arg does 1 noop) \
(arg goal 0 oplayer) \
(arg goal 0 xplayer) \
(arg goal 1 0) \
(arg goal 1 100) \
(arg goal 1 50) \
(arg init 0 cell 0 1) \
(arg init 0 cell 0 2) \
(arg init 0 cell 0 3) \
(arg init 0 cell 1 1) \
(arg init 0 cell 1 2) \
(arg init 0 cell 1 3) \
(arg init 0 cell 2 b) \
(arg init 0 control 0 xplayer) \
(arg legal 0 oplayer) \
(arg legal 0 xplayer) \
(arg legal 1 mark 0 1) \
(arg legal 1 mark 0 2) \
(arg legal 1 mark 0 3) \
(arg legal 1 mark 1 1) \
(arg legal 1 mark 1 2) \
(arg legal 1 mark 1 3) \
(arg legal 1 noop) \
(arg next 0 cell 0 1) \
(arg next 0 cell 0 2) \
(arg next 0 cell 0 3) \
(arg next 0 cell 1 1) \
(arg next 0 cell 1 2) \
(arg next 0 cell 1 3) \
(arg next 0 cell 2 b) \
(arg next 0 cell 2 o) \
(arg next 0 cell 2 x) \
(arg next 0 control 0 oplayer) \
(arg next 0 control 0 xplayer) \
(arg role 0 oplayer) \
(arg role 0 xplayer) \
(arg terminal ) \
(arg true 0 cell 0 1) \
(arg true 0 cell 0 2) \
(arg true 0 cell 0 3) \
(arg true 0 cell 1 1) \
(arg true 0 cell 1 2) \
(arg true 0 cell 1 3) \
(arg true 0 cell 2 b) \
(arg true 0 cell 2 o) \
(arg true 0 cell 2 x) \
(arg true 0 control 0 oplayer) \
(arg true 0 control 0 xplayer) \
(arg column 0 1) \
(arg column 0 2) \
(arg column 0 3) \
(arg column 1 b) \
(arg column 1 o) \
(arg column 1 x) \
(arg diagonal 0 b) \
(arg diagonal 0 o) \
(arg diagonal 0 x) \
(arg line 0 b) \
(arg line 0 o) \
(arg line 0 x) \
(arg open ) \
(arg row 0 1) \
(arg row 0 2) \
(arg row 0 3) \
(arg row 1 b) \
(arg row 1 o) \
(arg row 1 x) \
 \
;;;; DOMAINS \
 \
(domain_p does (set oplayer xplayer) (set mark noop)) \
(domain_p goal (set oplayer xplayer) (set 0 100 50)) \
(domain_p init (set cell control)) \
(domain_p legal (set oplayer xplayer) (set mark noop)) \
(domain_p next (set cell control)) \
(domain_p role (set oplayer xplayer)) \
(domain_p terminal ) \
(domain_p true (set cell control)) \
(domain_p column (set 1 2 3) (set b o x)) \
(domain_p diagonal (set b o x)) \
(domain_p line (set b o x)) \
(domain_p open ) \
(domain_p row (set 1 2 3) (set b o x)) \
 \
(domain_s 0 ) \
(domain_s 1 ) \
(domain_s 100 ) \
(domain_s 2 ) \
(domain_s 3 ) \
(domain_s 50 ) \
(domain_s b ) \
(domain_s cell (set 1 2 3) (set 1 2 3) (set b o x)) \
(domain_s control (set oplayer xplayer)) \
(domain_s mark (set 1 2 3) (set 1 2 3)) \
(domain_s noop ) \
(domain_s o ) \
(domain_s oplayer ) \
(domain_s x ) \
(domain_s xplayer ) \
";

/****************************************************************
 * Test functions of the State class
 ****************************************************************/


BOOST_AUTO_TEST_CASE(unordered_map_test)
{
	Game game(boost::filesystem::path("./tictactoe.gdl"));

    boost::unordered_map<Player, std::string> playermap;
    std::vector<Player> players;
    game.players(players);
    BOOST_FOREACH(const Player& p, players)
    {
        playermap[p] = p.tostring();
    }
    typedef std::pair<Player,std::string> pp_t;
    BOOST_FOREACH(const pp_t& pp, playermap)
    {
        BOOST_CHECK_EQUAL(pp.first.tostring(), pp.second);
    }
}



/****************************************************************
 * Test functions of the State class
 ****************************************************************/

BOOST_AUTO_TEST_CASE(state_functions)
{
	Game game(boost::filesystem::path("./tictactoe.gdl"));
	State state1 = game.initState();
	BOOST_CHECK(!state1.isTerminal());

	State state2(state1);
	BOOST_CHECK(!state2.isTerminal());

	std::vector<PlayerMove> legs;
	state1.legals(legs);	   
	BOOST_CHECK_EQUAL(legs.size(), 10);

	// To test the PortableState we make a single move from the
	// initial state, then save this as a portable state. Run a
	// playout, reload the portable state and check that this is takes
	// us back to the correct state.

	// NOTE: There is no function to compare that two states are equal!
	// Best we can do is test that it is no-longer terminal. Need to
	// ask Michael to add some functions.

	std::vector<PlayerMove> does;
	does.push_back(pick_first(legs, "xplayer"));
	does.push_back(pick_first(legs, "oplayer"));
	BOOST_CHECK_EQUAL(does.size(), 2);
	state1.play(does);
	state2.play(does);
	BOOST_CHECK(!state1.isTerminal());
	BOOST_CHECK(!state2.isTerminal());
	boost::shared_ptr<PortableState> pstate = state2.CreatePortableState();
	BOOST_CHECK(pstate.get() != NULL);

	std::vector<PlayerGoal> result;
	state2.playout(result);
	BOOST_CHECK(state2.isTerminal());

	state2.LoadPortableState(*pstate);
	BOOST_CHECK(!state2.isTerminal());
	
	// Now test the serialisation of Portable state. Do the same test
	// as before but by taking the further step of (de-)serializing.
	std::ostringstream oserialstream;
	boost::archive::text_oarchive oa(oserialstream);
	oa << pstate;
	std::string serialised(oserialstream.str());
	BOOST_TEST_MESSAGE(serialised);
	
	std::istringstream iserialstream(serialised);
	boost::archive::text_iarchive ia(iserialstream);
	boost::shared_ptr<PortableState> pstate2;
	ia >> pstate2;

	state2.playout(result);
	BOOST_CHECK(state2.isTerminal());

	state2.LoadPortableState(*pstate);
	BOOST_CHECK(!state2.isTerminal());

}

/****************************************************************
 * Testing that PortableState works across games. Load 2 games.
 * For a state in game 1 playout (so it is in a terminal state).
 * Then serialise and deserialise this into a state in game 2
 * and check that it is in a terminal state.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(send_state_across_games)
{
	Game game1(g_gdl);
	Game game2(boost::filesystem::path("./tictactoe.gdl"));
	State state1 = game1.initState();
	State state2 = game2.initState();
	BOOST_CHECK(!state1.isTerminal());
	BOOST_CHECK(!state2.isTerminal());

	std::vector<PlayerGoal> result;
	state1.playout(result);
	BOOST_CHECK(state1.isTerminal());

	boost::shared_ptr<PortableState> pstate1 = state1.CreatePortableState();
	boost::shared_ptr<PortableState> pstate2;

	std::ostringstream oserialstream;
	boost::archive::text_oarchive oa(oserialstream);
	oa << pstate1;
	std::string serialised(oserialstream.str());
	
	std::istringstream iserialstream(serialised);
	boost::archive::text_iarchive ia(iserialstream);
	ia >> pstate2;

	state2.LoadPortableState(*pstate2);
	BOOST_CHECK(state2.isTerminal());
}

BOOST_AUTO_TEST_CASE(send_state_across_games2)
{
	Game game1(g_gdl);
	Game game2(boost::filesystem::path("./tictactoe.gdl"));
	State state1(game1);
	BOOST_CHECK(!state1.isTerminal());

	std::vector<PlayerGoal> result;
	state1.playout(result);
	BOOST_CHECK(state1.isTerminal());

    PortableState pstate1(state1);

	std::ostringstream oserialstream;
	boost::archive::text_oarchive oa(oserialstream);
	oa << pstate1;
	std::string serialised(oserialstream.str());
	std::istringstream iserialstream(serialised);
	boost::archive::text_iarchive ia(iserialstream);
    PortableState pstate2;
    ia >> pstate2;
	State state2(game2, pstate2);

	BOOST_CHECK(state2.isTerminal());
}


/****************************************************************
 * Test that the move and player text generated for tictactoe 
 * are correct.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(text_check)
{
	Game game(boost::filesystem::path("./tictactoe.gdl"));
	State state1 = game.initState();
	BOOST_CHECK(!state1.isTerminal());

	std::vector<PlayerMove> legs;
	state1.legals(legs);	   

	unsigned int count = 0;
	bool matchply = false;
	bool matchmv = false;
	BOOST_FOREACH(const PlayerMove& pm, legs)
	{
		std::string ply = pm.first.tostring();
		std::string mv = pm.second.tostring();
		BOOST_CHECK(ply == "oplayer" || ply == "xplayer");
		BOOST_CHECK(mv == "noop" || mv == "(mark 1 1)" ||
					mv == "(mark 1 2)" || mv == "(mark 1 3)" ||
					mv == "(mark 2 1)" || mv == "(mark 2 2)" ||
					mv == "(mark 2 3)" || mv == "(mark 3 1)" ||
					mv == "(mark 3 2)" || mv == "(mark 3 3)");
	}
}


// NOTE: WE CURRENTLY HAVE NO WAY OF PICKING A MOVE EXPLICITLY. SO THE BEST
// THAT WE CAN DO IS TO PICK THE FIRST FOR EACH PLAYER AND THEN TEST THAT
// IT IS NOT TERMINAL FOR THE FIRST 5 MOVES.
BOOST_AUTO_TEST_CASE(tictactoe)
{
//	BOOST_TEST_MESSAGE("Testing HSFC with tictactoe");
//	BOOST_CHECK_THROW(Game game1("./nonexistentfile"), HSFCException);

	Game game(boost::filesystem::path("./tictactoe.gdl"));
	BOOST_CHECK_EQUAL(game.numPlayers(), 2);
	State state = game.initState();
	BOOST_CHECK(!state.isTerminal());

//	BOOST_TEST_MESSAGE("Choosing legal joint moves until terminal intersperced with playouts.");
	// Tictactoe is turn taking with available moves counting
	// down from 9.
	unsigned int turn = 0;
	int step= 9;
	while (!state.isTerminal())
	{		
		tictactoe_playout_check(state);
		BOOST_CHECK(!state.isTerminal());
		std::vector<PlayerMove> legs;
		state.legals(legs);	   
		if (turn == 0)
		{
			BOOST_CHECK_EQUAL(count_player_moves(legs, "xplayer"), step);
			BOOST_CHECK_EQUAL(count_player_moves(legs, "oplayer"), 1);			
		}
		else
		{
			BOOST_CHECK_EQUAL(count_player_moves(legs, "xplayer"), 1);
			BOOST_CHECK_EQUAL(count_player_moves(legs, "oplayer"), step);
		}
		std::vector<PlayerMove> does;
		does.push_back(pick_first(legs, "xplayer"));
		does.push_back(pick_first(legs, "oplayer"));
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
	BOOST_TEST_MESSAGE("Succesfully played tictactoe till termination.");
}


