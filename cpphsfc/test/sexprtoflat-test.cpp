//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HSFCTest

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <boost/foreach.hpp>
#include <sexprtoflat.h>

using namespace HSFC;


BOOST_AUTO_TEST_CASE(sexprtoflat)
{   
    // For s-expression there is a difference between X and (X).
    // But flat cannot represent (X).
	BOOST_CHECK_EQUAL(sexpr_to_flat("aa"), "aa");
	BOOST_CHECK_EQUAL(sexpr_to_flat("(aa ab)"), "aa/1 ab");
	BOOST_CHECK_EQUAL(sexpr_to_flat("(aa (ab ac) ad)"), "aa/2 ab/1 ac ad");
	BOOST_CHECK_EQUAL(sexpr_to_flat("(aa (ab ac (ad ae) af) ag)"), "aa/2 ab/3 ac ad/1 ae af ag");

	// This will fail
//	BOOST_CHECK_EQUAL(sexpr_to_flat("((aa ab) ac)"), "aa/1/1 ab ac");

}

BOOST_AUTO_TEST_CASE(flattosexpr)
{   
	BOOST_CHECK_EQUAL(flat_to_sexpr("aa"), "aa");
	BOOST_CHECK_EQUAL(flat_to_sexpr("aa/1 ab"), "(aa ab)");
	BOOST_CHECK_EQUAL(flat_to_sexpr("aa/2 ab/1 ac ad"), "(aa (ab ac) ad)");
	BOOST_CHECK_EQUAL(flat_to_sexpr("aa/2 ab/3 ac ad/1 ae af ag"), "(aa (ab ac (ad ae) af) ag)");
}


