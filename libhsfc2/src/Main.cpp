//=============================================================================
// Project: High Speed Forward Chaining
// Module: Main
// Authors: Michael Schofield UNSW
// 
//=============================================================================

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
#include <time.h>
#include "dirent.h"

using namespace std;

#include "hsfcEngine.h"

//=============================================================================
// Global Variables
//=============================================================================
int LogFileNumber;

//-----------------------------------------------------------------------------
// ValidateGame
//-----------------------------------------------------------------------------
void ValidateGame(char* GDLFileName) {

	hsfcEngine* Engine;
	hsfcParameters Parameters;
	string* FileName;
	FILE* SummaryFile;

	// Create the Engine
	Engine = new hsfcEngine();

	// Initialise the Engine with the game rules and game Parameters
	Parameters.LogDetail = 2;
	Parameters.LogFileName = NULL;
	Parameters.LowSpeedOnly = false;
	Parameters.MaxLookupSize = 30000000;
	Parameters.MaxRelationSize = 1000000;
	Parameters.MaxStateSize = 30000000;
	Parameters.SCLOnly = false;
	Parameters.SchemaOnly = false;

	// Validate the gdl game
	FileName = new string(GDLFileName);
	Engine->Validate(FileName, Parameters);

	// Record a summary of the results
	SummaryFile = fopen("J:\\HSFC\\Testing\\HSFC_Sumary.log", "a");
	if (SummaryFile != NULL) {
		fprintf(SummaryFile, "%s", GDLFileName);
		if (Parameters.SCLOnly) fprintf(SummaryFile, "\tfalse"); else fprintf(SummaryFile, "\ttrue");
		if (Parameters.SchemaOnly) fprintf(SummaryFile, "\tfalse"); else fprintf(SummaryFile, "\ttrue");
		fprintf(SummaryFile, "\t%f", Parameters.TimeBuildSchema);
		fprintf(SummaryFile, "\t%f", Parameters.TimeOptimise);
		fprintf(SummaryFile, "\t%f", Parameters.TimeBuildLookup);
		fprintf(SummaryFile, "\t%lu", Parameters.StateSize);
		fprintf(SummaryFile, "\t%lu", Parameters.TotalLookupSize);
		fprintf(SummaryFile, "\t%f", Parameters.Playouts);
		fprintf(SummaryFile, "\t%f", Parameters.GamesPerSec);
		fprintf(SummaryFile, "\t%f", Parameters.AveRounds);
		fprintf(SummaryFile, "\t%f", Parameters.StDevRounds);
		fprintf(SummaryFile, "\t%f", Parameters.AveScore0);
		fprintf(SummaryFile, "\t%f", Parameters.StDevScore0);
		fprintf(SummaryFile, "\t%f", Parameters.TreeNodes1);
		fprintf(SummaryFile, "\t%f", Parameters.TreeNodes2);
		fprintf(SummaryFile, "\t%f\n", Parameters.TreeNodes3);
		fflush(SummaryFile);
		fclose(SummaryFile);
	}

	// Clean up
	delete FileName;
	delete Engine;

}

//-----------------------------------------------------------------------------
// TestFilesDebug
//-----------------------------------------------------------------------------
void TestFilesDebug(char* FileName) {

  ValidateGame(FileName);

}
  
//-----------------------------------------------------------------------------
// TestFilesSmall
//-----------------------------------------------------------------------------
void TestFilesSmall() {

	ValidateGame("J:\\GGP\\GameController\\GDL Games\\8puzzle.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\amazons.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\asteroids.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\asteroidsserial.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\babel.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\beatmania.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\blocker.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\blockerparallel.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\blockerserial.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\breakthrough.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\breakthroughsuicide.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\bunk_t.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\buttons.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\checkers.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\chess.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\chomp.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\connect4.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\hanoi.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\hanoi_6_disks.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\knightstour.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\knightthrough.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\nim1.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othello.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\pancakes.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\pancakes6.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\queens.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\sheep_and_wolf.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoe_3d_2player.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoe_3d_6player.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoex9.gdl");

}

//-----------------------------------------------------------------------------
// TestFilesStanford
//-----------------------------------------------------------------------------
void TestFilesStanford() {
	ValidateGame("J:\\GGP\\GameController\\Stanford\\3puzzle.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\8puzzle.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\alquerque.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\bestbuttonsandlights.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\breakthrough.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\breakthroughsmall.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\buttonsandlights.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\checkersonabarrelnokings.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\chinesecheckers.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\chinesecheckers4.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\chinook.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\connectfour.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\dualconnectfour.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\dualhamilton.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\dualhunter.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\dualrainbow.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\farmingquandries.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\firesheep.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\freeforall.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\hamilton.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\hex.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\horseshoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\hunter.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\jointbuttonsandlights.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\jointconnectfour.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\knightstour.gdl");
	//ValidateGame("J:\\GGP\\GameController\\Stanford\\kono.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\madness.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\multiplebuttonsandlights.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\multiplebuttonsandlights_9.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\multiplehamilton.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\multipletictactoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\nineboardtictactoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\pentago.gdl");
	//ValidateGame("J:\\GGP\\GameController\\Stanford\\pilgrimage.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\platformjumpers.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\rainbow.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\skirmish.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\tictactoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\tictactoe3.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\tictactoe5.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\tictactoe7.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\tictictoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\ttcc4.gdl");
	ValidateGame("J:\\GGP\\GameController\\Stanford\\untwistycorridor.gdl");

}


//-----------------------------------------------------------------------------
// TestFiles
//-----------------------------------------------------------------------------
void TestFiles() {

	goto Start;
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\2player_normal_form_2010.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\3conn3.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\3pffa.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\3pttc.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\3qbf-5cnf-20var-40cl.0.qdimacs.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\3qbf-5cnf-20var-40cl.1.qdimacs.SAT.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\3qbf-5cnf-20var-40cl.1.qdimacs.viz.SAT.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\3qbf-5cnf-20var-40cl.2.qdimacs.SAT.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\3qbf-5cnf-20var-40cl.2.qdimacs.satlike.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\4pttc.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\8puzzle.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\ad_game_2x2.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\aipsrovers01.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\alquerque.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\amazons.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\asteroids.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\asteroidsparallel.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\asteroidsserial.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\babel.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\battle.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\battlebrushes.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\BattleSnakes.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\battlesnakes1409.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\battlesnakes1509.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\battlesnakes2011.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\beatmania.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\bidding-tictactoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\bidding-tictactoe_10coins.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\blobwars.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\blocker.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\blockerparallel.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\blockerserial.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\blocks.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\blocks2player.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\blocksworldparallel.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\blocksworldserial.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\bomberman2p.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\brain_teaser_extended.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\brawl.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\breakthrough.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\breakthroughsuicide.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\breakthroughsuicide_v2.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\bunk_t.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\buttons.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\capture_the_king.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\Catch.Me.If.You.Can.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\catch_me.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\catch_me_if_u_can.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\catch_me_if_u_can_1.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\catcha_mouse.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\CatchMeIfYouCan.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\Catch-Me-If-You-Can.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\CatchMeIfYouCanTest.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\CephalopodMicro.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\checkers.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\checkers-cylinder-mustjump.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\checkers-mustjump.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\checkers-mustjump-torus.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\checkers-newgoals.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\checkers-suicide-cylinder-mustjump.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\chess.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\chickentictactoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\chickentoetictac.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\chinesecheckers1.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\chinesecheckers2.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\chinesecheckers3.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\chinesecheckers4.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\chinesecheckers6.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\chinesecheckers6-simultaneous.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\chomp.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\circlesolitaire.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\clobber.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\coins.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\conn4.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\connect4.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\connect5.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\connectfour.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\connectfoursuicide.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\coopconn4.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\crisscross.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\crissrace.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\crossers3.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\cube_2x2x2.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\cubicup.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\cubicup_3player.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\cylinder-checkers.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\dobutsushogi.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\DotsAndBoxesExperimental.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\double_tictactoe_dengji.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\DoubleAuctionDJ.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\doubletictactoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\doubletoetictac.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\dualconnect4.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\duplicatestatelarge.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\duplicatestatemedium.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\duplicatestatesmall.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\endgame.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\eotcatcit.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\eotcitcit.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\farmers.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\farmingquandries.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\fighter.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\firefighter.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\firesheep.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\four_way_battle.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\frogs_and_toads.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\gameofsquares.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\ghostmaze2p.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\god.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\golden_rectangle.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\grid_game.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\grid_game2.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\gt_attrition.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\gt_centipede.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\gt_chicken.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\gt_prisoner.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\gt_ultimatum.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\Guard_Intruder.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\guard_intruder_test.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\guess.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\hallway.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\hanoi.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\hanoi_6_disks.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\hanoi7.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\hanoi7_bugfix.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\hitori.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\incredible.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\javastrike.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\jkkj.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\kalaha_2009.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\kitten_escapes_from_fire.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\knightazons.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\knightfight.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\knightmove.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\knightstour.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\knightthrough.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\knightwar.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\laikLee_hex.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\lightson2x2.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\lightsout.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\lightsout2.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\logistics.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\max_knights.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\maze.gdl");
Start:
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\merrills.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\mimikry.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\minichess.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\minichess-evilconjuncts.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\mummymaze1p.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\mummymaze2p.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\mummymaze2p-comp2007.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\nim1.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\nim2.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\nim3.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\nim4.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\Nine_Mens_Morris_0.1_2p.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\Nine_Mens_Morris_0.11_2p.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\nothello.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\numbertictactoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\oisters_farm.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othello.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othello2.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othello-comp2007.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othello-cornercontrol.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othello-fourway.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othello-fourway-teamswitch.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othello-fourway-teamswitchA.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othello-fourway-teamswitchB.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othello-new.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othello-new-horse.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othellooo.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\othellosuicide.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\pacman3p.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\pancakes.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\pancakes6.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\pancakes88.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\pawn_whopping.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\pawn_whopping_corrected.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\pawn_whopping_simultaneous.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\pawntoqueen.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\peg.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\PEG_3v1_turn.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\peg_bugfixed.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\pentago_2008.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\Pilgrimage.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\platformjumpers.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\point_grab.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\pointgrab_state.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\quad.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\quad_5x5.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\quad_5x5_8_2.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\quad_7x7.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\quarto.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\quartosuicide.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\queens.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\Qyshinsu.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\racer.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\racer4.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\racetrackcorridor.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\rendezvous_asteroids.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\RobinsonRoulete.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\RobinsonRoulette.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\RobinsonRoulette_no_white_peg.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\roshambo2.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\RPS.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\ruledepthexponential.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\ruledepthlinear.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\ruledepthquadratic.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\Runners.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\sat_test_20v_91c.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\sat_test_20v_91c_version2.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\sat_test_20v_91c_visualisation.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\satlike_20v_91c.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\satlike_20v_91c_version2.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\sheep_and_wolf.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\slidingpieces.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\smallest.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\smallest_4player.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\snake_2008.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\snake_2009.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\snake_2009_big.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\statespacelarge.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\statespacemedium.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\statespacesmall.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\sudoku_simple.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\sum15.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\test6868.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\themathematician_easy.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\themathematician_medium.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\Thief_Police.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\ticblock.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoe_3d_2player.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoe_3d_6player.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoe_3d_small_2player.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoe_3d_small_6player.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoe_3player.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoe_orthogonal.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoe-init1.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoelarge.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoelargesuicide.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoeparallel.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoeserial.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictactoex9.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tictictoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\toetictac.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\towerworld.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tpeg.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\Travelers-Dilemma.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tritactoe.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\troublemaker01.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\troublemaker02.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\tttcc4.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\twisty-passages.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\uf20-01.cnf.SAT.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\uf20-010.cnf.SAT.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\uf20-010.cnf.SAT.satlike.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\uf20-020.cnf.SAT.gdl");
	//ValidateGame("J:\\GGP\\GameController\\GDL Games\\uf20-020.cnf.SAT.satlike.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\walkingman.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\wallmaze.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\wargame01.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\wargame02.gdl");
	ValidateGame("J:\\GGP\\GameController\\GDL Games\\Zhadu.gdl");

}


//=============================================================================
// Main
//=============================================================================
int main(int argc, char* argv[]) {

	srand(time(NULL));
	LogFileNumber = 0;

	for (int i = 0; i < 1; i++) {
		//TestFilesDebug("J:\\GGP\\GameController\\GDL Games\\Debug.gdl");
		//TestFilesDebug("J:\\GGP\\GameController\\Stanford\\pilgrimage.gdl");
		//TestFilesDebug("J:\\GGP\\GameController\\GDL Games\\breakthrough.gdl");
		//TestFilesDebug("J:\\GGP\\GameController\\GDL Games\\alquerque.gdl");
		//TestFilesDebug("J:\\GGP\\GameController\\GDL Games\\tictactoe.gdl");
		//TestFilesStanford();
		//TestFilesSmall();
		TestFiles();
	}

	return 0;

}

